/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef WiFiModePage_h
#define WiFiModePage_h

#include "../GUI/UIPage.h"
#include "../GUI/Widget/UILabel.h"
#include "../GUI/Widget/UISelectValue.h"
#include "../GUI/Widget/UINavBar.h"
#include "../GUIRender.h"
#include <WiFi.h>

class WiFiModePage : public UIPage {
public:
    WiFiModePage();
    
    // 重写按钮事件处理
    void onButtonBack(void* context = nullptr) override;
    void onButtonEnter(void* context = nullptr) override;
    void onButton2(void* context = nullptr) override;
    void onButton4(void* context = nullptr) override;
    void onButton6(void* context = nullptr) override;
    void onButton8(void* context = nullptr) override;
    
    // 重写update方法，用于定时刷新WiFi状态
    void update() override;
    
private:
    void initLayout(); // 初始化页面布局
    void updateDisplay(); // 更新显示
    void toggleWiFi(); // 切换WiFi状态
    void saveSettings(); // 保存设置
    void updateHomePageWiFiStatus(); // 更新主页WiFi状态
    void refreshWiFiInfo(); // 刷新WiFi信息
    
    UILabel* titleLabel;
    UILabel* ssidLabel;        // "SSID:"
    UILabel* ssidValue;        // 显示SSID或"未配置"
    UILabel* ipLabel;          // "IP地址:"
    UILabel* ipValue;          // 显示IP地址
    UILabel* statusLabel;      // "状态:"
    UISelectValue* statusValue; // 显示开启/关闭状态
    UINavBar* navBar;
    
    bool wifiEnabled;              // WiFi是否启用
    String wifiSSID;               // WiFi SSID
    String wifiPassword;           // WiFi密码
    String wifiIP;                 // WiFi IP地址
    bool isConnected;              // 是否已连接
    int selectedItem;              // 当前选中的项 (0=状态)
    unsigned long lastUpdateTime;  // 上次更新时间（毫秒）
    wl_status_t lastWiFiStatus;    // 上次WiFi状态
};

#endif

