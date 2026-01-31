/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// VersionPage.cpp
#include "VersionPage.h"
#include "../GUI/UIEngine.h"
#include "../../Version.h"
#include <Esp.h>

extern UIEngine uiEngine;

VersionPage::VersionPage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),
navBar(new UINavBar(0, SCREEN_HEIGHT - 10, SCREEN_WIDTH, 10)) {
    initLayout();
}

void VersionPage::initLayout() {
    // 标题
    titleLabel = new UILabel();
    titleLabel->x = 0;
    titleLabel->y = 0;
    titleLabel->width = SCREEN_WIDTH;
    titleLabel->height = 12;
    titleLabel->label = "系统版本";
    titleLabel->textAlign = CENTER;
    titleLabel->verticalAlign = MIDDLE;
    addWidget(titleLabel);
    
    // 固件版本
    versionLabel = new UILabel();
    versionLabel->x = 3;
    versionLabel->y = 12;
    versionLabel->width = SCREEN_WIDTH;
    versionLabel->height = 12;
    versionLabel->label = "固件版本: V" + String(FIRMWARE_VERSION);
    versionLabel->textAlign = LEFT;
    versionLabel->verticalAlign = MIDDLE;
    addWidget(versionLabel);
    
    // 编译日期
    dateLabel = new UILabel();
    dateLabel->x = 3;
    dateLabel->y = 24;
    dateLabel->width = SCREEN_WIDTH;
    dateLabel->height = 12;
    dateLabel->label = "编译日期: " + convertDateFormat(FIRMWARE_BUILD_DATE);
    dateLabel->textAlign = LEFT;
    dateLabel->verticalAlign = MIDDLE;
    addWidget(dateLabel);
    
    // 设备序列号
    serialLabel = new UILabel();
    serialLabel->x = 3;
    serialLabel->y = 36;
    serialLabel->width = SCREEN_WIDTH;
    serialLabel->height = 12;
    String serialNumber = String((uint32_t)(ESP.getEfuseMac() >> 32), HEX) + 
                         String((uint32_t)ESP.getEfuseMac(), HEX);
    serialNumber.toUpperCase();
    serialLabel->label = "序列号: " + serialNumber;
    serialLabel->textAlign = LEFT;
    serialLabel->verticalAlign = MIDDLE;
    addWidget(serialLabel);
    
    // 导航栏
    navBar->setLeftButtonText("返回");
    navBar->setMiddleButtonText("");
    navBar->setRightButtonText("");
    addWidget(navBar);
}


String VersionPage::convertDateFormat(const char* buildDate) {
    // __DATE__ 格式: "Nov  7 2025" 或 "Nov 17 2025"
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    
    char monthStr[4];
    int day, year;
    
    // 使用sscanf解析，自动处理多个空格
    sscanf(buildDate, "%s %d %d", monthStr, &day, &year);
    
    // 查找月份对应的数字
    int month = 1;
    for (int i = 0; i < 12; i++) {
        if (strcmp(monthStr, months[i]) == 0) {
            month = i + 1;
            break;
        }
    }
    
    // 格式化为 YYYY.MM.DD
    char formattedDate[12];
    sprintf(formattedDate, "%d.%02d.%02d", year, month, day);
    
    return String(formattedDate);
}

void VersionPage::onButtonBack(void* context) {
    // 显示左键闪烁动画，动画完成后执行跳转
    navBar->showLeftBlink(1, 80, 80, [this]() {
        uiEngine.navigateBack();
    });
}

