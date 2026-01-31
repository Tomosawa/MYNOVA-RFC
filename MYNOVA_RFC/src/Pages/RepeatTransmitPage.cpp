/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// RepeatTransmitPage.cpp
#include "RepeatTransmitPage.h"
#include "../GUI/UIEngine.h"
#include "../SystemSetting.h"
#include "../RadioHelper.h"

extern UIEngine uiEngine;
extern SystemSetting systemSetting;
extern RadioHelper radioHelper;

RepeatTransmitPage::RepeatTransmitPage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) {
    // 从SystemSetting读取当前重复发送次数设置
    currentRepeatTransmit = systemSetting.getRepeatTransmit();
    // 如果为0，使用默认值15
    if (currentRepeatTransmit == 0) {
        currentRepeatTransmit = 15;
    }
    originalRepeatTransmit = currentRepeatTransmit; // 保存原始值
    
    initLayout();
}

void RepeatTransmitPage::initLayout() {
    // 计算布局：屏幕中间显示"重复发送 XX 次"
    // 需要计算各部分的位置，使整体居中
    
    // 前缀标签（显示"重复发送"）
    prefixLabel = new UILabel(); 
    prefixLabel->label = "重复发送";
    prefixLabel->textFont = u8g2_font_wqy12_t_gb2312;
    prefixLabel->textAlign = RIGHT;
    prefixLabel->verticalAlign = MIDDLE;
    prefixLabel->height = 12;
    prefixLabel->y = 20;
    addWidget(prefixLabel);
    
    // 可编辑数字组件（显示在中间）
    numberEdit = new UIEditableNumber();
    numberEdit->setMode(EDITABLE_DECIMAL);
    numberEdit->setMaxDigits(2);  // 最多2位数（1-50）
    numberEdit->setRange(1, 50);  // 范围1-50
    numberEdit->setValue(currentRepeatTransmit);
    numberEdit->setSelected(true);  // 默认选中
    numberEdit->textFont = u8g2_font_wqy12_t_gb2312;
    numberEdit->width = 15;
    numberEdit->height = 12;
    numberEdit->y = 20;
    numberEdit->bVisible = true;
    addWidget(numberEdit);
    
    // 后缀标签（显示"次"）
    suffixLabel = new UILabel();
    suffixLabel->label = "次";
    suffixLabel->textFont = u8g2_font_wqy12_t_gb2312;
    suffixLabel->textAlign = LEFT;
    suffixLabel->verticalAlign = MIDDLE;
    suffixLabel->height = 12;
    suffixLabel->y = 20;
    addWidget(suffixLabel);
    
    // 计算各部分位置，使整体居中
    // 使用固定值估算各部分宽度："重复发送"约48像素，"次"约12像素，数字30像素
    int prefixWidth = 48;
    int numberWidth = 15;
    int suffixWidth = 12;
    int totalWidth = prefixWidth + numberWidth + suffixWidth;
    
    // 计算起始X坐标，使整体居中
    int startX = (SCREEN_WIDTH - totalWidth) / 2;
    
    prefixLabel->x = startX;
    prefixLabel->width = prefixWidth;
    
    numberEdit->x = startX + prefixWidth + 5;
    
    suffixLabel->x = startX + prefixWidth + numberWidth + 10;
    suffixLabel->width = suffixWidth;
    
    // 导航栏
    navBar = new UINavBar(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH, 12);
    navBar->setLeftButtonText("取消");
    navBar->setRightButtonText("确定");
    addWidget(navBar);
    
    // 更新显示
    updateRepeatTransmit();
}

void RepeatTransmitPage::updateRepeatTransmit() {
    // 从数字编辑组件获取当前值
    currentRepeatTransmit = numberEdit->getValue();
}

void RepeatTransmitPage::restoreRepeatTransmit() {
    // 恢复原始值
    currentRepeatTransmit = originalRepeatTransmit;
    numberEdit->setValue(currentRepeatTransmit);
    radioHelper.SetRepeatTransmit(currentRepeatTransmit);
}

void RepeatTransmitPage::saveRepeatTransmit() {
    // 从数字编辑组件获取最终值
    currentRepeatTransmit = numberEdit->getValue();
    // 保存到SystemSetting（会同时保存到存储）
    systemSetting.setRepeatTransmit(currentRepeatTransmit, true); // true表示保存
    // 确保RadioHelper也使用新值
    radioHelper.SetRepeatTransmit(currentRepeatTransmit);
}

void RepeatTransmitPage::onButtonBack(void* context) {
    // 返回键：恢复原始值，不保存
    navBar->showLeftBlink(1, 80, 80, [this]() {
        restoreRepeatTransmit();
        uiEngine.navigateBack();
    });
}

void RepeatTransmitPage::onButtonEnter(void* context) {
    // 确定键：保存设置并返回
    navBar->showRightBlink(1, 80, 80, [this]() {
        saveRepeatTransmit();
        uiEngine.navigateBack();
    });
}

void RepeatTransmitPage::onButton2(void* context) {
    // 增加当前位的值
    numberEdit->incrementDigit();
    updateRepeatTransmit();
}

void RepeatTransmitPage::onButton4(void* context) {
    // 光标左移（向高位移动）
    numberEdit->moveCursorLeft();
}

void RepeatTransmitPage::onButton6(void* context) {
    // 光标右移（向低位移动）
    numberEdit->moveCursorRight();
}

void RepeatTransmitPage::onButton8(void* context) {
    // 减少当前位的值
    numberEdit->decrementDigit();
    updateRepeatTransmit();
}

