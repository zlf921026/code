#include "NetWorkManager.h"
#include "CCLuaEngine.h"
#include "HelperNode.h"
#include "MTNotificationQueue.h"
#include "../cocos/deprecated/CCNotificationCenter.h"

NetWorkManager* NetWorkManager::m_pInstance = NULL;

NetWorkManager::CCGarbo NetWorkManager::m_Garbo;

NetWorkManager::NetWorkManager()
{
	isConnect = false;
	//m_socket = -1;
	m_serverIp = "";
	m_serverPort = 0;
	m_connectCount = 0;
	m_helperNode = NULL;
	NotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(NetWorkManager::sendMessage), "recv", NULL);
}

NetWorkManager::~NetWorkManager()
{

}

NetWorkManager* NetWorkManager::getInstance()
{
	if (m_pInstance == NULL)
	{
		m_pInstance = new NetWorkManager();
	}
	return m_pInstance;
}

void NetWorkManager::sendMessage(Ref* pSender)
{
	auto pL = LuaEngine::getInstance()->getLuaStack()->getLuaState();
	auto path = FileUtils::getInstance()->fullPathForFilename("Helper.lua");
	luaL_dofile(pL, path.c_str());
	lua_getglobal(pL, "showTips");
	lua_pushstring(pL, "reConnect");
	lua_call(pL, 1, 0);
}

void NetWorkManager::setHelper(HelperNode* node)
{
	m_helperNode = node;
}

void NetWorkManager::initData()
{
	Director::getInstance()->getScheduler()->scheduleSelector(schedule_selector(MTNotificationQueue::postNotifications),
		MTNotificationQueue::getInstance(), 1.0 / 60.0, false);
	auto path = FileUtils::getInstance()->fullPathForFilename("data/strings.xml");
	CCLOG("file path = %s", path.c_str());
	if (path != "")
	{
		auto strings = FileUtils::getInstance()->getValueMapFromFile(path);
		std::string ip = strings["ip"].asString();
		std::string port = strings["port"].asString();
		m_serverIp = ip;
		m_serverPort = atoi(port.c_str());
		CCLOG("ip===%s", ip.c_str());
		CCLOG("port===%d", atoi(port.c_str()));

	}
	else
	{
		auto strings = FileUtils::getInstance()->getValueMapFromFile("data/strings.xml");
		std::string ip = strings["ip"].asString();
		std::string port = strings["port"].asString();
		CCLOG("ip===%s", ip.c_str());
		CCLOG("port===%s", port.c_str());
		m_serverIp = ip;
		m_serverPort = atoi(port.c_str());
	}
}

void NetWorkManager::init()
{
	initData();
	CCLOG("start init socket");
	int result = m_socket.Init();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) 
	if (result == -1)
		return;
#endif
	m_socket.Create(AF_INET, SOCK_STREAM);
	std::thread connectThread(&NetWorkManager::connectFunc, this);
	std::thread recvThread(&NetWorkManager::recvFunc, this);
	connectThread.detach();
	recvThread.detach();

}

void NetWorkManager::connectFunc()
{
	CCLOG("start connect server");
	while (true)
	{
		CCLOG("connect count is %d", m_connectCount);
		if (m_connectCount >= 10)
		{
			CCLOG("try connect count 1000, not net work or server is close");



			/*if (m_helperNode != NULL)
			{
				m_helperNode->addFunction([]
				{
					auto pL = LuaEngine::getInstance()->getLuaStack()->getLuaState();
					auto path = FileUtils::getInstance()->fullPathForFilename("Helper.lua");
					luaL_dofile(pL, path.c_str());
					lua_getglobal(pL, "showTips");
					lua_pushstring(pL, "reConnect");
					lua_call(pL, 1, 0);
				});
			}*/

			//Director::getInstance()->getScheduler()->performFunctionInCocosThread([]
			//{
			//	auto pL = LuaEngine::getInstance()->getLuaStack()->getLuaState();
			//	auto path = FileUtils::getInstance()->fullPathForFilename("Helper.lua");
			//	luaL_dofile(pL, path.c_str());
			//	lua_getglobal(pL, "showTips");
			//	//int code = 0;
			//	lua_pushstring(pL, "reConnect");
			//	lua_call(pL, 1, 0);
			//	//CCLOG("code===%d", code);
			//});
			return;
		}
		isConnect = m_socket.Connect(m_serverIp.c_str(), m_serverPort);
		if (!isConnect)
		{
			m_connectCount++;
			continue;
		}
		else
		{
			
			break;
		}
	}
	CCLOG("connect server success");
}

void NetWorkManager::recvFunc()
{
	while (true)
	{
		if (!isConnect)
		{
			//CCLOG("Has not been linked to the server");
			continue;
		}
		char buff[MSGLEN];
		int result = recv(m_socket.getSocket(), buff, MSGLEN, 0);
		if (result <= 0)
		{
			CCLOG("server connect close");
			continue;
		}
		buff[result] = '\0';


		Director::getInstance()->getScheduler()->performFunctionInCocosThread([&, this]
		{
			auto pL = LuaEngine::getInstance()->getLuaStack()->getLuaState();
			auto path = FileUtils::getInstance()->fullPathForFilename("Helper.lua");
			luaL_dofile(pL, path.c_str());
			lua_getglobal(pL, "updateChatView");
			lua_pushstring(pL, buff);
			lua_call(pL, 1, 0);
		});
	}
	m_socket.Close();
	m_socket = -1;
}

void NetWorkManager::sendData(std::string data)
{
	if (!isConnect)
	{
		CCLOG("Has not been linked to the server");
		return;
	}
	m_socket.Send(data.c_str(), strlen(data.c_str()));
}

void NetWorkManager::reConnect()
{
	m_connectCount = 0;
	std::thread connectThread(&NetWorkManager::connectFunc, this);
	connectThread.detach();
}