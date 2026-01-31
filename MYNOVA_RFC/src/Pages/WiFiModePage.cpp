/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// WiFiModePage.cpp
#include "WiFiModePage.h"
#include "../GUI/UIEngine.h"
#include "../SystemSetting.h"
#include "../GUIRender.h"
#include "../Pages/HomePage.h"

extern UIEngine uiEngine;
extern SystemSetting systemSetting;
extern HomePage uiPageHome;

WiFiModePage::WiFiModePage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) {
    // 从SystemSetting读取当前设置
    SystemConfig config = systemSetting.getConfig();
    wifiEnabled = config.WifiEnabled;
    wifiSSID = config.WifiName;
    wifiPassword = config.WifiPassword;
    
    // 获取WiFi连接状态
    isConnected = WiFi.status() == WL_CONNECTED;
    if (isConnected) {
        wifiIP = WiFi.localIP().toString();
    } else {
        wifiIP = "未连接";
    }
    
    // 默认选中第一项（状态）
    selectedItem = 0;
    lastUpdateTime = 0;
    lastWiFiStatus = WiFi.status();
    
    initLayout();
}

void WiFiModePage::initLayout() {
    // 标题
    titleLabel = new UILabel();
    titleLabel->x = 0;
    titleLabel->y = 0;
    titleLabel->width = SCREEN_WIDTH;
    titleLabel->height = 12;
    titleLabel->label = "WiFi模式设置";
    titleLabel->textAlign = CENTER;
    titleLabel->verticalAlign = MIDDLE;
    addWidget(titleLabel);
    
    // 第一行：SSID
    ssidLabel = new UILabel();
    ssidLabel->x = 4;
    ssidLabel->y = 13;
    ssidLabel->width = 40;
    ssidLabel->height = 12;
    ssidLabel->label = "SSID:";
    ssidLabel->textAlign = LEFT;
    ssidLabel->verticalAlign = MIDDLE;
    ssidLabel->textFont = u8g2_font_wqy12_t_gb2312;
    addWidget(ssidLabel);
    
    ssidValue = new UILabel();
    ssidValue->x = 40;
    ssidValue->y = 13;
    ssidValue->width = 88;
    ssidValue->height = 12;
    ssidValue->textAlign = LEFT;
    ssidValue->verticalAlign = MIDDLE;
    ssidValue->textFont = u8g2_font_wqy12_t_gb2312;
    addWidget(ssidValue);
    
    // 第二行：IP地址
    ipLabel = new UILabel();
    ipLabel->x = 4;
    ipLabel->y = 26;
    ipLabel->width = 40;
    ipLabel->height = 12;
    ipLabel->label = "IP:";
    ipLabel->textAlign = LEFT;
    ipLabel->verticalAlign = MIDDLE;
    ipLabel->textFont = u8g2_font_wqy12_t_gb2312;
    addWidget(ipLabel);
    
    ipValue = new UILabel();
    ipValue->x = 40;
    ipValue->y = 26;
    ipValue->width = 88;
    ipValue->height = 12;
    ipValue->textAlign = LEFT;
    ipValue->verticalAlign = MIDDLE;
    ipValue->textFont = u8g2_font_wqy12_t_gb2312;
    ipValue->label = "未连接";
    addWidget(ipValue);
    
    // 第三行：状态
    statusLabel = new UILabel();
    statusLabel->x = 4;
    statusLabel->y = 39;
    statusLabel->width = 40;
    statusLabel->height = 12;
    statusLabel->label = "状态:";
    statusLabel->textAlign = LEFT;
    statusLabel->verticalAlign = MIDDLE;
    statusLabel->textFont = u8g2_font_wqy12_t_gb2312;
    addWidget(statusLabel);
    
    statusValue = new UISelectValue();
    statusValue->x = 40;
    statusValue->y = 39;
    statusValue->width = 88;
    statusValue->height = 12;
    statusValue->bShowBorder = true;
    statusValue->textFont = u8g2_font_wqy12_t_gb2312;
    addWidget(statusValue);
    
    // 导航栏
    navBar = new UINavBar(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH, 12);
    navBar->setLeftButtonText("返回");
    navBar->setRightButtonText("切换");
    addWidget(navBar);
    
    // 更新显示
    updateDisplay();
}

void WiFiModePage::updateDisplay() {
    // 刷新WiFi信息
    refreshWiFiInfo();
    
    // 更新SSID显示
    if (wifiSSID.length() == 0) {
        ssidValue->label = "未配置";
    } else if (isConnected) {
        ssidValue->label = wifiSSID;
        ipValue->label = wifiIP;
    } else {
        ssidValue->label = wifiSSID;
        ipValue->label = "未连接";
    }
    
    // 更新状态显示
    statusValue->value = wifiEnabled ? "开启" : "关闭";
    statusValue->bSelected = (selectedItem == 0);
}

void WiFiModePage::refreshWiFiInfo() {
    // 获取WiFi连接状态
    wl_status_t status = WiFi.status();
    isConnected = (status == WL_CONNECTED);
    
    if (isConnected) {
        wifiIP = WiFi.localIP().toString();
        // 如果已连接，更新SSID（可能与配置不同）
        String connectedSSID = WiFi.SSID();
        if (connectedSSID.length() > 0) {
            wifiSSID = connectedSSID;
        }
    } else if (wifiEnabled && status != WL_DISCONNECTED && status != WL_NO_SHIELD && status != WL_IDLE_STATUS) {
        // WiFi已启用且正在连接中
        wifiIP = "连接中...";
    } else {
        wifiIP = "未连接";
    }
}

void WiFiModePage::toggleWiFi() {
    wifiEnabled = !wifiEnabled;
    // 立即应用设置
    systemSetting.setWifiEnabled(wifiEnabled, true);
    updateHomePageWiFiStatus();
    updateDisplay();
}

void WiFiModePage::saveSettings() {
    // 保存设置
    updateHomePageWiFiStatus();
}

void WiFiModePage::updateHomePageWiFiStatus() {
    // 更新主页的WiFi状态显示
    uiPageHome.updateStatus();
}

void WiFiModePage::onButtonBack(void* context) {
    // 返回键
    navBar->showLeftBlink(1, 80, 80, [this]() {
        saveSettings();
        updateHomePageWiFiStatus();
        uiEngine.navigateBack();
    });
}

void WiFiModePage::onButtonEnter(void* context) {
    if (selectedItem == 0) {
        toggleWiFi();
        updateDisplay();
    }
}

void WiFiModePage::onButton2(void* context) {
    // 上移选择（目前只有一项，保留以便将来扩展）
    updateDisplay();
}

void WiFiModePage::onButton4(void* context) {
    if (selectedItem == 0) {
        // 切换WiFi状态为关闭
        if (wifiEnabled) {
            toggleWiFi();
        }
    }
}

void WiFiModePage::onButton6(void* context) {
    if (selectedItem == 0) {
        // 切换WiFi状态为开启
        if (!wifiEnabled) {
            toggleWiFi();
        }
    }
}

void WiFiModePage::onButton8(void* context) {
    // 下移选择（目前只有一项，保留以便将来扩展）
    updateDisplay();
}

void WiFiModePage::update() {
    // 每500毫秒检查一次WiFi状态
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime >= 500) {
        lastUpdateTime = currentTime;
        
        wl_status_t currentStatus = WiFi.status();
        
        // 如果WiFi状态发生变化，更新显示
        if (currentStatus != lastWiFiStatus) {
            lastWiFiStatus = currentStatus;
            updateDisplay();
        }
    }
}

