/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef __WEBSERVICE__H__
#define __WEBSERVICE__H__
#include <Arduino.h>
#include "ESPAsyncWebServer.h"
#include <DNSServer.h>
#include <SPIFFS.h>
#include <LittleFS.h>

#include "WiFiManager.h"
#include "DataStore.h"

class WebService
{
public:
    WebService();
    void init(WiFiManager *wifiMgr);
    void serverStart();
    void processRequest();

    // WIFI相关接口
    void handleWIFIScanRequest(AsyncWebServerRequest *request);
    void handleWIFISaveRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleWIFIClearRequest(AsyncWebServerRequest *request);
    void handleWIFIConnectRequest(AsyncWebServerRequest *request);
    void handleWIFIEnableRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleAPWIFIInfoRequest(AsyncWebServerRequest *request);
    void handleAPSaveRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleAPEnableRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    
    // 无线遥控数据管理接口
    void handleRadioDataListRequest(AsyncWebServerRequest *request);
    void handleRadioDataAddRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleRadioDataUpdateRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleRadioDataDeleteRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleRadioDataGetRequest(AsyncWebServerRequest *request);
    void handleRadioDataSendRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    
    // MQTT/HA配置接口
    void handleMQTTConfigGetRequest(AsyncWebServerRequest *request);
    void handleMQTTConfigSaveRequest(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void handleMQTTConfigClearRequest(AsyncWebServerRequest *request);
    void handleMQTTConnectRequest(AsyncWebServerRequest *request);
    void handleMQTTStatusRequest(AsyncWebServerRequest *request);
    
private:
    AsyncWebServer server;
    WiFiManager *pWiFiManager;
};

#endif