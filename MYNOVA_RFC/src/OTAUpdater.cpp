/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "OTAUpdater.h"
#include <HTTPClient.h>
#include <Update.h>
#include <WiFiClientSecure.h>

OTAUpdater::OTAUpdater() {
    lastError = "";
}

bool OTAUpdater::checkUpdate(String url, OTAInfo& info) {
    if (WiFi.status() != WL_CONNECTED) {
        lastError = "WiFi未连接";
        return false;
    }

    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure(); // 忽略SSL证书验证，简化操作
    client.setTimeout(10000); // 增加超时时间到10秒
    
    Serial.print("Checking update from: ");
    Serial.println(url);

    // 配置跟随重定向
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    
    if (!http.begin(client, url)) {
        lastError = "连接初始化失败";
        return false;
    }
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        lastError = "HTTP错误: " + String(httpCode) + " (" + http.errorToString(httpCode) + ")";
        http.end();
        return false;
    }
    
    String payload = http.getString();
    http.end();
    
    Serial.println("Update JSON: " + payload);
    
    // 解析JSON
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
        lastError = "JSON解析失败";
        Serial.println(error.c_str());
        return false;
    }
    
    info.version = doc["version"].as<String>();
    info.firmwareUrl = doc["url"].as<String>();
    info.firmwareSize = doc["filesize"].as<size_t>();
    info.firmwareCrc = doc["crc32"].as<String>();
    info.description = doc["description"].as<String>();
    
    // 处理Spiffs
    if (doc.containsKey("spiffs")) {
        JsonObject spiffs = doc["spiffs"];
        info.hasSpiffs = true;
        info.spiffsUrl = spiffs["url"].as<String>();
        info.spiffsSize = spiffs["filesize"].as<size_t>();
        info.spiffsCrc = spiffs["crc32"].as<String>();
    } else {
        info.hasSpiffs = false;
    }
    
    // 去除URL中的空格（如果有）
    info.firmwareUrl.trim();
    if (info.hasSpiffs) info.spiffsUrl.trim();
    
    return true;
}

bool OTAUpdater::performUpdate(String url, size_t size, int type, ProgressCallback callback) {
    if (WiFi.status() != WL_CONNECTED) {
        lastError = "WiFi未连接";
        return false;
    }
    
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(10000); // 设置超时 10s
    
    Serial.print("Downloading update from: ");
    Serial.println(url);
    
    // 配置跟随重定向
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    
    if (!http.begin(client, url)) {
        lastError = "连接失败";
        return false;
    }
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        lastError = "HTTP错误: " + String(httpCode);
        http.end();
        return false;
    }
    
    int len = http.getSize();
    // 如果服务器返回长度，优先使用服务器返回的长度，否则使用JSON中的长度
    if (len > 0) {
        size = len;
    }
    
    // 检查是否有足够的空间
    bool canBegin = Update.begin(size, type == 1 ? U_SPIFFS : U_FLASH);
    if (!canBegin) {
        lastError = "空间不足";
        http.end();
        return false;
    }
    
    WiFiClient* stream = http.getStreamPtr();
    size_t written = 0;
    size_t total = size;
    uint8_t buff[1024];
    
    while (http.connected() && (written < total)) {
        size_t available = stream->available();
        if (available) {
            int c = stream->readBytes(buff, ((available > sizeof(buff)) ? sizeof(buff) : available));
            size_t w = Update.write(buff, c);
            if (w > 0) {
                written += w;
                if (callback) {
                    if (!callback(written, total)) {
                        lastError = "用户取消更新";
                        http.end();
                        Update.end();
                        return false;
                    }
                }
            } else {
                // 写入失败
                break;
            }
        } else {
            // 没有数据，检查是否取消
            if (callback) {
                if (!callback(written, total)) {
                    lastError = "用户取消更新";
                    http.end();
                    Update.end();
                    return false;
                }
            }
            delay(10); // 稍微增加延迟，避免空转过快
        }
    }
    
    http.end();
    
    if (written != total) {
        Update.end();
        lastError = "下载不完整";
        return false;
    }
    
    if (!Update.end()) {
        lastError = "更新失败: " + String(Update.getError());
        return false;
    }
    
    return true;
}
