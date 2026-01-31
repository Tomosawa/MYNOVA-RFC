/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef OTAUpdater_h
#define OTAUpdater_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>
#include <WiFi.h>

struct OTAInfo {
    String version;
    String description;
    String firmwareUrl;
    size_t firmwareSize;
    String firmwareCrc; // CRC32 string from JSON
    
    bool hasSpiffs;
    String spiffsUrl;
    size_t spiffsSize;
    String spiffsCrc;
};

class OTAUpdater {
public:
    OTAUpdater();
    
    // Check for updates
    // Returns true if successful and populates info
    bool checkUpdate(String url, OTAInfo& info);
    
    // Callback for progress: (current, total)
    // Return false to abort update
    typedef std::function<bool(size_t, size_t)> ProgressCallback;
    
    // Perform update
    // type: 0 = Firmware (App), 1 = SPIFFS
    bool performUpdate(String url, size_t size, int type, ProgressCallback callback);
    
    String getLastError() { return lastError; }

private:
    String lastError;
};

#endif
