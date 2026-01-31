/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "HAManager.h"
#include <ArduinoJson.h>

// 静态实例指针，用于回调函数
static HAManager* g_haManagerInstance = nullptr;

HAManager::HAManager() : mqttClient(wifiClient) {
    pDataStore = nullptr;
    pRadioHelper = nullptr;
    pBatteryManager = nullptr;
    mqttPort = 1883;
    lastReconnectAttempt = 0;
    lastDiscoveryPublish = 0;
    lastBatteryPublish = 0;
    
    // 设置默认设备信息
    deviceName = "MYNOVA RFC";
    deviceModel = "RF Remote Controller";
    deviceManufacturer = "MYNOVA";
    deviceSWVersion = "1.0.0";
    
    g_haManagerInstance = this;
}

void HAManager::init(DataStore* dataStore, RadioHelper* radioHelper) {
    pDataStore = dataStore;
    pRadioHelper = radioHelper;
    
    // 生成唯一设备ID（基于MAC地址）
    deviceID = generateDeviceID();
    
    // 设置MQTT主题前缀
    topicPrefix = "homeassistant/button/mynova_rfc_" + deviceID;
    availabilityTopic = topicPrefix + "/availability";
    
    Serial.println("HAManager: 初始化完成");
    Serial.println("设备ID: " + deviceID);
    Serial.println("主题前缀: " + topicPrefix);
    
    // 初始化mDNS
    if (!MDNS.begin("mynova_rfc")) {
        Serial.println("HAManager: mDNS初始化失败");
    } else {
        Serial.println("HAManager: mDNS初始化成功");
        MDNS.addService("http", "tcp", 80);  // 广播Web服务
    }
    
    // 检查是否有保存的MQTT配置
    if (pDataStore && pDataStore->HasMQTTConfig()) {
        Serial.println("HAManager: 检测到已保存的MQTT配置");
    }
}

void HAManager::setDeviceInfo(const char* name, const char* model, 
                              const char* manufacturer, const char* swVersion) {
    deviceName = name;
    deviceModel = model;
    deviceManufacturer = manufacturer;
    deviceSWVersion = swVersion;
}

void HAManager::setBatteryManager(BatteryManager* batteryManager) {
    pBatteryManager = batteryManager;
}

bool HAManager::connectAuto() {
    // 检查WiFi连接
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("HAManager: WiFi未连接，无法连接MQTT");
        return false;
    }
    
    if (!pDataStore) {
        Serial.println("HAManager: DataStore未初始化");
        return false;
    }
    
    Serial.println("==================================================");
    Serial.println("HAManager: 开始自动连接流程");
    Serial.println("==================================================");
    
    // 加载配置
    MQTTMode mode;
    String server;
    uint16_t port;
    String username;
    String password;
    
    if (!pDataStore->LoadMQTTConfig(mode, server, port, username, password)) {
        Serial.println("✗ 未找到MQTT配置");
        Serial.println("提示：请通过Web界面配置 Home Assistant");
        Serial.println("==================================================");
        return false;
    }
    
    Serial.printf("配置模式: %s\n", mode == MQTTMode::AUTO ? "auto" : "manual");
    
    // 根据模式决定连接方式
    if (mode == MQTTMode::AUTO) {
        // 自动模式：使用 mDNS 发现
        Serial.println("步骤1: 使用mDNS自动发现...");
        String discoveredServer;
        uint16_t discoveredPort;
        
        if (discoverMQTTServer(discoveredServer, discoveredPort)) {
            Serial.println("✓ mDNS发现成功！");
            Serial.printf("  服务器: %s:%d\n", discoveredServer.c_str(), discoveredPort);
            
            if (discoveredServer.length() > 0 && discoveredServer != "0.0.0.0") {
                // 使用发现的服务器 + 配置的用户名密码
                if (connect(discoveredServer.c_str(), discoveredPort,
                           username.length() > 0 ? username.c_str() : nullptr,
                           password.length() > 0 ? password.c_str() : nullptr,
                           false)) {
                    Serial.println("✓ 自动发现并连接成功！");
                    Serial.println("==================================================");
                    return true;
                } else {
                    Serial.println("✗ 连接失败，请检查用户名密码");
                }
            }
        } else {
            Serial.println("✗ mDNS发现失败");
        }
    } else if (mode == MQTTMode::MANUAL) {
        // 手动模式：使用配置的服务器地址
        Serial.println("步骤1: 使用手动配置的服务器...");
        
        if (server.length() > 0) {
            Serial.printf("  服务器: %s:%d\n", server.c_str(), port);
            
            if (connect(server.c_str(), port,
                       username.length() > 0 ? username.c_str() : nullptr,
                       password.length() > 0 ? password.c_str() : nullptr,
                       false)) {
                Serial.println("✓ 连接成功！");
                Serial.println("==================================================");
                return true;
            } else {
                Serial.println("✗ 连接失败，请检查服务器地址和认证信息");
            }
        } else {
            Serial.println("✗ 未配置服务器地址");
        }
    }
    
    Serial.println("==================================================");
    Serial.println("✗ 连接失败");
    Serial.println("提示：请检查 Home Assistant 配置");
    Serial.println("  1. 确认 Mosquitto broker 已启动");
    Serial.println("  2. 检查用户名密码是否正确");
    Serial.println("  3. 或在 Mosquitto 中启用匿名访问");
    Serial.println("==================================================");
    
    return false;
}

bool HAManager::connect() {
    // connect() 现在调用 connectAuto()
    return connectAuto();
}

bool HAManager::discoverMQTTServer(String& server, uint16_t& port) {
    Serial.println("--------------------");
    Serial.println("开始mDNS服务发现");
    Serial.println("--------------------");
    
    // mDNS查询需要时间，等待一下
    delay(500);
    
    // 策略1: 查询标准MQTT服务 (_mqtt._tcp)
    Serial.println("[1/3] 查询标准MQTT服务: _mqtt._tcp");
    int n = MDNS.queryService("mqtt", "tcp");
    if (n > 0) {
        Serial.printf("  ✓ 找到 %d 个MQTT服务\n", n);
        // ESP32 mDNS API: 使用 address() 或 IP()
        IPAddress ip = MDNS.address(0);
        if (ip != IPAddress(0, 0, 0, 0)) {
            server = ip.toString();
            port = MDNS.port(0);
            Serial.printf("  使用: %s:%d\n", server.c_str(), port);
            return true;
        }
    }
    Serial.println("  ✗ 未找到");
    
    // 策略2: 直接解析 homeassistant.local
    Serial.println("[2/3] 解析 homeassistant.local");
    IPAddress haIP = MDNS.queryHost("homeassistant");
    if (haIP != IPAddress(0, 0, 0, 0)) {
        server = haIP.toString();
        port = 1883;  // 使用默认MQTT端口
        Serial.printf("  ✓ 解析成功: %s:%d\n", server.c_str(), port);
        return true;
    }
    Serial.println("  ✗ 解析失败");
    
    // 策略3: 查询Home Assistant HTTP服务，假设MQTT在同一主机
    Serial.println("[3/3] 查询Home Assistant HTTP服务");
    n = MDNS.queryService("http", "tcp");
    if (n > 0) {
        Serial.printf("  找到 %d 个HTTP服务\n", n);
        for (int i = 0; i < n; i++) {
            String hostname = MDNS.hostname(i);
            Serial.printf("  服务 %d: %s\n", i + 1, hostname.c_str());
            
            // 检查是否是Home Assistant
            if (hostname.indexOf("homeassistant") >= 0 || 
                hostname.indexOf("hassio") >= 0) {
                IPAddress ip = MDNS.address(i);
                if (ip != IPAddress(0, 0, 0, 0)) {
                    server = ip.toString();
                    port = 1883;  // 假设MQTT在同一主机的1883端口
                    Serial.printf("  ✓ 疑似HA服务: %s:%d\n", server.c_str(), port);
                    return true;
                }
            }
        }
        Serial.println("  ✗ 未找到HA相关服务");
    } else {
        Serial.println("  ✗ 未找到HTTP服务");
    }
    
    Serial.println("--------------------");
    Serial.println("mDNS发现失败");
    Serial.println("--------------------");
    return false;
}

bool HAManager::connect(const char* haServerIP, uint16_t port, 
                        const char* username, const char* password,
                        bool saveConfig) {
    mqttServer = haServerIP;
    mqttPort = port;
    
    if (username != nullptr) {
        mqttUsername = username;
    } else {
        mqttUsername = "";
    }
    
    if (password != nullptr) {
        mqttPassword = password;
    } else {
        mqttPassword = "";
    }
    
    // 保存配置到DataStore（如果需要）
    if (saveConfig && pDataStore) {
        pDataStore->SaveMQTTConfig(MQTTMode::MANUAL, mqttServer, mqttPort, mqttUsername, mqttPassword);
    }
    
    mqttClient.setServer(haServerIP, port);
    mqttClient.setCallback(mqttCallback);
    mqttClient.setBufferSize(2048);  // 增加缓冲区以支持Discovery消息
    
    Serial.print("HAManager: 连接到 ");
    Serial.print(haServerIP);
    Serial.print(":");
    Serial.println(port);
    
    return reconnect();
}

bool HAManager::reconnect() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("HAManager: WiFi未连接");
        return false;
    }
    
    String clientId = "mynova_rfc_" + deviceID;
    
    bool connected = false;
    if (mqttUsername.length() > 0) {
        connected = mqttClient.connect(clientId.c_str(), 
                                       mqttUsername.c_str(), 
                                       mqttPassword.c_str(),
                                       availabilityTopic.c_str(),
                                       0, true, "offline");
    } else {
        connected = mqttClient.connect(clientId.c_str(),
                                       availabilityTopic.c_str(),
                                       0, true, "offline");
    }
    
    if (connected) {
        Serial.println("HAManager: MQTT连接成功");
        
        // 发布在线状态
        publishAvailability(true);
        
        // 订阅命令主题
        String commandTopic = topicPrefix + "/+/command";
        mqttClient.subscribe(commandTopic.c_str());
        Serial.println("订阅主题: " + commandTopic);
        
        // 发布Discovery配置
        publishDiscovery();
        
        return true;
    } else {
        Serial.print("HAManager: MQTT连接失败，错误代码: ");
        Serial.println(mqttClient.state());
        return false;
    }
}

void HAManager::disconnect() {
    if (mqttClient.connected()) {
        publishAvailability(false);
        mqttClient.disconnect();
        Serial.println("HAManager: 已断开MQTT连接");
    }
}

void HAManager::loop() {
    if (!mqttClient.connected()) {
        unsigned long now = millis();
        // 每5秒尝试重连一次
        if (now - lastReconnectAttempt > 5000) {
            lastReconnectAttempt = now;
            if (reconnect()) {
                lastReconnectAttempt = 0;
            }
        }
    } else {
        mqttClient.loop();
        
        unsigned long now = millis();
        
        // 每10分钟重新发布Discovery（确保HA重启后也能发现设备）
        if (now - lastDiscoveryPublish > 600000) {
            publishDiscovery();
            lastDiscoveryPublish = now;
        }
        
        // 每30秒发布电池电量
        if (now - lastBatteryPublish > 30000) {
            publishBatteryState();
            lastBatteryPublish = now;
        }
    }
}

bool HAManager::isConnected() {
    return mqttClient.connected();
}

void HAManager::publishAvailability(bool online) {
    if (!mqttClient.connected()) return;
    
    const char* status = online ? "online" : "offline";
    mqttClient.publish(availabilityTopic.c_str(), status, true);
    
    Serial.print("HAManager: 发布状态 - ");
    Serial.println(status);
}

void HAManager::publishDiscovery() {
    if (!pDataStore) {
        Serial.println("HAManager: DataStore未初始化");
        return;
    }
    
    if (!mqttClient.connected()) {
        Serial.println("HAManager: MQTT未连接");
        return;
    }
    
    Serial.println("HAManager: 开始发布Discovery配置");
    
    int publishedCount = 0;
    
    // 遍历所有数据位置（1-100）
    for (int i = 1; i <= 100; i++) {
        RadioData radioData = pDataStore->ReadData(i);
        
        // 只发布有效数据
        if (radioData.name.length() > 0) {
            publishButtonDiscovery(i, radioData);
            publishedCount++;
            mqttClient.loop();
            delay(50);
        }
    }
    
    // 发布电池传感器Discovery配置
    publishBatteryDiscovery();
    
    lastDiscoveryPublish = millis();
    
    Serial.print("HAManager: 已发布 ");
    Serial.print(publishedCount);
    Serial.println(" 个按钮配置");
}

void HAManager::publishButtonDiscovery(int index, const RadioData& data) {
    // Discovery主题
    String configTopic = "homeassistant/button/mynova_rfc_" + deviceID + 
                        "/button_" + String(index) + "/config";
    
    // 命令主题
    String commandTopic = topicPrefix + "/button_" + String(index) + "/command";
    
    // 创建Discovery配置JSON
    JsonDocument doc;
    
    // 基本信息
    doc["name"] = data.name;
    doc["unique_id"] = "mynova_rfc_" + deviceID + "_button_" + String(index);
    doc["command_topic"] = commandTopic;
    
    // 可用性主题
    doc["availability_topic"] = availabilityTopic;
    
    // 设备信息
    JsonObject device = doc["device"].to<JsonObject>();
    device["identifiers"][0] = deviceID;
    device["name"] = deviceName;
    device["model"] = deviceModel;
    device["manufacturer"] = deviceManufacturer;
    device["sw_version"] = deviceSWVersion;
    
    // 图标（根据频率类型选择）
    if (data.rcData.freqType == FREQ_315) {
        doc["icon"] = "mdi:remote";
    } else {
        doc["icon"] = "mdi:remote-tv";
    }
    
    // 序列化JSON
    String payload;
    serializeJson(doc, payload);
    
    // 发布配置
    bool success = mqttClient.publish(configTopic.c_str(), payload.c_str(), true);
    
    if (success) {
        Serial.print("  [");
        Serial.print(index);
        Serial.print("] ");
        Serial.println(data.name);
    } else {
        Serial.print("  [");
        Serial.print(index);
        Serial.println("] 发布失败");
    }
}

String HAManager::generateDeviceID() {
    // 使用MAC地址生成唯一ID
    uint8_t mac[6];
    WiFi.macAddress(mac);
    
    char macStr[13];
    sprintf(macStr, "%02X%02X%02X%02X%02X%02X", 
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    return String(macStr);
}

void HAManager::mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (g_haManagerInstance != nullptr) {
        String topicStr = String(topic);
        String payloadStr = "";
        
        for (unsigned int i = 0; i < length; i++) {
            payloadStr += (char)payload[i];
        }
        
        g_haManagerInstance->handleCommand(topicStr, payloadStr);
    }
}

bool HAManager::hasSavedConfig() {
    if (!pDataStore) {
        return false;
    }
    return pDataStore->HasMQTTConfig();
}

void HAManager::handleCommand(String topic, String payload) {
    Serial.print("收到命令 - 主题: ");
    Serial.print(topic);
    Serial.print(", 内容: ");
    Serial.println(payload);
    
    // 解析主题，提取按钮索引
    // 格式: homeassistant/button/mynova_rfc_XXXXXX/button_N/command
    int buttonStartPos = topic.indexOf("button_");
    if (buttonStartPos == -1) {
        Serial.println("无效的主题格式");
        return;
    }
    
    int buttonEndPos = topic.indexOf("/", buttonStartPos);
    if (buttonEndPos == -1) {
        buttonEndPos = topic.length();
    }
    
    String buttonIndexStr = topic.substring(buttonStartPos + 7, buttonEndPos);
    int buttonIndex = buttonIndexStr.toInt();
    
    if (buttonIndex < 1 || buttonIndex > 100) {
        Serial.println("无效的按钮索引: " + String(buttonIndex));
        return;
    }
    
    // 如果payload是"PRESS"，发送射频信号
    if (payload == "PRESS") {
        // 从DataStore读取数据
        if (pDataStore) {
            RadioData radioData = pDataStore->ReadData(buttonIndex);
            
            if (radioData.name.length() > 0) {
                Serial.print("发送射频信号: [");
                Serial.print(buttonIndex);
                Serial.print("] ");
                Serial.print(radioData.name);
                Serial.print(" | 数据: ");
                Serial.print((unsigned long)radioData.rcData.data);
                Serial.print(" | 频率: ");
                Serial.println(radioData.rcData.freqType == FREQ_315 ? "315MHz" : "433MHz");
                
                // 发送射频信号
                if (pRadioHelper) {
                    pRadioHelper->SendData(radioData.rcData);
                    Serial.println("射频信号发送成功");
                } else {
                    Serial.println("RadioHelper未初始化");
                }
            } else {
                Serial.println("按钮数据为空");
            }
        } else {
            Serial.println("DataStore未初始化");
        }
    }
}

void HAManager::publishBatteryDiscovery() {
    if (!mqttClient.connected()) {
        return;
    }
    
    String configTopic = "homeassistant/sensor/mynova_rfc_" + deviceID + "/battery/config";
    String stateTopic = topicPrefix + "/battery/state";
    
    JsonDocument doc;
    
    doc["name"] = "Battery";
    doc["unique_id"] = "mynova_rfc_" + deviceID + "_battery";
    doc["state_topic"] = stateTopic;
    doc["availability_topic"] = availabilityTopic;
    doc["unit_of_measurement"] = "%";
    doc["device_class"] = "battery";
    doc["state_class"] = "measurement";
    doc["icon"] = "mdi:battery";
    
    JsonObject device = doc["device"].to<JsonObject>();
    device["identifiers"][0] = deviceID;
    device["name"] = deviceName;
    device["model"] = deviceModel;
    device["manufacturer"] = deviceManufacturer;
    device["sw_version"] = deviceSWVersion;
    
    String payload;
    serializeJson(doc, payload);
    
    bool success = mqttClient.publish(configTopic.c_str(), payload.c_str(), true);
    
    if (success) {
        Serial.println("  [电池] 电池传感器Discovery配置已发布");
    } else {
        Serial.println("  [电池] 电池传感器Discovery配置发布失败");
    }
}

void HAManager::publishBatteryState() {
    if (!mqttClient.connected()) {
        return;
    }
    
    if (!pBatteryManager) {
        return;
    }
    
    float voltage = pBatteryManager->readVoltage();
    float percent = pBatteryManager->calculateBatteryPercent(voltage);
    
    String stateTopic = topicPrefix + "/battery/state";
    String payload = String(percent, 1);
    
    bool success = mqttClient.publish(stateTopic.c_str(), payload.c_str(), true);
    
    if (success) {
        Serial.print("HAManager: 电池电量已发布 - ");
        Serial.print(percent, 1);
        Serial.println("%");
    }
}
