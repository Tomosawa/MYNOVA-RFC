/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef __WIFIMANAGER_H__
#define __WIFIMANAGER_H__

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

struct WiFiScanResult {
    String ssid;
    int32_t rssi;
    uint8_t encryptionType;
    bool isScanning;
    int networksFound;
};

class WiFiManager {
public:
    WiFiManager();
    ~WiFiManager();
    
    // 初始化WiFi管理器
    void init();
    
    // ==================== WiFi STA 相关 ====================
    // 异步扫描WiFi网络
    void startScan();
    
    // 检查扫描是否完成
    bool isScanComplete();
    
    // 获取扫描结果的JSON字符串
    String getScanResultJson();
    
    // 连接到指定的WiFi网络（阻塞式）
    bool connectToWiFi(const String& ssid, const String& password);
    
    // 异步连接WiFi（非阻塞）
    void connectToWiFiAsync(const String& ssid, const String& password);
    
    // 断开WiFi连接
    void disconnect();
    
    // 检查WiFi连接状态
    bool isConnected();
    
    // 获取当前连接的WiFi信息
    String getConnectionInfo();
    
    // 完全关闭WiFi（用于休眠）
    void shutdown();
    
    // ==================== AP 热点相关 ====================
    // 启动AP热点（异步）
    void startAPAsync(const String& ssid, const String& password);
    
    // 停止AP热点
    void stopAP();
    
    // 检查AP是否已启动
    bool isAPStarted();
    
    // 获取AP的IP地址
    String getAPIP();
    
    // ==================== 配置存储相关 ====================
    // 保存WiFi配置
    bool saveConfig(const String& ssid, const String& password);
    
    // 加载WiFi配置
    bool loadConfig(String& ssid, String& password);
    
    // 清除WiFi配置
    void clearConfig();
    
private:
    // 扫描任务
    static void scanTask(void* parameter);
    
    // WiFi连接任务
    static void wifiConnectTask(void* parameter);
    
    // AP启动任务
    static void apSetupTask(void* parameter);
    
    // 扫描结果
    std::vector<WiFiScanResult> scanResults;
    
    // 状态标志
    bool scanning;
    bool scanComplete;
    bool apStarted;
    
    // 任务句柄
    TaskHandle_t scanTaskHandle;
    TaskHandle_t wifiConnectTaskHandle;
    TaskHandle_t apSetupTaskHandle;
    
    // 互斥锁
    SemaphoreHandle_t mutex;
    
    // 当前连接/AP信息
    String currentSSID;
    String currentPassword;
    String apSSID;
    String apPassword;
    bool connected;
    
    // AP配置
    static IPAddress AP_IP;
    static IPAddress AP_GATEWAY;
    static IPAddress AP_SUBNET;
};

#endif
