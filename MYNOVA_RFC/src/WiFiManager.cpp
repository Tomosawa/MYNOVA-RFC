/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "WiFiManager.h"
#include "DataStore.h"
#include "HAManager.h"

extern DataStore dataStore;
extern HAManager haManager;
// 静态成员初始化
IPAddress WiFiManager::AP_IP(10, 0, 0, 1);
IPAddress WiFiManager::AP_GATEWAY(10, 0, 0, 1);
IPAddress WiFiManager::AP_SUBNET(255, 255, 255, 0);

WiFiManager::WiFiManager() : scanning(false), scanComplete(false), apStarted(false),
                              scanTaskHandle(NULL), wifiConnectTaskHandle(NULL), 
                              apSetupTaskHandle(NULL), connected(false) {
    mutex = xSemaphoreCreateMutex();
}

WiFiManager::~WiFiManager() {
    if (scanTaskHandle != NULL) {
        vTaskDelete(scanTaskHandle);
    }
    if (wifiConnectTaskHandle != NULL) {
        vTaskDelete(wifiConnectTaskHandle);
    }
    if (apSetupTaskHandle != NULL) {
        vTaskDelete(apSetupTaskHandle);
    }
    if (mutex != NULL) {
        vSemaphoreDelete(mutex);
    }
}

void WiFiManager::init() {
    Serial.println("WiFiManager: 初始化");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
    WiFi.disconnect();
    WiFi.setHostname("MYNOVA_RFC");
    delay(100);
    Serial.println("WiFiManager: 初始化完成");
}

// ==================== WiFi 扫描相关 ====================

void WiFiManager::startScan() {
    if (scanning) {
        Serial.println("WiFiManager: 扫描已在进行中");
        return;
    }
    
    xSemaphoreTake(mutex, portMAX_DELAY);
    scanning = true;
    scanComplete = false;
    scanResults.clear();
    xSemaphoreGive(mutex);
    
    xTaskCreate(
        scanTask,
        "WiFiScanTask",
        4096,
        this,
        1,
        &scanTaskHandle
    );
}

void WiFiManager::scanTask(void* parameter) {
    WiFiManager* manager = static_cast<WiFiManager*>(parameter);
    
    Serial.println("WiFiManager: 开始扫描WiFi...");
    int n = WiFi.scanNetworks();
    
    xSemaphoreTake(manager->mutex, portMAX_DELAY);
    
    if (n == 0) {
        Serial.println("WiFiManager: 未找到WiFi网络");
    } else {
        Serial.printf("WiFiManager: 找到 %d 个网络\n", n);
        manager->scanResults.clear();
        
        for (int i = 0; i < n; ++i) {
            WiFiScanResult result;
            result.ssid = WiFi.SSID(i);
            result.rssi = WiFi.RSSI(i);
            result.encryptionType = WiFi.encryptionType(i);
            result.isScanning = false;
            result.networksFound = n;
            
            manager->scanResults.push_back(result);
        }
    }
    
    manager->scanning = false;
    manager->scanComplete = true;
    
    xSemaphoreGive(manager->mutex);
    
    WiFi.scanDelete();
    
    manager->scanTaskHandle = NULL;
    vTaskDelete(NULL);
}

bool WiFiManager::isScanComplete() {
    xSemaphoreTake(mutex, portMAX_DELAY);
    bool complete = scanComplete;
    xSemaphoreGive(mutex);
    return complete;
}

String WiFiManager::getScanResultJson() {
    xSemaphoreTake(mutex, portMAX_DELAY);
    
    JsonDocument doc;
    JsonArray wifiArray = doc["WIFI_SCAN"].to<JsonArray>();
    
    for (const auto& result : scanResults) {
        JsonObject wifi = wifiArray.add<JsonObject>();
        wifi["SSID"] = result.ssid;
        wifi["RSSI"] = result.rssi;
        wifi["Encryption"] = (result.encryptionType == WIFI_AUTH_OPEN) ? "Open" : "Encrypted";
    }
    
    doc["scanning"] = scanning;
    doc["count"] = scanResults.size();
    
    xSemaphoreGive(mutex);
    
    String output;
    serializeJson(doc, output);
    return output;
}

// ==================== WiFi STA 连接相关 ====================

bool WiFiManager::connectToWiFi(const String& ssid, const String& password) {
    Serial.printf("WiFiManager: 连接WiFi: %s\n", ssid.c_str());
    
    WiFi.begin(ssid.c_str(), password.c_str());
    
    int attempts = 0;
    const int maxAttempts = 20; // 10秒超时
    
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFiManager: WiFi连接成功!");
        Serial.print("  IP地址: ");
        Serial.println(WiFi.localIP());
        
        xSemaphoreTake(mutex, portMAX_DELAY);
        currentSSID = ssid;
        currentPassword = password;
        connected = true;
        xSemaphoreGive(mutex);
        
        return true;
    } else {
        Serial.println("\nWiFiManager: WiFi连接失败!");
        return false;
    }
}

void WiFiManager::connectToWiFiAsync(const String& ssid, const String& password) {
    Serial.println("WiFiManager: 异步连接WiFi");
    
    // 如果已经有连接任务在运行，先停止它
    if (wifiConnectTaskHandle != NULL) {
        vTaskDelete(wifiConnectTaskHandle);
        wifiConnectTaskHandle = NULL;
    }
    
    // 保存连接参数
    xSemaphoreTake(mutex, portMAX_DELAY);
    currentSSID = ssid;
    currentPassword = password;
    xSemaphoreGive(mutex);
    
    // 创建WiFi连接任务
    xTaskCreatePinnedToCore(
        wifiConnectTask,
        "WiFiConnectTask",
        8192,
        this,
        1,
        &wifiConnectTaskHandle,
        1  // 固定到Core 1
    );
}

void WiFiManager::wifiConnectTask(void* parameter) {
    WiFiManager* manager = static_cast<WiFiManager*>(parameter);
    
    xSemaphoreTake(manager->mutex, portMAX_DELAY);
    String ssid = manager->currentSSID;
    String password = manager->currentPassword;
    xSemaphoreGive(manager->mutex);
    
    Serial.println("WiFiConnectTask: 开始连接WiFi");
    Serial.print("  SSID: "); Serial.println(ssid);
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    WiFi.begin(ssid.c_str(), password.c_str());
    
    // 等待连接（最多10秒）
    int timeout = 20;
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        vTaskDelay(pdMS_TO_TICKS(500));
        Serial.print(".");
        timeout--;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFiConnectTask: WiFi连接成功");
        Serial.print("  IP地址: "); Serial.println(WiFi.localIP());
        
        xSemaphoreTake(manager->mutex, portMAX_DELAY);
        manager->connected = true;
        xSemaphoreGive(manager->mutex);
        
         // 如果WiFi已连接且有MQTT配置，尝试连接到HA
        if (haManager.hasSavedConfig()) {
            Serial.println("Setup: 检测到MQTT配置，尝试连接到Home Assistant...");
            if (haManager.connectAuto()) {
                Serial.println("Setup: ✓ Home Assistant 连接成功！");
            } else {
                Serial.println("Setup: ✗ Home Assistant 连接失败");
                Serial.println("       请通过Web界面检查配置");
            }
        } else {
            Serial.println("Setup: 未配置Home Assistant");
            Serial.println("       请通过Web界面进行配置");
        }
 
    } else {
        Serial.println("\nWiFiConnectTask: WiFi连接失败");
        
        xSemaphoreTake(manager->mutex, portMAX_DELAY);
        manager->connected = false;
        xSemaphoreGive(manager->mutex);
    }
    
    manager->wifiConnectTaskHandle = NULL;
    vTaskDelete(NULL);
}

void WiFiManager::disconnect() {
    Serial.println("WiFiManager: 断开WiFi");
    
    // 如果有正在运行的连接任务，先停止它
    if (wifiConnectTaskHandle != NULL) {
        vTaskDelete(wifiConnectTaskHandle);
        wifiConnectTaskHandle = NULL;
    }
    
    WiFi.disconnect(true);
    
    xSemaphoreTake(mutex, portMAX_DELAY);
    connected = false;
    currentSSID = "";
    currentPassword = "";
    xSemaphoreGive(mutex);
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getConnectionInfo() {
    JsonDocument doc;
    
    if (WiFi.status() == WL_CONNECTED) {
        doc["connected"] = true;
        doc["ssid"] = WiFi.SSID();
        doc["ip"] = WiFi.localIP().toString();
        doc["rssi"] = WiFi.RSSI();
    } else {
        doc["connected"] = false;
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}

void WiFiManager::shutdown() {
    Serial.println("WiFiManager: 关闭WiFi");
    
    // 停止所有任务
    if (scanTaskHandle != NULL) {
        vTaskDelete(scanTaskHandle);
        scanTaskHandle = NULL;
    }
    if (wifiConnectTaskHandle != NULL) {
        vTaskDelete(wifiConnectTaskHandle);
        wifiConnectTaskHandle = NULL;
    }
    if (apSetupTaskHandle != NULL) {
        vTaskDelete(apSetupTaskHandle);
        apSetupTaskHandle = NULL;
    }
    
    // 停止AP
    if (apStarted) {
        WiFi.softAPdisconnect(true);
        apStarted = false;
    }
    
    // 断开WiFi并关闭
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    
    xSemaphoreTake(mutex, portMAX_DELAY);
    connected = false;
    xSemaphoreGive(mutex);
}

// ==================== AP 热点相关 ====================

void WiFiManager::startAPAsync(const String& ssid, const String& password) {
    Serial.println("WiFiManager: 异步启动AP热点");
    
    // 如果已经有AP启动任务在运行，先停止它
    if (apSetupTaskHandle != NULL) {
        vTaskDelete(apSetupTaskHandle);
        apSetupTaskHandle = NULL;
    }
    
    // 保存AP参数
    xSemaphoreTake(mutex, portMAX_DELAY);
    apSSID = ssid;
    apPassword = password;
    xSemaphoreGive(mutex);
    
    // 创建AP启动任务
    xTaskCreatePinnedToCore(
        apSetupTask,
        "APSetupTask",
        10240,
        this,
        1,
        &apSetupTaskHandle,
        1  // 固定到Core 1
    );
}

void WiFiManager::apSetupTask(void* parameter) {
    WiFiManager* manager = static_cast<WiFiManager*>(parameter);
    
    xSemaphoreTake(manager->mutex, portMAX_DELAY);
    String ssid = manager->apSSID;
    String password = manager->apPassword;
    xSemaphoreGive(manager->mutex);
    
    Serial.println("APSetupTask: 开始启动AP热点");
    Serial.print("  SSID: "); Serial.println(ssid);
    Serial.print("  密码: "); Serial.println(password);
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 确保是AP_STA模式
    WiFi.mode(WIFI_AP_STA);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 配置AP
    WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
    
    // 启动AP
    bool result = WiFi.softAP(ssid.c_str(), password.c_str());
    vTaskDelay(pdMS_TO_TICKS(100));
    
    if (result) {
        Serial.println("APSetupTask: AP热点启动成功");
        Serial.print("  IP地址: "); Serial.println(WiFi.softAPIP());
        
        xSemaphoreTake(manager->mutex, portMAX_DELAY);
        manager->apStarted = true;
        xSemaphoreGive(manager->mutex);
    } else {
        Serial.println("APSetupTask: AP热点启动失败");
        
        xSemaphoreTake(manager->mutex, portMAX_DELAY);
        manager->apStarted = false;
        xSemaphoreGive(manager->mutex);
    }
    
    manager->apSetupTaskHandle = NULL;
    vTaskDelete(NULL);
}

void WiFiManager::stopAP() {
    Serial.println("WiFiManager: 停止AP热点");
    
    // 如果有正在运行的AP启动任务，先停止它
    if (apSetupTaskHandle != NULL) {
        vTaskDelete(apSetupTaskHandle);
        apSetupTaskHandle = NULL;
    }
    
    WiFi.softAPdisconnect(true);
    
    xSemaphoreTake(mutex, portMAX_DELAY);
    apStarted = false;
    xSemaphoreGive(mutex);
}

bool WiFiManager::isAPStarted() {
    xSemaphoreTake(mutex, portMAX_DELAY);
    bool started = apStarted;
    xSemaphoreGive(mutex);
    return started;
}

String WiFiManager::getAPIP() {
    return WiFi.softAPIP().toString();
}

// ==================== 配置存储相关 ====================

bool WiFiManager::saveConfig(const String& ssid, const String& password) {
    return dataStore.SaveWiFiConfig(ssid, password);
}

bool WiFiManager::loadConfig(String& ssid, String& password) {
    return dataStore.LoadWiFiConfig(ssid, password);
}

void WiFiManager::clearConfig() {
    dataStore.ClearWiFiConfig();
}
