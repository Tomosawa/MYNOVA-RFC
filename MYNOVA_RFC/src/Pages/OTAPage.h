/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef OTAPage_h
#define OTAPage_h

#include "../GUI/UIPage.h"
#include "../GUI/Widget/UILabel.h"
#include "../GUI/Widget/UIProgressBar.h"
#include "../GUI/Widget/UITextArea.h"
#include "../GUI/Widget/UINavBar.h"
#include "../OTAUpdater.h"

enum OTAState {
    OTA_STATE_CHECK_WIFI,
    OTA_STATE_CHECKING,
    OTA_STATE_LATEST, // 新增：已是最新版本
    OTA_STATE_SHOW_INFO,
    OTA_STATE_UPDATING_APP,
    OTA_STATE_UPDATING_SPIFFS,
    OTA_STATE_DONE,
    OTA_STATE_ERROR
};

class OTAPage : public UIPage {
public:
    OTAPage();
    virtual ~OTAPage();
    
    void update() override;
    
    // 按钮事件
    void onButtonBack(void* context = nullptr) override;
    void onButtonEnter(void* context = nullptr) override;
    void onButton2(void* context = nullptr) override; // 上滚
    void onButton8(void* context = nullptr) override; // 下滚
    
    // 任务相关
    static void checkUpdateTask(void* parameter);
    static void performUpdateTask(void* parameter);
    
private:
    void initLayout();
    void updateUIState();
    
    OTAState currentState;
    OTAState lastState;
    
    UILabel* titleLabel;
    UILabel* statusLabel;
    UITextArea* descArea;
    UIProgressBar* progressBar;
    UINavBar* navBar;
    
    OTAUpdater otaUpdater;
    OTAInfo otaInfo;
    
    // 线程同步相关
    volatile bool taskRunning;
    volatile bool abortRequested; // 新增：请求中止标志
    volatile int progressPercent;
    String errorMessage;
    
    TaskHandle_t taskHandle;
};

#endif
