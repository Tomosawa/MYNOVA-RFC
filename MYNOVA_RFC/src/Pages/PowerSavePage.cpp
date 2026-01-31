/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// PowerSavePage.cpp
#include "PowerSavePage.h"
#include "../GUI/UIEngine.h"
#include "../SystemSetting.h"

extern UIEngine uiEngine;
extern SystemSetting systemSetting;

// 预设休眠时间选项（毫秒）
const long PowerSavePage::sleepTimeOptions[sleepTimeOptionsCount] = {
    0,           // 关闭
    60000,       // 1分钟
    180000,      // 3分钟
    300000,      // 5分钟
    600000,      // 10分钟
    1800000,     // 30分钟
    3600000      // 60分钟
};

const char* PowerSavePage::sleepTimeLabels[sleepTimeOptionsCount] = {
    "关闭",
    "1分钟",
    "3分钟",
    "5分钟",
    "10分钟",
    "30分钟",
    "60分钟"
};

// 预设息屏时间选项（毫秒）
const long PowerSavePage::screenOffTimeOptions[screenOffTimeOptionsCount] = {
    0,      // 关闭
    10000,  // 10秒
    30000,  // 30秒
    60000,  // 1分钟
    180000, // 3分钟
    300000  // 5分钟
};

const char* PowerSavePage::screenOffTimeLabels[screenOffTimeOptionsCount] = {
    "关闭",
    "10秒",
    "30秒",
    "1分钟",
    "3分钟",
    "5分钟"
};

PowerSavePage::PowerSavePage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) {
    // 从SystemSetting读取当前设置
    sleepTime = systemSetting.getAutoSleepTime();
    screenOffTime = systemSetting.getAutoScreenOffTime();
    
    // 保存原始值
    originalSleepTime = sleepTime;
    originalScreenOffTime = screenOffTime;
    
    // 默认选中第一项
    selectedItem = 0;
    
    initLayout();
}

void PowerSavePage::initLayout() {
    // 标题
    titleLabel = new UILabel();
    titleLabel->x = 0;
    titleLabel->y = 0;
    titleLabel->width = SCREEN_WIDTH;
    titleLabel->height = 12;
    titleLabel->label = "节能设置";
    titleLabel->textAlign = CENTER;
    titleLabel->verticalAlign = MIDDLE;
    addWidget(titleLabel);
    
    // 第一行：休眠时间
    // 左侧固定文字
    sleepTimeLabel = new UILabel();
    sleepTimeLabel->x = 4;
    sleepTimeLabel->y = 16;
    sleepTimeLabel->width = 50;
    sleepTimeLabel->height = 15;
    sleepTimeLabel->label = "休眠:";
    sleepTimeLabel->textAlign = LEFT;
    sleepTimeLabel->verticalAlign = MIDDLE;
    addWidget(sleepTimeLabel);
    
    // 右侧可选值
    sleepTimeValue = new UISelectValue();
    sleepTimeValue->x = 56;
    sleepTimeValue->y = 16;
    sleepTimeValue->width = 68;
    sleepTimeValue->height = 15;
    sleepTimeValue->bShowBorder = true;
    addWidget(sleepTimeValue);
    
    // 第二行：息屏时间
    // 左侧固定文字
    screenOffTimeLabel = new UILabel();
    screenOffTimeLabel->x = 4;
    screenOffTimeLabel->y = 33;
    screenOffTimeLabel->width = 50;
    screenOffTimeLabel->height = 15;
    screenOffTimeLabel->label = "息屏:";
    screenOffTimeLabel->textAlign = LEFT;
    screenOffTimeLabel->verticalAlign = MIDDLE;
    addWidget(screenOffTimeLabel);
    
    // 右侧可选值
    screenOffTimeValue = new UISelectValue();
    screenOffTimeValue->x = 56;
    screenOffTimeValue->y = 33;
    screenOffTimeValue->width = 68;
    screenOffTimeValue->height = 15;
    screenOffTimeValue->bShowBorder = true;
    addWidget(screenOffTimeValue);
    
    // 导航栏
    navBar = new UINavBar(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH, 12);
    navBar->setLeftButtonText("取消");
    navBar->setRightButtonText("确定");
    addWidget(navBar);
    
    // 更新显示
    updateDisplay();
}

String PowerSavePage::formatTime(long timeMs) {
    if (timeMs == 0) {
        return "关闭";
    }
    
    if (timeMs < 60000) {
        return String(timeMs / 1000) + "秒";
    } else if (timeMs < 3600000) {
        return String(timeMs / 60000) + "分钟";
    } else {
        return String(timeMs / 3600000) + "小时";
    }
}

int PowerSavePage::findCurrentIndex(int item) {
    if (item == 0) {
        // 查找休眠时间索引
        for (int i = 0; i < sleepTimeOptionsCount; i++) {
            if (sleepTimeOptions[i] == sleepTime) {
                return i;
            }
        }
        // 如果找不到精确匹配，找最接近的
        for (int i = sleepTimeOptionsCount - 1; i >= 0; i--) {
            if (sleepTime >= sleepTimeOptions[i]) {
                return i;
            }
        }
        return 0;
    } else {
        // 查找息屏时间索引
        for (int i = 0; i < screenOffTimeOptionsCount; i++) {
            if (screenOffTimeOptions[i] == screenOffTime) {
                return i;
            }
        }
        // 如果找不到精确匹配，找最接近的
        for (int i = screenOffTimeOptionsCount - 1; i >= 0; i--) {
            if (screenOffTime >= screenOffTimeOptions[i]) {
                return i;
            }
        }
        return 0;
    }
}

void PowerSavePage::updateDisplay() {
    // 更新休眠时间显示
    sleepTimeValue->value = formatTime(sleepTime);
    sleepTimeValue->bSelected = (selectedItem == 0);
    
    // 更新息屏时间显示
    screenOffTimeValue->value = formatTime(screenOffTime);
    screenOffTimeValue->bSelected = (selectedItem == 1);
}

void PowerSavePage::selectPrevItem() {
    selectedItem--;
    if (selectedItem < 0) {
        selectedItem = 1; // 循环到最后一项
    }
    updateDisplay(); // 更新选中状态
}

void PowerSavePage::selectNextItem() {
    selectedItem++;
    if (selectedItem > 1) {
        selectedItem = 0; // 循环到第一项
    }
    updateDisplay(); // 更新选中状态
}

void PowerSavePage::decreaseValue() {
    if (selectedItem == 0) {
        // 调整休眠时间
        int currentIndex = findCurrentIndex(0);
        currentIndex--;
        if (currentIndex < 0) {
            currentIndex = 0;
        }
        sleepTime = sleepTimeOptions[currentIndex];
        
        // 实时应用设置（不保存）
        systemSetting.setAutoSleepTime(sleepTime, false);
    } else {
        // 调整息屏时间
        int currentIndex = findCurrentIndex(1);
        currentIndex--;
        if (currentIndex < 0) {
            currentIndex = 0;
        }
        screenOffTime = screenOffTimeOptions[currentIndex];
        
        // 实时应用设置（不保存）
        systemSetting.setAutoScreenOffTime(screenOffTime, false);
    }
    
    updateDisplay();
}

void PowerSavePage::increaseValue() {
    if (selectedItem == 0) {
        // 调整休眠时间
        int currentIndex = findCurrentIndex(0);
        currentIndex++;
        if (currentIndex >= sleepTimeOptionsCount) {
            currentIndex = sleepTimeOptionsCount - 1;
        }
        sleepTime = sleepTimeOptions[currentIndex];
        
        // 实时应用设置（不保存）
        systemSetting.setAutoSleepTime(sleepTime, false);
    } else {
        // 调整息屏时间
        int currentIndex = findCurrentIndex(1);
        currentIndex++;
        if (currentIndex >= screenOffTimeOptionsCount) {
            currentIndex = screenOffTimeOptionsCount - 1;
        }
        screenOffTime = screenOffTimeOptions[currentIndex];
        
        // 实时应用设置（不保存）
        systemSetting.setAutoScreenOffTime(screenOffTime, false);
    }
    
    updateDisplay();
}

void PowerSavePage::saveSettings() {
    // 保存设置
    systemSetting.setAutoSleepTime(sleepTime, true);
    systemSetting.setAutoScreenOffTime(screenOffTime, true);
}

void PowerSavePage::restoreSettings() {
    // 恢复原始设置
    sleepTime = originalSleepTime;
    screenOffTime = originalScreenOffTime;
    systemSetting.setAutoSleepTime(sleepTime, false);
    systemSetting.setAutoScreenOffTime(screenOffTime, false);
}

void PowerSavePage::onButtonBack(void* context) {
    // 取消键：恢复原始设置，不保存
    navBar->showLeftBlink(1, 80, 80, [this]() {
        restoreSettings();
        uiEngine.navigateBack();
    });
}

void PowerSavePage::onButtonEnter(void* context) {
    // 确定键：保存设置并返回
    navBar->showRightBlink(1, 80, 80, [this]() {
        saveSettings();
        uiEngine.navigateBack();
    });
}

void PowerSavePage::onButton2(void* context) {
    // 上移选择
    selectPrevItem();
}

void PowerSavePage::onButton8(void* context) {
    // 下移选择
    selectNextItem();
}

void PowerSavePage::onButton4(void* context) {
    // 减少数值
    decreaseValue();
}

void PowerSavePage::onButton6(void* context) {
    // 增加数值
    increaseValue();
}

