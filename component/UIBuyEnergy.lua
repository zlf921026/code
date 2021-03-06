UIBuyEnergy = class("UIBuyEnergy")

function UIBuyEnergy:ctor(panel,isShowMsgBox)
	local uiSkin = UISkin.new("UIBuyEnergy")
    uiSkin:setParent(panel)
    uiSkin:setLocalZOrder(100)

    
    self.secLvBg = UISecLvPanelBg.new(uiSkin:getRootNode(), self)
    self.secLvBg:setContentHeight(280)
    self.secLvBg:setBackGroundColorOpacity(120)
    self.secLvBg:setTitle(TextWords:getTextWord(1605))
    

    self.proxy = panel:getProxy(GameProxys.Role)

    self._uiSkin = uiSkin
    self._panel = panel
    self._isShowMsgBox = isShowMsgBox

    local mainPanel = self:getChildByName("Panel_1")
    mainPanel:setLocalZOrder(101)
    for i=1,2 do
        local name = string.format("Panel_1/Panel%d", i)
        local parent = self:getChildByName(name)
        local btn = parent:getChildByName("Button_11")
        btn.id = i
        ComponentUtils:addTouchEventListener(btn, self.buyHandler, nil,self)
        self["label"..i] = parent:getChildByName("needLab")
    end

    self:registerProxyEvent()
    self:show()
end

function UIBuyEnergy:registerProxyEvent()
    self.proxy:addEventListener(AppEvent.PROXY_BUYEVENT_UPDATE, self, self.hide)
end

function UIBuyEnergy:finalize()
    self.proxy:removeEventListener(AppEvent.PROXY_BUYEVENT_UPDATE, self, self.hide)
    self._uiSkin:finalize()
end

function UIBuyEnergy:getChildByName(name)
    return self._uiSkin:getChildByName(name)
end

function UIBuyEnergy:show()
    self._uiSkin:setVisible(true)
    self.label1:setString(self.proxy:getEnergyNeedMoney())
    self.label2:setString(self.proxy:getCrusadeEnergyNeedMoney())
end

function UIBuyEnergy:hide()
    self._uiSkin:setVisible(false)
end

function UIBuyEnergy:buyHandler(sender)
    local id = sender.id
    if id == 1 then
        self.proxy:getBuyEnergyBox(self._panel,self._isShowMsgBox)
    else
        self.proxy:getBuyCrusadeEnergyBox(self._panel,self._isShowMsgBox)
    end
end

-- 是否弹窗元宝不足
function UIBuyEnergy:isShowRechargeUI(sender)
    local needMoney = sender.money

    local haveGold = self.proxy:getRoleAttrValue(PlayerPowerDefine.POWER_gold) or 0--拥有元宝

    if needMoney > haveGold then
        local parent = self._panel:getParent()
        local panel = parent.panel
        if panel == nil then
            local panel = UIRecharge.new(parent, self)
            parent.panel = panel
        else
            panel:show()
        end

    else
        sender.callFunc()
    end
end