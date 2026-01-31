/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// SystemSetting.cpp
#include "SystemSetting.h"
#include "DataStore.h"
#include "GUIRender.h"
#include "IOPin.h"
#include <esp_sleep.h>
#include <driver/rtc_io.h>
#include <driver/gpio.h>

extern DataStore dataStore;
extern GUIRender guiRender;

SystemSetting::SystemSetting() {
    lastActivityTime = 0;
    isScreenOff = false;
    pWiFiManager = nullptr;
}

void SystemSetting::init(WiFiManager* wifiMgr) {
    Serial.println("SystemSetting: 初始化系统设置");
    
    // 保存WiFiManager指针
    pWiFiManager = wifiMgr;
    
    // 从存储读取配置
    config = dataStore.LoadSystemConfig();
    
    // 设置默认值（如果配置为空）
    if (config.APName.length() == 0) {
        config.APName = DEFAULT_AP_NAME;
    }
    if (config.APPassword.length() == 0) {
        config.APPassword = DEFAULT_AP_PASSWORD;
    }
    
    // 应用亮度配置
    applyBrightness();
    
    // 启动AP（如果启用）- 通过WiFiManager
    if (config.APEnabled && pWiFiManager) {
        pWiFiManager->startAPAsync(config.APName, config.APPassword);
    }
    
    // 连接WiFi（如果启用）- 通过WiFiManager
    if (config.WifiEnabled && config.WifiName.length() > 0 && pWiFiManager) {
        pWiFiManager->connectToWiFiAsync(config.WifiName, config.WifiPassword);
    }
    
    // 重置空闲计时器
    resetIdleTimer();
    
    Serial.println("SystemSetting: 初始化完成");
    Serial.print("  亮度: "); Serial.println(config.brightness);
    Serial.print("  蜂鸣器: "); Serial.println(config.buzzerEnable);
    Serial.print("  自动休眠时间: "); Serial.println(config.autoSleepTime);
    Serial.print("  自动息屏时间: "); Serial.println(config.autoScreenOffTime);
    Serial.print("  AP热点: "); Serial.println(config.APEnabled);
    Serial.print("  WiFi: "); Serial.println(config.WifiEnabled);
    Serial.print("  WiFi名称: "); Serial.println(config.WifiName);
}

void SystemSetting::update() {
    // 检查自动休眠
    if (config.autoSleepTime > 0) {
        checkAutoSleep();
    }
    
    // 检查自动息屏
    if (config.autoScreenOffTime > 0) {
        checkAutoScreenOff();
    }
}

void SystemSetting::resetIdleTimer() {
    lastActivityTime = millis();
    
    // 如果屏幕已关闭，则打开屏幕
    if (isScreenOff) {
        screenOn();
    }
}

void SystemSetting::checkAutoSleep() {
    unsigned long currentTime = millis();
    unsigned long idleTime = currentTime - lastActivityTime;
    
    // 检查是否超时（需要考虑millis溢出）
    if (idleTime >= config.autoSleepTime) {
        Serial.println("SystemSetting: 自动休眠时间到，进入休眠");
        enterSleep();
    }
}

void SystemSetting::checkAutoScreenOff() {
    // 如果屏幕已经关闭，不需要再次检查
    if (isScreenOff) {
        return;
    }
    
    unsigned long currentTime = millis();
    unsigned long idleTime = currentTime - lastActivityTime;
    
    // 检查是否超时
    if (idleTime >= config.autoScreenOffTime) {
        Serial.println("SystemSetting: 自动息屏时间到，关闭屏幕");
        screenOff();
    }
}

void SystemSetting::enterSleep() {
    Serial.println("SystemSetting: 进入深度休眠模式（Deep Sleep）");
    
    // 关闭屏幕
    screenOff();
    
    // 通过WiFiManager关闭WiFi和AP
    if (pWiFiManager) {
        pWiFiManager->shutdown();
    }
    
    Serial.println("SystemSetting: 配置RTC GPIO唤醒源");
    
    // ESP32-S3的RTC GPIO: 0-21
    // 只配置RTC GPIO的按键作为唤醒源
    uint64_t wakeup_pin_mask = 0;
    int rtc_gpio_count = 0;
    
    // 按键是INPUT_PULLUP，按下时为低电平
    // 使用rtc_gpio_pullup_en和rtc_gpio_pulldown_dis确保上拉状态
    
    if (PIN_KEY_ENTER <= 21) {
        wakeup_pin_mask |= (1ULL << PIN_KEY_ENTER);
        rtc_gpio_pullup_en((gpio_num_t)PIN_KEY_ENTER);
        rtc_gpio_pulldown_dis((gpio_num_t)PIN_KEY_ENTER);
        rtc_gpio_hold_en((gpio_num_t)PIN_KEY_ENTER);
        rtc_gpio_count++;
        Serial.print("  KEY_ENTER (GPIO"); Serial.print(PIN_KEY_ENTER); Serial.println(")");
    }
    
    if (PIN_KEY_3 <= 21) {
        wakeup_pin_mask |= (1ULL << PIN_KEY_3);
        rtc_gpio_pullup_en((gpio_num_t)PIN_KEY_3);
        rtc_gpio_pulldown_dis((gpio_num_t)PIN_KEY_3);
        rtc_gpio_hold_en((gpio_num_t)PIN_KEY_3);
        rtc_gpio_count++;
        Serial.print("  KEY_3 (GPIO"); Serial.print(PIN_KEY_3); Serial.println(")");
    }
    
    if (PIN_KEY_6 <= 21) {
        wakeup_pin_mask |= (1ULL << PIN_KEY_6);
        rtc_gpio_pullup_en((gpio_num_t)PIN_KEY_6);
        rtc_gpio_pulldown_dis((gpio_num_t)PIN_KEY_6);
        rtc_gpio_hold_en((gpio_num_t)PIN_KEY_6);
        rtc_gpio_count++;
        Serial.print("  KEY_6 (GPIO"); Serial.print(PIN_KEY_6); Serial.println(")");
    }
    
    if (rtc_gpio_count > 0) {
        // 配置ext1唤醒：任意一个按键按下（0=低电平）即唤醒
        esp_sleep_enable_ext1_wakeup_io(wakeup_pin_mask, ESP_EXT1_WAKEUP_ALL_LOW);
        Serial.print("SystemSetting: 已配置"); 
        Serial.print(rtc_gpio_count); 
        Serial.println("个RTC按键作为唤醒源");
        Serial.println("SystemSetting: 提示 - 只有ENTER键、3键、6键可以唤醒");
    } else {
        // 如果没有RTC GPIO按键，使用定时器唤醒
        Serial.println("SystemSetting: 警告 - 没有RTC GPIO按键，使用定时器唤醒（60秒）");
        esp_sleep_enable_timer_wakeup(60 * 1000000ULL);
    }
    
    Serial.println("SystemSetting: 进入深度休眠...");
    Serial.flush();
    delay(100);
    
    // 进入深度休眠
    // 注意：Deep Sleep会重启设备，从setup()重新开始执行
    esp_deep_sleep_start();
    
    // 这行代码永远不会执行
}

void SystemSetting::wakeUp() {
    Serial.println("SystemSetting: 从休眠唤醒");
    
    // 打开屏幕
    screenOn();
    
    // 重启AP（如果之前启用）- 通过WiFiManager
    if (config.APEnabled && pWiFiManager) {
        pWiFiManager->startAPAsync(config.APName, config.APPassword);
    }
    
    // 重置计时器
    resetIdleTimer();
}

void SystemSetting::screenOff() {
    if (isScreenOff) {
        return;
    }
    
    Serial.println("SystemSetting: 关闭屏幕");
    guiRender.setPowerSave(true);
    isScreenOff = true;
}

void SystemSetting::screenOn() {
    if (!isScreenOff) {
        return;
    }
    
    Serial.println("SystemSetting: 打开屏幕");
    guiRender.setPowerSave(false);
    isScreenOff = false;
}

void SystemSetting::applyBrightness() {
    // 将亮度值（0-100）转换为对比度（0-255）
    int contrast = (config.brightness * 255) / 100;
    guiRender.setContrast(contrast);
}

// 获取和设置配置
SystemConfig SystemSetting::getConfig() {
    return config;
}

void SystemSetting::setConfig(SystemConfig newConfig, bool saveToFlash) {
    config = newConfig;
    
    // 应用配置
    applyBrightness();
    
    // 处理AP - 通过WiFiManager
    if (pWiFiManager) {
        if (config.APEnabled && !pWiFiManager->isAPStarted()) {
            pWiFiManager->startAPAsync(config.APName, config.APPassword);
        } else if (!config.APEnabled && pWiFiManager->isAPStarted()) {
            pWiFiManager->stopAP();
        }
        
        // 处理WiFi - 通过WiFiManager
        if (config.WifiEnabled && !pWiFiManager->isConnected()) {
            pWiFiManager->connectToWiFiAsync(config.WifiName, config.WifiPassword);
        } else if (!config.WifiEnabled && pWiFiManager->isConnected()) {
            pWiFiManager->disconnect();
        }
    }
    
    if (saveToFlash) {
        saveConfig();
    }
}

void SystemSetting::saveConfig() {
    Serial.println("SystemSetting: 保存配置到存储");
    dataStore.SaveSystemConfig(config);
}

// 单独设置项的方法
void SystemSetting::setBrightness(int brightness, bool saveToFlash) {
    config.brightness = brightness;
    applyBrightness();
    
    if (saveToFlash) {
        saveConfig();
    }
}

void SystemSetting::setBuzzerEnable(bool enable, bool saveToFlash) {
    config.buzzerEnable = enable;
    
    if (saveToFlash) {
        saveConfig();
    }
}

void SystemSetting::setRepeatTransmit(int times, bool saveToFlash) {
    config.repeatTransmit = times;
    
    if (saveToFlash) {
        saveConfig();
    }
}

void SystemSetting::setAutoSleepTime(long timeMs, bool saveToFlash) {
    config.autoSleepTime = timeMs;
    
    if (saveToFlash) {
        saveConfig();
    }
}

void SystemSetting::setAutoScreenOffTime(long timeMs, bool saveToFlash) {
    config.autoScreenOffTime = timeMs;
    
    if (saveToFlash) {
        saveConfig();
    }
}

void SystemSetting::setAPEnabled(bool enabled, bool saveToFlash) {
    config.APEnabled = enabled;
    
    // 先保存配置，再启动异步任务，避免并发冲突
    if (saveToFlash) {
        saveConfig();
    }
    
    // 通过WiFiManager控制AP
    if (pWiFiManager) {
        if (config.APEnabled) {
            pWiFiManager->startAPAsync(config.APName, config.APPassword);
        } else {
            pWiFiManager->stopAP();
        }
    }
}

void SystemSetting::setWifiEnabled(bool enabled, bool saveToFlash) {
    config.WifiEnabled = enabled;
    
    // 先保存配置，再启动异步任务，避免并发冲突
    if (saveToFlash) {
        saveConfig();
    }
    
    // 通过WiFiManager控制WiFi
    if (pWiFiManager) {
        if (config.WifiEnabled && !pWiFiManager->isConnected()) {
            pWiFiManager->connectToWiFiAsync(config.WifiName, config.WifiPassword);
        } else if (!config.WifiEnabled && pWiFiManager->isConnected()) {
            pWiFiManager->disconnect();
        }
    }
}

void SystemSetting::setAPConfig(String name, String password, bool saveToFlash) {
    config.APName = name;
    config.APPassword = password;
    
    // 如果AP正在运行，重启以应用新配置
    if (config.APEnabled && pWiFiManager) {
        pWiFiManager->stopAP();
        pWiFiManager->startAPAsync(config.APName, config.APPassword);
    }
    
    if (saveToFlash) {
        saveConfig();
    }
}

void SystemSetting::setWifiConfig(String ssid, String password, bool saveToFlash) {
    config.WifiName = ssid;
    config.WifiPassword = password;
    
    // 如果WiFi已连接，断开并重新连接
    if (config.WifiEnabled && pWiFiManager && pWiFiManager->isConnected()) {
        pWiFiManager->disconnect();
        pWiFiManager->connectToWiFiAsync(config.WifiName, config.WifiPassword);
    }
    
    if (saveToFlash) {
        saveConfig();
    }
}

// 获取单独的配置项
bool SystemSetting::getBuzzerEnable() {
    return config.buzzerEnable;
}

int SystemSetting::getBrightness() {
    return config.brightness;
}

int SystemSetting::getRepeatTransmit() {
    return config.repeatTransmit;
}

long SystemSetting::getAutoSleepTime() {
    return config.autoSleepTime;
}

long SystemSetting::getAutoScreenOffTime() {
    return config.autoScreenOffTime;
}

// MQTT/Home Assistant配置
void SystemSetting::setMQTTEnabled(bool enabled, bool saveToFlash) {
    config.MQTTEnabled = enabled;
    
    if (saveToFlash) {
        saveConfig();
    }
    
    Serial.print("SystemSetting: MQTT ");
    Serial.println(enabled ? "启用" : "禁用");
}

void SystemSetting::setMQTTConfig(String server, int port, String username, String password, bool saveToFlash) {
    config.MQTTServer = server;
    config.MQTTPort = port;
    config.MQTTUsername = username;
    config.MQTTPassword = password;
    
    if (saveToFlash) {
        saveConfig();
    }
    
    Serial.println("SystemSetting: MQTT配置已更新");
    Serial.print("  服务器: ");
    Serial.println(server);
    Serial.print("  端口: ");
    Serial.println(port);
}