/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "OTAPage.h"
#include "../GUI/UIEngine.h"
#include "../WiFiManager.h"
#include "../GUIRender.h"
#include "../../Version.h"

extern UIEngine uiEngine;
extern WiFiManager wifiManager; // 假设有全局实例

// 替换为你的域名 Please replace with your domain
#define UPDATE_JSON_URL "https://mydomain.com/rfc/update.json" 

// 版本比较辅助函数
// 返回 1: v1 > v2
// 返回 -1: v1 < v2
// 返回 0: v1 == v2
int compareVersions(String v1, String v2) {
    int i = 0, j = 0;
    while (i < v1.length() || j < v2.length()) {
        int num1 = 0;
        while (i < v1.length() && v1[i] != '.') {
            num1 = num1 * 10 + (v1[i] - '0');
            i++;
        }
        
        int num2 = 0;
        while (j < v2.length() && v2[j] != '.') {
            num2 = num2 * 10 + (v2[j] - '0');
            j++;
        }
        
        if (num1 > num2) return 1;
        if (num1 < num2) return -1;
        
        i++;
        j++;
    }
    return 0;
}

OTAPage::OTAPage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) {
    currentState = OTA_STATE_CHECK_WIFI;
    lastState = OTA_STATE_ERROR; // 强制刷新
    taskRunning = false;
    abortRequested = false;
    progressPercent = 0;
    taskHandle = NULL;
    
    initLayout();
}

OTAPage::~OTAPage() {
    if (taskHandle != NULL) {
        vTaskDelete(taskHandle);
    }
}

void OTAPage::initLayout() {
    // 标题
    titleLabel = new UILabel();
    titleLabel->x = 0;
    titleLabel->y = 0;
    titleLabel->width = SCREEN_WIDTH;
    titleLabel->height = 12;
    titleLabel->label = "固件更新";
    titleLabel->textAlign = CENTER;
    titleLabel->verticalAlign = MIDDLE;
    addWidget(titleLabel);
    
    // 状态/信息显示区域
    statusLabel = new UILabel();
    statusLabel->x = 0;
    statusLabel->y = 20;
    statusLabel->width = SCREEN_WIDTH;
    statusLabel->height = 12;
    statusLabel->label = "";
    statusLabel->textAlign = CENTER;
    addWidget(statusLabel);
    
    // 描述文本区域
    descArea = new UITextArea();
    descArea->x = 2;
    descArea->y = 14;
    descArea->width = SCREEN_WIDTH - 4;
    descArea->height = SCREEN_HEIGHT - 14 - 14; // 减去标题和导航栏
    descArea->bVisible = false;
    addWidget(descArea);
    
    // 进度条
    progressBar = new UIProgressBar();
    progressBar->x = 10;
    progressBar->y = 38;
    progressBar->width = SCREEN_WIDTH - 20;
    progressBar->height = 14;
    progressBar->bShowText = true;
    progressBar->bVisible = false;
    addWidget(progressBar);
    
    // 导航栏
    navBar = new UINavBar(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH, 12);
    navBar->setLeftButtonText("返回");
    addWidget(navBar);
}

void OTAPage::update() {
    // 状态机处理
    if (currentState != lastState) {
        updateUIState();
        lastState = currentState;
    }

    // 检查取消请求是否已完成
    if (abortRequested && !taskRunning) {
        // 任务已结束，执行返回
        uiEngine.navigateBack();
        return;
    }
    
    // 更新进度条
    if (currentState == OTA_STATE_UPDATING_APP || currentState == OTA_STATE_UPDATING_SPIFFS) {
        progressBar->setValue(progressPercent);
    }
    
    // 检查WiFi连接 (仅在初始状态)
    if (currentState == OTA_STATE_CHECK_WIFI) {
        if (WiFi.status() == WL_CONNECTED) {
            currentState = OTA_STATE_CHECKING;
            // 启动检查任务
            if (!taskRunning) {
                taskRunning = true;
                xTaskCreate(checkUpdateTask, "CheckUpdate", 8192, this, 1, &taskHandle);
            }
        }
    }
}

void OTAPage::updateUIState() {
    switch (currentState) {
        case OTA_STATE_CHECK_WIFI:
            statusLabel->label = "请先连接WiFi";
            statusLabel->bVisible = true;
            descArea->bVisible = false;
            progressBar->bVisible = false;
            navBar->setLeftButtonText("返回");
            navBar->setRightButtonText("");
            break;
            
        case OTA_STATE_CHECKING:
            statusLabel->label = "正在检查更新...";
            statusLabel->bVisible = true;
            descArea->bVisible = false;
            progressBar->bVisible = false;
            navBar->setRightButtonText("");
            break;

        case OTA_STATE_LATEST:
            statusLabel->label = "当前已经是最新版";
            statusLabel->bVisible = true;
            descArea->bVisible = false;
            progressBar->bVisible = false;
            navBar->setLeftButtonText("返回");
            navBar->setRightButtonText("");
            break;
            
        case OTA_STATE_SHOW_INFO:
            statusLabel->bVisible = false; // 隐藏状态，显示描述
            descArea->bVisible = true;
            {
                String info = "最新版本: V" + otaInfo.version + "\n" + otaInfo.description;
                descArea->setText(info);
            }
            navBar->setLeftButtonText("返回");
            navBar->setRightButtonText("更新");
            break;
            
        case OTA_STATE_UPDATING_APP:
            statusLabel->label = "正在更新固件...";
            statusLabel->bVisible = true;
            descArea->bVisible = false;
            progressBar->bVisible = true;
            navBar->setLeftButtonText("取消");
            navBar->setRightButtonText("");
            break;
            
        case OTA_STATE_UPDATING_SPIFFS:
            statusLabel->label = "正在更新资源...";
            statusLabel->bVisible = true;
            descArea->bVisible = false;
            progressBar->bVisible = true;
            navBar->setLeftButtonText("取消");
            navBar->setRightButtonText("");
            break;
            
        case OTA_STATE_DONE:
            statusLabel->label = "更新完成!";
            statusLabel->bVisible = true;
            progressBar->bVisible = false;
            navBar->setLeftButtonText("");
            navBar->setRightButtonText("重启");
            break;
            
        case OTA_STATE_ERROR:
            statusLabel->label = "错误: " + errorMessage;
            statusLabel->bVisible = true;
            descArea->bVisible = false;
            progressBar->bVisible = false;
            navBar->setLeftButtonText("返回");
            navBar->setRightButtonText("重试");
            break;
    }
}

void OTAPage::checkUpdateTask(void* parameter) {
    OTAPage* page = (OTAPage*)parameter;
    
    if (page->otaUpdater.checkUpdate(UPDATE_JSON_URL, page->otaInfo)) {
        // 版本比较
        String currentVersion = FIRMWARE_VERSION;
        String remoteVersion = page->otaInfo.version;
        
        // 如果远程版本大于当前版本，则提示更新
        if (compareVersions(remoteVersion, currentVersion) > 0) {
            page->currentState = OTA_STATE_SHOW_INFO;
        } else {
            page->currentState = OTA_STATE_LATEST;
        }
    } else {
        page->errorMessage = page->otaUpdater.getLastError();
        page->currentState = OTA_STATE_ERROR;
    }
    
    page->taskRunning = false;
    page->taskHandle = NULL;
    vTaskDelete(NULL);
}

void OTAPage::performUpdateTask(void* parameter) {
    OTAPage* page = (OTAPage*)parameter;
    
    // 1. 更新固件
    page->currentState = OTA_STATE_UPDATING_APP;
    page->progressPercent = 0;
    
    bool success = page->otaUpdater.performUpdate(
        page->otaInfo.firmwareUrl, 
        page->otaInfo.firmwareSize, 
        0, // U_FLASH
        [page](size_t current, size_t total) {
            if (page->abortRequested) return false;
            if (total > 0) {
                page->progressPercent = (current * 100) / total;
            }
            return true;
        }
    );
    
    if (page->abortRequested) {
        page->taskRunning = false;
        page->taskHandle = NULL;
        page->currentState = OTA_STATE_ERROR; // 刷新状态
        vTaskDelete(NULL);
        return;
    }
    
    if (!success) {
        page->errorMessage = page->otaUpdater.getLastError();
        page->currentState = OTA_STATE_ERROR;
        page->taskRunning = false;
        page->taskHandle = NULL;
        vTaskDelete(NULL);
        return;
    }
    
    // 2. 更新SPIFFS (如果存在)
    if (page->otaInfo.hasSpiffs) {
        page->currentState = OTA_STATE_UPDATING_SPIFFS;
        page->progressPercent = 0;
        
        success = page->otaUpdater.performUpdate(
            page->otaInfo.spiffsUrl, 
            page->otaInfo.spiffsSize, 
            1, // U_SPIFFS
            [page](size_t current, size_t total) {
                if (page->abortRequested) return false;
                if (total > 0) {
                    page->progressPercent = (current * 100) / total;
                }
                return true;
            }
        );
        
        if (page->abortRequested) {
            page->taskRunning = false;
            page->taskHandle = NULL;
            page->currentState = OTA_STATE_ERROR; // 刷新状态
            vTaskDelete(NULL);
            return;
        }
        
        if (!success) {
            page->errorMessage = page->otaUpdater.getLastError();
            page->currentState = OTA_STATE_ERROR;
            page->taskRunning = false;
            page->taskHandle = NULL;
            vTaskDelete(NULL);
            return;
        }
    }
    
    page->currentState = OTA_STATE_DONE;
    page->taskRunning = false;
    page->taskHandle = NULL;
    vTaskDelete(NULL);
}

void OTAPage::onButtonBack(void* context) {
    if (currentState == OTA_STATE_DONE) return; // 完成状态只能重启
    
    navBar->showLeftBlink(1, 80, 80, [this]() {
        if (taskRunning) {
            // 如果任务正在运行，则请求中止
            abortRequested = true;
            statusLabel->label = "正在取消...";
            
            // 标记一个标志，指示用户想要退出
            // 在 update() 中检查，如果任务结束了且 abortRequested 为 true，则退出
        } else {
            uiEngine.navigateBack();
        }
    });
}

void OTAPage::onButtonEnter(void* context) {
    if (currentState == OTA_STATE_SHOW_INFO) {
        navBar->showRightBlink(1, 80, 80, [this]() {
            // 开始更新
            if (!taskRunning) {
                taskRunning = true;
                xTaskCreate(performUpdateTask, "PerformUpdate", 8192, this, 1, &taskHandle);
            }
        });
    } else if (currentState == OTA_STATE_DONE) {
        navBar->showRightBlink(1, 80, 80, [this]() {
            ESP.restart();
        });
    }
}

void OTAPage::onButton2(void* context) {
    if (currentState == OTA_STATE_SHOW_INFO) {
        descArea->scrollUp();
    }
}

void OTAPage::onButton8(void* context) {
    if (currentState == OTA_STATE_SHOW_INFO) {
        descArea->scrollDown();
    }
}
