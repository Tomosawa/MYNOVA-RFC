/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "DataStore.h"
#include <Preferences.h>

Preferences preferences;

#define KEY_NAMESPACE   "RadioData"
#define KEY_NAME        "NAME"
#define KEY_FREQTYPE    "FREQTYPE"
#define KEY_DATA        "DATA"
#define KEY_BITLENGTH   "BLENGTH"
#define KEY_PULSELENGTH "PLENGTH"
#define KEY_PROTOCAL    "PROTOCOL"
#define KEY_QUICKKEY    "QUICKKEY"
#define KEY_QUICKKEY_1  "QKEY_1"
#define KEY_QUICKKEY_2  "QKEY_2"
#define KEY_QUICKKEY_3  "QKEY_3"
#define KEY_QUICKKEY_4  "QKEY_4"
#define KEY_QUICKKEY_5  "QKEY_5"
#define KEY_QUICKKEY_6  "QKEY_6"
#define KEY_QUICKKEY_7  "QKEY_7"
#define KEY_QUICKKEY_8  "QKEY_8"
#define KEY_QUICKKEY_9  "QKEY_9"
#define KEY_BUZZER_ENABLE "BUZZER_ENABLE"
#define KEY_REPEAT_TRANSMIT "REPEAT_TRANSMIT"
#define KEY_BRIGHTNESS "BRIGHTNESS"
#define KEY_AUTO_SLEEP_TIME "AUTO_SLEEP_TIME"
#define KEY_AUTO_SCREEN_OFF_TIME "AUTO_SCREEN_OFF_TIME"
#define KEY_AP_ENABLED "AP_ENABLED"
#define KEY_WIFI_ENABLED "WIFI_ENABLED"
#define KEY_AP_NAME "AP_NAME"

// MQTT模式与字符串转换辅助函数
static String mqttModeToString(MQTTMode mode) {
    switch (mode) {
        case MQTTMode::AUTO:
            return "auto";
        case MQTTMode::MANUAL:
            return "manual";
        default:
            return "auto";
    }
}

static MQTTMode stringToMQTTMode(const String& mode) {
    if (mode == "manual") {
        return MQTTMode::MANUAL;
    }
    return MQTTMode::AUTO;
}
#define KEY_AP_PASSWORD "AP_PASSWORD"
#define KEY_WIFI_NAME "WIFI_NAME"
#define KEY_WIFI_PASSWORD "WIFI_PASSWORD"

// MQTT配置专用命名空间
#define MQTT_NAMESPACE "mqtt"
#define MQTT_KEY_MODE "mode"
#define MQTT_KEY_SERVER "server"
#define MQTT_KEY_PORT "port"
#define MQTT_KEY_USERNAME "username"
#define MQTT_KEY_PASSWORD "password"

DataStore::DataStore()
{
    // 创建互斥锁，并检查是否创建成功
    preferencesMutex = xSemaphoreCreateMutex();
    if (preferencesMutex == nullptr) {
        Serial.println("DataStore: 互斥锁创建失败!");
    }
}

DataStore::~DataStore()
{
    // 删除互斥锁
    if (preferencesMutex != nullptr) {
        vSemaphoreDelete(preferencesMutex);
        preferencesMutex = nullptr;
    }
}

void DataStore::SaveData(int index, RadioData radioData)
{
    // 检查互斥锁是否有效
    if (preferencesMutex == nullptr) {
        Serial.println("DataStore::SaveData: 互斥锁未初始化");
        return;
    }
    
    // 获取互斥锁，增加超时保护
    if (xSemaphoreTake(preferencesMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        char keySuffix[16];
        sprintf(keySuffix, "_%d", index);
        
        char nameKey[32];
        char freqTypeKey[32];
        char dataKey[32];
        char bitLengthKey[32];
        char pulseLengthKey[32];
        char protocolKey[32];
        
        strcpy(nameKey, KEY_NAME);
        strcat(nameKey, keySuffix);
        
        strcpy(freqTypeKey, KEY_FREQTYPE);
        strcat(freqTypeKey, keySuffix);
        
        strcpy(dataKey, KEY_DATA);
        strcat(dataKey, keySuffix);
        
        strcpy(bitLengthKey, KEY_BITLENGTH);
        strcat(bitLengthKey, keySuffix);
        
        strcpy(pulseLengthKey, KEY_PULSELENGTH);
        strcat(pulseLengthKey, keySuffix);
        
        strcpy(protocolKey, KEY_PROTOCAL);
        strcat(protocolKey, keySuffix);

        preferences.begin(KEY_NAMESPACE);
        preferences.putString(nameKey, radioData.name);
        preferences.putUInt(freqTypeKey, (uint32_t)radioData.rcData.freqType);
        preferences.putUInt(bitLengthKey, radioData.rcData.bitLength);
        preferences.putUInt(protocolKey, radioData.rcData.protocal);
        preferences.putUInt(dataKey, radioData.rcData.data);
        preferences.putUInt(pulseLengthKey, radioData.rcData.pulseLength);
        
        preferences.end();
        
        // 释放互斥锁
        xSemaphoreGive(preferencesMutex);
    } else {
        Serial.println("DataStore::SaveData: 获取互斥锁超时");
    }
}

RadioData DataStore::ReadData(int index)
{
    RadioData radioData;
    
    // 检查互斥锁是否有效
    if (preferencesMutex == nullptr) {
        Serial.println("DataStore::ReadData: 互斥锁未初始化");
        return radioData;
    }
    
    // 获取互斥锁，增加超时保护
    if (xSemaphoreTake(preferencesMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        char keySuffix[16];
        sprintf(keySuffix, "_%d", index);
        
        char nameKey[32];
        char freqTypeKey[32];
        char dataKey[32];
        char bitLengthKey[32];
        char pulseLengthKey[32];
        char protocolKey[32];
        
        strcpy(nameKey, KEY_NAME);
        strcat(nameKey, keySuffix);
        
        strcpy(freqTypeKey, KEY_FREQTYPE);
        strcat(freqTypeKey, keySuffix);
        
        strcpy(dataKey, KEY_DATA);
        strcat(dataKey, keySuffix);
        
        strcpy(bitLengthKey, KEY_BITLENGTH);
        strcat(bitLengthKey, keySuffix);
        
        strcpy(pulseLengthKey, KEY_PULSELENGTH);
        strcat(pulseLengthKey, keySuffix);
        
        strcpy(protocolKey, KEY_PROTOCAL);
        strcat(protocolKey, keySuffix);

        preferences.begin(KEY_NAMESPACE);
        radioData.name = preferences.getString(nameKey);
        radioData.rcData.freqType = (FreqType)preferences.getUInt(freqTypeKey);
        radioData.rcData.bitLength = preferences.getUInt(bitLengthKey);
        radioData.rcData.protocal = preferences.getUInt(protocolKey);
        radioData.rcData.data = preferences.getUInt(dataKey);
        radioData.rcData.pulseLength = preferences.getUInt(pulseLengthKey);
        preferences.end();
        
        // 释放互斥锁
        xSemaphoreGive(preferencesMutex);
    } else {
        Serial.println("DataStore::ReadData: 获取互斥锁超时");
    }
    
    return radioData;
}

void DataStore::SaveQuickKey(QuickKey keyData)
{
    // 检查互斥锁是否有效
    if (preferencesMutex == nullptr) {
        Serial.println("DataStore::SaveQuickKey: 互斥锁未初始化");
        return;
    }
    
    // 获取互斥锁，增加超时保护
    if (xSemaphoreTake(preferencesMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        preferences.begin(KEY_NAMESPACE);

        preferences.putUInt(KEY_QUICKKEY_1, keyData.key1);
        preferences.putUInt(KEY_QUICKKEY_2, keyData.key2);
        preferences.putUInt(KEY_QUICKKEY_3, keyData.key3);
        preferences.putUInt(KEY_QUICKKEY_4, keyData.key4);
        preferences.putUInt(KEY_QUICKKEY_5, keyData.key5);
        preferences.putUInt(KEY_QUICKKEY_6, keyData.key6);
        preferences.putUInt(KEY_QUICKKEY_7, keyData.key7);
        preferences.putUInt(KEY_QUICKKEY_8, keyData.key8);
        preferences.putUInt(KEY_QUICKKEY_9, keyData.key9);
        
        preferences.end();
        
        // 释放互斥锁
        xSemaphoreGive(preferencesMutex);
        
        // 打印出值
        Serial.print("SaveQuickKey: ");
        Serial.print(keyData.key1);
        Serial.print(", ");
        Serial.print(keyData.key2);
        Serial.print(", ");
        Serial.print(keyData.key3);
        Serial.print(", ");
        Serial.print(keyData.key4);
        Serial.print(", ");
        Serial.print(keyData.key5);
        Serial.print(", ");
        Serial.print(keyData.key6);
        Serial.print(", ");
        Serial.print(keyData.key7);
        Serial.print(", ");
        Serial.print(keyData.key8);
        Serial.print(", ");
        Serial.print(keyData.key9);
        Serial.println();
        Serial.println("DataStore: SaveQuickKey() 执行完成");
    } else {
        Serial.println("DataStore::SaveQuickKey: 获取互斥锁超时");
    }
}

QuickKey DataStore::LoadQuickKey()
{
    QuickKey quickKey = {0}; // 初始化为0
    
    // 检查互斥锁是否有效
    if (preferencesMutex == nullptr) {
        Serial.println("DataStore::LoadQuickKey: 互斥锁未初始化");
        return quickKey;
    }
    
    // 获取互斥锁，增加超时保护
    if (xSemaphoreTake(preferencesMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        Serial.println("DataStore: LoadQuickKey() 被调用");
        preferences.begin(KEY_NAMESPACE);

        quickKey.key1 = preferences.getUInt(KEY_QUICKKEY_1, 0);
        quickKey.key2 = preferences.getUInt(KEY_QUICKKEY_2, 0);
        quickKey.key3 = preferences.getUInt(KEY_QUICKKEY_3, 0);
        quickKey.key4 = preferences.getUInt(KEY_QUICKKEY_4, 0);
        quickKey.key5 = preferences.getUInt(KEY_QUICKKEY_5, 0);
        quickKey.key6 = preferences.getUInt(KEY_QUICKKEY_6, 0);
        quickKey.key7 = preferences.getUInt(KEY_QUICKKEY_7, 0);
        quickKey.key8 = preferences.getUInt(KEY_QUICKKEY_8, 0);
        quickKey.key9 = preferences.getUInt(KEY_QUICKKEY_9, 0);
        preferences.end();

        // 释放互斥锁
        xSemaphoreGive(preferencesMutex);

        // 打印出值
        Serial.print("QuickKey: ");
        Serial.print(quickKey.key1);
        Serial.print(", ");
        Serial.print(quickKey.key2);
        Serial.print(", ");
        Serial.print(quickKey.key3);
        Serial.print(", ");
        Serial.print(quickKey.key4);
        Serial.print(", ");
        Serial.print(quickKey.key5);
        Serial.print(", ");
        Serial.print(quickKey.key6);
        Serial.print(", ");
        Serial.print(quickKey.key7);
        Serial.print(", ");
        Serial.print(quickKey.key8);
        Serial.print(", ");
        Serial.print(quickKey.key9);
        Serial.println();
        Serial.println("DataStore: LoadQuickKey() 执行完成");
    } else {
        Serial.println("DataStore::LoadQuickKey: 获取互斥锁超时");
    }
    
    return quickKey;
}

void DataStore::SaveSystemConfig(SystemConfig systemConfig)
{
    // 检查互斥锁是否有效
    if (preferencesMutex == nullptr) {
        Serial.println("DataStore::SaveSystemConfig: 互斥锁未初始化");
        return;
    }
    // 获取互斥锁，增加超时保护
    if (xSemaphoreTake(preferencesMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        preferences.begin(KEY_NAMESPACE);

        preferences.putBool(KEY_BUZZER_ENABLE, systemConfig.buzzerEnable);
        preferences.putInt(KEY_REPEAT_TRANSMIT, systemConfig.repeatTransmit);
        preferences.putInt(KEY_BRIGHTNESS, systemConfig.brightness);
        preferences.putLong(KEY_AUTO_SLEEP_TIME, systemConfig.autoSleepTime);
        preferences.putLong(KEY_AUTO_SCREEN_OFF_TIME, systemConfig.autoScreenOffTime);
        preferences.putBool(KEY_AP_ENABLED, systemConfig.APEnabled);
        preferences.putBool(KEY_WIFI_ENABLED, systemConfig.WifiEnabled);
        preferences.putString(KEY_AP_NAME, systemConfig.APName);
        preferences.putString(KEY_AP_PASSWORD, systemConfig.APPassword);
        preferences.putString(KEY_WIFI_NAME, systemConfig.WifiName);
        preferences.putString(KEY_WIFI_PASSWORD, systemConfig.WifiPassword);
        preferences.end();

        // 释放互斥锁
        xSemaphoreGive(preferencesMutex);

        // 打印出值
        Serial.print("SystemConfig: ");
        Serial.print(systemConfig.buzzerEnable);
        Serial.print(", ");
        Serial.print(systemConfig.repeatTransmit);
        Serial.print(", ");
        Serial.print(systemConfig.brightness);
        Serial.print(", ");
        Serial.print(systemConfig.autoSleepTime);
        Serial.print(", ");
        Serial.print(systemConfig.autoScreenOffTime);
        Serial.print(", ");
        Serial.print(systemConfig.APEnabled);
        Serial.print(", ");
        Serial.print(systemConfig.WifiEnabled);
        Serial.print(", ");
        Serial.print(systemConfig.APName);
        Serial.print(", ");
        Serial.print(systemConfig.APPassword);
        Serial.print(", ");
        Serial.print(systemConfig.WifiName);
        Serial.print(", ");
        Serial.print(systemConfig.WifiPassword);
        Serial.println();
    } else {
        Serial.println("DataStore::SaveSystemConfig: 获取互斥锁超时");
    }
    Serial.println("DataStore: SaveSystemConfig() 执行完成");
}

SystemConfig DataStore::LoadSystemConfig()
{
    SystemConfig systemConfig = {0}; // 初始化为0
    
    // 检查互斥锁是否有效
    if (preferencesMutex == nullptr) {
        Serial.println("DataStore::LoadSystemConfig: 互斥锁未初始化");
        return systemConfig;
    }
    // 获取互斥锁，增加超时保护
    if (xSemaphoreTake(preferencesMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        preferences.begin(KEY_NAMESPACE);

        systemConfig.buzzerEnable = preferences.getBool(KEY_BUZZER_ENABLE, true);
        systemConfig.repeatTransmit = preferences.getInt(KEY_REPEAT_TRANSMIT, 15);
        systemConfig.brightness = preferences.getInt(KEY_BRIGHTNESS, 100);
        systemConfig.autoSleepTime = preferences.getLong(KEY_AUTO_SLEEP_TIME, 0);
        systemConfig.autoScreenOffTime = preferences.getLong(KEY_AUTO_SCREEN_OFF_TIME, 0);
        systemConfig.APEnabled = preferences.getBool(KEY_AP_ENABLED, false);
        systemConfig.WifiEnabled = preferences.getBool(KEY_WIFI_ENABLED, false);
        systemConfig.APName = preferences.getString(KEY_AP_NAME, "MYNOVA_RFC");
        systemConfig.APPassword = preferences.getString(KEY_AP_PASSWORD, "MYNOVA123");
        systemConfig.WifiName = preferences.getString(KEY_WIFI_NAME, "");
        systemConfig.WifiPassword = preferences.getString(KEY_WIFI_PASSWORD, "");
        preferences.end();

        // 释放互斥锁
        xSemaphoreGive(preferencesMutex);

        // 打印出值
        Serial.print("SystemConfig: ");
        Serial.print(systemConfig.buzzerEnable);
        Serial.print(", ");
        Serial.print(systemConfig.repeatTransmit);
        Serial.print(", ");
        Serial.print(systemConfig.brightness);
        Serial.print(", ");
        Serial.print(systemConfig.autoSleepTime);
        Serial.print(", ");
        Serial.print(systemConfig.autoScreenOffTime);
        Serial.print(", ");
        Serial.print(systemConfig.APEnabled);
        Serial.print(", ");
        Serial.print(systemConfig.WifiEnabled);
        Serial.print(", ");
        Serial.print(systemConfig.APName);
        Serial.print(", ");
        Serial.print(systemConfig.APPassword);
        Serial.print(", ");
        Serial.print(systemConfig.WifiName);
        Serial.print(", ");
        Serial.print(systemConfig.WifiPassword);
        Serial.println();
        Serial.println("DataStore: LoadSystemConfig() 执行完成");
    } else {
        Serial.println("DataStore::LoadSystemConfig: 获取互斥锁超时");
    }
    
    return systemConfig;
}

void DataStore::ClearAllData()
{
    // 检查互斥锁是否有效
    if (preferencesMutex == nullptr) {
        Serial.println("DataStore::ClearAllData: 互斥锁未初始化");
        return;
    }
    
    // 获取互斥锁，增加超时保护
    if (xSemaphoreTake(preferencesMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        Serial.println("DataStore: ClearAllData() 开始清空所有数据");
        
        preferences.begin(KEY_NAMESPACE);
        
        // 清空所有数据
        preferences.clear();
        
        preferences.end();
        
        // 释放互斥锁
        xSemaphoreGive(preferencesMutex);
        
        Serial.println("DataStore: ClearAllData() 执行完成");
    } else {
        Serial.println("DataStore::ClearAllData: 获取互斥锁超时");
    }
}

bool DataStore::SaveWiFiConfig(const String& ssid, const String& password)
{
    // 检查互斥锁是否有效
    if (preferencesMutex == nullptr) {
        Serial.println("DataStore::SaveWiFiConfig: 互斥锁未初始化");
        return false;
    }
    
    // 获取互斥锁，增加超时保护
    if (xSemaphoreTake(preferencesMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        bool success = preferences.begin(KEY_NAMESPACE, false);
        
        if (!success) {
            Serial.println("DataStore::SaveWiFiConfig: 打开Preferences失败");
            xSemaphoreGive(preferencesMutex);
            return false;
        }
        
        preferences.putString(KEY_WIFI_NAME, ssid);
        preferences.putString(KEY_WIFI_PASSWORD, password);
        preferences.end();
        
        // 释放互斥锁
        xSemaphoreGive(preferencesMutex);
        
        Serial.println("DataStore: WiFi配置已保存");
        return true;
    } else {
        Serial.println("DataStore::SaveWiFiConfig: 获取互斥锁超时");
        return false;
    }
}

bool DataStore::LoadWiFiConfig(String& ssid, String& password)
{
    // 检查互斥锁是否有效
    if (preferencesMutex == nullptr) {
        Serial.println("DataStore::LoadWiFiConfig: 互斥锁未初始化");
        return false;
    }
    
    // 获取互斥锁，增加超时保护
    if (xSemaphoreTake(preferencesMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        bool success = preferences.begin(KEY_NAMESPACE, true);
        
        if (!success) {
            Serial.println("DataStore::LoadWiFiConfig: 打开Preferences失败");
            xSemaphoreGive(preferencesMutex);
            return false;
        }
        
        ssid = preferences.getString(KEY_WIFI_NAME, "");
        password = preferences.getString(KEY_WIFI_PASSWORD, "");
        preferences.end();
        
        // 释放互斥锁
        xSemaphoreGive(preferencesMutex);
        
        if (ssid.length() > 0) {
            Serial.println("DataStore: WiFi配置已加载");
            return true;
        }
        
        return false;
    } else {
        Serial.println("DataStore::LoadWiFiConfig: 获取互斥锁超时");
        return false;
    }
}

void DataStore::ClearWiFiConfig()
{
    // 检查互斥锁是否有效
    if (preferencesMutex == nullptr) {
        Serial.println("DataStore::ClearWiFiConfig: 互斥锁未初始化");
        return;
    }
    
    // 获取互斥锁，增加超时保护
    if (xSemaphoreTake(preferencesMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        bool success = preferences.begin(KEY_NAMESPACE, false);
        
        if (!success) {
            Serial.println("DataStore::ClearWiFiConfig: 打开Preferences失败");
            xSemaphoreGive(preferencesMutex);
            return;
        }
        
        preferences.putString(KEY_WIFI_NAME, "");
        preferences.putString(KEY_WIFI_PASSWORD, "");
        preferences.end();
        
        // 释放互斥锁
        xSemaphoreGive(preferencesMutex);
        
        Serial.println("DataStore: WiFi配置已清除");
    } else {
        Serial.println("DataStore::ClearWiFiConfig: 获取互斥锁超时");
    }
}

// ==================== MQTT配置相关 ====================

bool DataStore::SaveMQTTConfig(MQTTMode mode, const String& server, uint16_t port, const String& username, const String& password)
{
    // 检查互斥锁是否有效
    if (preferencesMutex == nullptr) {
        Serial.println("DataStore::SaveMQTTConfig: 互斥锁未初始化");
        return false;
    }
    
    // 获取互斥锁，增加超时保护
    if (xSemaphoreTake(preferencesMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        bool success = preferences.begin(MQTT_NAMESPACE, false);
        
        if (!success) {
            Serial.println("DataStore::SaveMQTTConfig: 打开Preferences失败");
            xSemaphoreGive(preferencesMutex);
            return false;
        }
        
        String modeStr = mqttModeToString(mode);
        preferences.putString(MQTT_KEY_MODE, modeStr);
        preferences.putString(MQTT_KEY_SERVER, server);
        preferences.putUShort(MQTT_KEY_PORT, port);
        preferences.putString(MQTT_KEY_USERNAME, username);
        preferences.putString(MQTT_KEY_PASSWORD, password);
        preferences.end();
        
        // 释放互斥锁
        xSemaphoreGive(preferencesMutex);
        
        Serial.println("DataStore: MQTT配置已保存");
        Serial.printf("  模式: %s\n", modeStr.c_str());
        if (mode == MQTTMode::MANUAL && server.length() > 0) {
            Serial.printf("  服务器: %s:%d\n", server.c_str(), port);
        }
        if (username.length() > 0) {
            Serial.printf("  用户名: %s\n", username.c_str());
        }
        return true;
    } else {
        Serial.println("DataStore::SaveMQTTConfig: 获取互斥锁超时");
        return false;
    }
}

bool DataStore::LoadMQTTConfig(MQTTMode& mode, String& server, uint16_t& port, String& username, String& password)
{
    // 检查互斥锁是否有效
    if (preferencesMutex == nullptr) {
        Serial.println("DataStore::LoadMQTTConfig: 互斥锁未初始化");
        return false;
    }
    
    // 获取互斥锁，增加超时保护
    if (xSemaphoreTake(preferencesMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        bool success = preferences.begin(MQTT_NAMESPACE, true);
        
        if (!success) {
            Serial.println("DataStore::LoadMQTTConfig: 打开Preferences失败");
            xSemaphoreGive(preferencesMutex);
            return false;
        }
        
        String modeStr = preferences.getString(MQTT_KEY_MODE, "auto");
        mode = stringToMQTTMode(modeStr);
        server = preferences.getString(MQTT_KEY_SERVER, "");
        port = preferences.getUShort(MQTT_KEY_PORT, 1883);
        username = preferences.getString(MQTT_KEY_USERNAME, "");
        password = preferences.getString(MQTT_KEY_PASSWORD, "");
        preferences.end();
        
        // 释放互斥锁
        xSemaphoreGive(preferencesMutex);
        
        // 只要有模式配置就认为有配置（auto 或 manual）
        if (modeStr.length() > 0) {
            Serial.println("DataStore: MQTT配置已加载");
            Serial.printf("  模式: %s\n", modeStr.c_str());
            if (mode == MQTTMode::MANUAL && server.length() > 0) {
                Serial.printf("  服务器: %s:%d\n", server.c_str(), port);
            }
            if (username.length() > 0) {
                Serial.printf("  用户名: %s\n", username.c_str());
            }
            return true;
        }
        
        return false;
    } else {
        Serial.println("DataStore::LoadMQTTConfig: 获取互斥锁超时");
        return false;
    }
}

void DataStore::ClearMQTTConfig()
{
    // 检查互斥锁是否有效
    if (preferencesMutex == nullptr) {
        Serial.println("DataStore::ClearMQTTConfig: 互斥锁未初始化");
        return;
    }
    
    // 获取互斥锁，增加超时保护
    if (xSemaphoreTake(preferencesMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        bool success = preferences.begin(MQTT_NAMESPACE, false);
        
        if (!success) {
            Serial.println("DataStore::ClearMQTTConfig: 打开Preferences失败");
            xSemaphoreGive(preferencesMutex);
            return;
        }
        
        preferences.clear();
        preferences.end();
        
        // 释放互斥锁
        xSemaphoreGive(preferencesMutex);
        
        Serial.println("DataStore: MQTT配置已清除");
    } else {
        Serial.println("DataStore::ClearMQTTConfig: 获取互斥锁超时");
    }
}

bool DataStore::HasMQTTConfig()
{
    MQTTMode mode;
    String server;
    uint16_t port;
    String username;
    String password;
    
    return LoadMQTTConfig(mode, server, port, username, password);
}