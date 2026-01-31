/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include <Arduino.h>
#include "src/IOPin.h"
#include "src/GUIRender.h"
#include "src/ButtonHandle.h"
#include "src/RadioHelper.h"
#include "src/BatteryManager.h"
#include "src/SystemSetting.h"
#include "src/GUI/Widget/UITitleBar.h"
#include "src/GUI/UIEngine.h"
#include "src/Pages/HomePage.h"
#include "Version.h"
#include "src/WebService.h"
#include "src/WiFiManager.h"
#include "src/DataStore.h"
#include "src/SplashScreen.h"
#include "src/HAManager.h"

DataStore dataStore;
GUIRender guiRender;
ButtonHandle buttonHandle;
RadioHelper radioHelper;
BatteryManager batteryManager;
SystemSetting systemSetting;
WebService webService;
WiFiManager wifiManager;
SplashScreen splashScreen;
HAManager haManager;
extern HomePage uiPageHome;

void setup()
{
    Serial.begin(115200);
    Serial.println(">>>>>>>>>>MYNOVA RFC Started!<<<<<<<<<<");
    // 版本信息
    String version_info = "MYNOVA RFC Firmware V" + String(FIRMWARE_VERSION) + " \r\nBuild " + String(FIRMWARE_BUILD_DATE);
    Serial.println(version_info);
    // 获取设备ID
    Serial.println("Device ID: " + String(ESP.getEfuseMac(), HEX));

    // ========== 第一步：初始化显示屏并显示启动画面 ==========
    guiRender.initDisplay();
    splashScreen.init(guiRender.getU8G2());
    splashScreen.setProgress(0, "Initializing");
    
    // ========== 初始化各模块，更新进度 ==========
    
    // 初始化WiFi管理器 (10%)
    splashScreen.setProgress(10, "WiFi Manager");
    wifiManager.init();
    delay(50);

    // 初始化Home Assistant集成 (25%)
    splashScreen.setProgress(25, "Home Assistant");
    haManager.init(&dataStore, &radioHelper);
    delay(50);

    // 初始化系统设置 (40%)
    splashScreen.setProgress(40, "System Settings");
    systemSetting.init(&wifiManager);
    delay(50);
    
    // 初始化Web服务 (55%)
    splashScreen.setProgress(55, "Web Service");
    webService.init(&wifiManager);
    webService.serverStart();
    delay(50);
    
    // 初始化按钮处理 (70%)
    splashScreen.setProgress(70, "Button Handler");
    buttonHandle.init();
    delay(50);
    
    // 初始化电池管理 (85%)
    splashScreen.setProgress(85, "Battery Monitor");
    batteryManager.init();
    haManager.setBatteryManager(&batteryManager);
    delay(50);
    
    // 初始化射频模块 (90%)
    splashScreen.setProgress(95, "RF Module");
    radioHelper.init();
    delay(50);
    
    // 完成初始化 (100%)
    splashScreen.setProgress(100, "Ready!");
    delay(300);
    
    // 播放完成动画
    splashScreen.finish();
    
    // ========== 启动主界面 ==========
    Serial.println("Setup: 启动主界面");
    guiRender.init();  // 启动GUI渲染任务
    uiPageHome.showPage();
    
    Serial.println("==========MYNOVA RFC Initialized!==========");
}
void loop()
{
  yield();
  
  // 更新系统设置（处理自动休眠和息屏）
  systemSetting.update();
  
  // 处理Home Assistant MQTT消息
  haManager.loop();
  
  delay(10);
}