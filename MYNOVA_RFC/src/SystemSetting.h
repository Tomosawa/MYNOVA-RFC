/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// SystemSetting.h
#ifndef SystemSetting_h
#define SystemSetting_h

#include <Arduino.h>
#include "DataStore.h"
#include "WiFiManager.h"

// 默认AP配置
#define DEFAULT_AP_NAME "MYNOVA_RFC"
#define DEFAULT_AP_PASSWORD "MYNOVA123"

class SystemSetting {
public:
    SystemSetting();
    
    // 初始化和更新
    void init(WiFiManager* wifiMgr); // 初始化，需要传入WiFiManager指针
    void update(); // 在主循环中调用，处理计时等
    void resetIdleTimer(); // 重置空闲计时器（按键时调用）
    
    // 获取和设置配置
    SystemConfig getConfig();
    void setConfig(SystemConfig config, bool saveToFlash = false);
    void saveConfig(); // 保存到存储
    
    // 单独设置项的方法（会立即生效）
    void setBrightness(int brightness, bool saveToFlash = false);
    void setBuzzerEnable(bool enable, bool saveToFlash = false);
    void setRepeatTransmit(int times, bool saveToFlash = false);
    void setAutoSleepTime(long timeMs, bool saveToFlash = false);
    void setAutoScreenOffTime(long timeMs, bool saveToFlash = false);
    void setAPEnabled(bool enabled, bool saveToFlash = false);
    void setWifiEnabled(bool enabled, bool saveToFlash = false);
    void setAPConfig(String name, String password, bool saveToFlash = false);
    void setWifiConfig(String ssid, String password, bool saveToFlash = false);
    
    // MQTT/Home Assistant配置
    void setMQTTEnabled(bool enabled, bool saveToFlash = false);
    void setMQTTConfig(String server, int port, String username, String password, bool saveToFlash = false);
    
    // 获取单独的配置项
    bool getBuzzerEnable();
    int getBrightness();
    int getRepeatTransmit();
    long getAutoSleepTime();
    long getAutoScreenOffTime();
    
    // 手动触发操作
    void enterSleep(); // 进入休眠
    void screenOff(); // 关闭屏幕
    void screenOn(); // 打开屏幕

private:
    SystemConfig config;
    WiFiManager* pWiFiManager; // WiFiManager指针
    unsigned long lastActivityTime; // 最后活动时间戳（毫秒）
    bool isScreenOff; // 屏幕是否已关闭
    
    void checkAutoSleep(); // 检查是否需要自动休眠
    void checkAutoScreenOff(); // 检查是否需要自动息屏
    void wakeUp(); // 从休眠唤醒
    void applyBrightness(); // 应用亮度设置
};

#endif