/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// BrightnessPage.cpp
#include "BrightnessPage.h"
#include "../GUI/UIEngine.h"
#include "../SystemSetting.h"

extern UIEngine uiEngine;
extern SystemSetting systemSetting;

BrightnessPage::BrightnessPage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) {
    // 从SystemSetting读取当前亮度设置
    currentBrightness = systemSetting.getBrightness();
    originalBrightness = currentBrightness; // 保存原始值
    
    initLayout();
}

void BrightnessPage::initLayout() {
    // 标题
    titleLabel = new UILabel(); 
    titleLabel->x = 0;
    titleLabel->y = 0;
    titleLabel->width = SCREEN_WIDTH;
    titleLabel->height = 12;
    titleLabel->label = "屏幕亮度";
    titleLabel->textAlign = CENTER;
    titleLabel->verticalAlign = MIDDLE;
    addWidget(titleLabel);
    
    // 进度条
    progressBar = new UIProgressBar();
    progressBar->x = 10;
    progressBar->y = 25;
    progressBar->width = 108;
    progressBar->height = 18;
    progressBar->setValue(currentBrightness);
    progressBar->bShowText = true;
    progressBar->bShowBorder = true;
    addWidget(progressBar);
    
    // 导航栏
    navBar = new UINavBar(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH, 12);
    navBar->setLeftButtonText("取消");
    navBar->setRightButtonText("确定");
    addWidget(navBar);
}

void BrightnessPage::updateBrightness() {
    // 更新进度条显示
    progressBar->setValue(currentBrightness);
    
    // 实时设置屏幕亮度（通过SystemSetting）
    systemSetting.setBrightness(currentBrightness, false); // false表示不保存
}

void BrightnessPage::restoreBrightness() {
    // 恢复原始亮度
    currentBrightness = originalBrightness;
    systemSetting.setBrightness(currentBrightness, false); // false表示不保存
}

void BrightnessPage::saveBrightness() {
    // 保存亮度到SystemSetting（会同时保存到存储）
    systemSetting.setBrightness(currentBrightness, true); // true表示保存
}

void BrightnessPage::onButtonBack(void* context) {
    // 返回键：恢复原始亮度，不保存
    navBar->showLeftBlink(1, 80, 80, [this]() {
        restoreBrightness();
        uiEngine.navigateBack();
    });
}

void BrightnessPage::onButtonEnter(void* context) {
    // 确定键：保存亮度设置并返回
    navBar->showRightBlink(1, 80, 80, [this]() {
        saveBrightness();
        uiEngine.navigateBack();
    });
}

void BrightnessPage::onButton2(void* context) {
    // 增加亮度
    if (currentBrightness < 100) {
        currentBrightness += 5;
        if (currentBrightness > 100) {
            currentBrightness = 100;
        }
        updateBrightness();
    }
}

void BrightnessPage::onButton4(void* context) {
    // 减少亮度
    if (currentBrightness > 0) {
        currentBrightness -= 5;
        if (currentBrightness < 0) {
            currentBrightness = 0;
        }
        updateBrightness();
    }
}

void BrightnessPage::onButton6(void* context) {
    // 增加亮度（与按键2相同）
    onButton2(context);
}

void BrightnessPage::onButton8(void* context) {
    // 减少亮度（与按键4相同）
    onButton4(context);
}

