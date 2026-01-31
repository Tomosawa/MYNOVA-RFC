/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef DATASTORE_H
#define DATASTORE_H

#include "RadioHelper.h"
#include <WString.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

enum class MQTTMode {
    AUTO,
    MANUAL
};

struct QuickKey{
    int key1;
    int key2;
    int key3;
    int key4;
    int key5;
    int key6;
    int key7;
    int key8;
    int key9;
};
struct RadioData
{
    String name;
    RCData rcData;
};

struct SystemConfig{
    bool buzzerEnable;
    int repeatTransmit;
    int brightness;//屏幕亮度
    long autoSleepTime;//自动休眠时间
    long autoScreenOffTime;//自动息屏时间
    bool APEnabled;
    bool WifiEnabled;
    String APName;
    String APPassword;
    String WifiName;
    String WifiPassword;
    // MQTT/Home Assistant配置
    bool MQTTEnabled;           // MQTT是否启用
    String MQTTServer;          // MQTT服务器地址
    int MQTTPort;               // MQTT端口
    String MQTTUsername;        // MQTT用户名
    String MQTTPassword;        // MQTT密码
};

class DataStore
{
public:
    DataStore();
    ~DataStore();
    void SaveData(int index, RadioData radioData);
    RadioData ReadData(int index);
    void SaveQuickKey(QuickKey keyData);
    QuickKey LoadQuickKey();
    void SaveSystemConfig(SystemConfig systemConfig);
    SystemConfig LoadSystemConfig();
    void ClearAllData(); // 清空所有存储数据
    
    // WiFi配置相关方法
    bool SaveWiFiConfig(const String& ssid, const String& password);
    bool LoadWiFiConfig(String& ssid, String& password);
    void ClearWiFiConfig();
    
    // MQTT/HA配置相关方法
    bool SaveMQTTConfig(MQTTMode mode, const String& server, uint16_t port, const String& username, const String& password);
    bool LoadMQTTConfig(MQTTMode& mode, String& server, uint16_t& port, String& username, String& password);
    void ClearMQTTConfig();
    bool HasMQTTConfig();
public:
    int buzzerEnable = 1;
    
private:
    // 添加互斥锁以确保线程安全
    SemaphoreHandle_t preferencesMutex;
};

#endif