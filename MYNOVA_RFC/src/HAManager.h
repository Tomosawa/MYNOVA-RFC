/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef HAMANAGER_H
#define HAMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESPmDNS.h>
#include "DataStore.h"
#include "RadioHelper.h"
#include "BatteryManager.h"
/**
 * Home Assistant MQTT集成管理器
 * 通过MQTT协议与Home Assistant通信
 * 使用MQTT Discovery自动发现设备和实体
 */
class HAManager {
public:
    HAManager();
    
    /**
     * 初始化HA管理器
     * @param dataStore DataStore实例指针
     * @param radioHelper RadioHelper实例指针
     */
    void init(DataStore* dataStore, RadioHelper* radioHelper);
    
    /**
     * 设置BatteryManager实例
     * @param batteryManager BatteryManager实例指针
     */
    void setBatteryManager(class BatteryManager* batteryManager);
    
    /**
     * 自动发现并连接到Home Assistant MQTT服务器
     * 优先使用mDNS自动发现，失败则使用保存的配置
     * @return 是否连接成功
     */
    bool connectAuto();
    
    /**
     * 连接到Home Assistant MQTT服务器（使用DataStore中保存的配置）
     * @return 是否连接成功
     */
    bool connect();
    
    /**
     * 连接到Home Assistant MQTT服务器（手动指定参数）
     * @param haServerIP HA服务器IP地址
     * @param port MQTT端口（默认1883）
     * @param username MQTT用户名（可选）
     * @param password MQTT密码（可选）
     * @param saveConfig 是否保存配置到DataStore（默认true）
     * @return 是否连接成功
     */
    bool connect(const char* haServerIP, uint16_t port = 1883, 
                 const char* username = nullptr, const char* password = nullptr,
                 bool saveConfig = true);
    
    /**
     * 使用mDNS发现Home Assistant MQTT服务器
     * @param server 输出参数：发现的服务器IP
     * @param port 输出参数：发现的服务器端口
     * @return 是否发现成功
     */
    bool discoverMQTTServer(String& server, uint16_t& port);
    
    /**
     * 断开与HA的连接
     */
    void disconnect();
    
    /**
     * 循环处理MQTT消息
     * 需要在主循环中调用
     */
    void loop();
    
    /**
     * 检查是否已连接
     */
    bool isConnected();
    
    /**
     * 发布设备在线状态
     */
    void publishAvailability(bool online);
    
    /**
     * 发送MQTT Discovery配置
     * 自动在HA中创建所有射频按钮
     */
    void publishDiscovery();
    
    /**
     * 检查是否有保存的MQTT配置
     */
    bool hasSavedConfig();
    
    /**
     * 设置设备信息
     */
    void setDeviceInfo(const char* deviceName, const char* model, 
                      const char* manufacturer, const char* swVersion);

private:
    WiFiClient wifiClient;
    PubSubClient mqttClient;
    
    DataStore* pDataStore;
    RadioHelper* pRadioHelper;
    BatteryManager* pBatteryManager;
    
    // 设备信息
    String deviceName;
    String deviceModel;
    String deviceManufacturer;
    String deviceSWVersion;
    String deviceID;  // 唯一设备ID（基于MAC地址）
    
    // MQTT主题前缀
    String topicPrefix;
    String availabilityTopic;
    
    // 连接参数
    String mqttServer;
    uint16_t mqttPort;
    String mqttUsername;
    String mqttPassword;
    
    unsigned long lastReconnectAttempt;
    unsigned long lastDiscoveryPublish;
    unsigned long lastBatteryPublish;
    
    /**
     * MQTT回调函数
     */
    static void mqttCallback(char* topic, byte* payload, unsigned int length);
    
    /**
     * 处理接收到的MQTT命令
     */
    void handleCommand(String topic, String payload);
    
    /**
     * 发布单个射频按钮的Discovery配置
     */
    void publishButtonDiscovery(int index, const RadioData& data);
    
    /**
     * 发布电池传感器Discovery配置
     */
    void publishBatteryDiscovery();
    
    /**
     * 发布电池电量数据
     */
    void publishBatteryState();
    
    /**
     * 生成唯一设备ID
     */
    String generateDeviceID();
    
    /**
     * 重连MQTT服务器
     */
    bool reconnect();
};

#endif // HAMANAGER_H
