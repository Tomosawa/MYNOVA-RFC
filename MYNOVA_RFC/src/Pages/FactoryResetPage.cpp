/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// FactoryResetPage.cpp
#include "FactoryResetPage.h"
#include "../GUI/UIEngine.h"
#include "../DataStore.h"
#include <ESP.h>

extern UIEngine uiEngine;
extern DataStore dataStore;

FactoryResetPage::FactoryResetPage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) {
    currentState = STATE_CONFIRM;
    initLayout();
}

void FactoryResetPage::initLayout() {
    // 标题
    titleLabel = new UILabel();
    titleLabel->x = 0;
    titleLabel->y = 0;
    titleLabel->width = SCREEN_WIDTH;
    titleLabel->height = 12;
    titleLabel->label = "恢复出厂设置";
    titleLabel->textAlign = CENTER;
    titleLabel->verticalAlign = MIDDLE;
    addWidget(titleLabel);
    
    // 警告文字第1行
    warningLabel1 = new UILabel();
    warningLabel1->x = 4;
    warningLabel1->y = 15;
    warningLabel1->width = SCREEN_WIDTH - 8;
    warningLabel1->height = 12;
    warningLabel1->label = "是否恢复出厂设置?";
    warningLabel1->textAlign = CENTER;
    warningLabel1->verticalAlign = MIDDLE;
    warningLabel1->textFont = u8g2_font_wqy12_t_gb2312;
    addWidget(warningLabel1);
    
    // 警告文字第2行
    warningLabel2 = new UILabel();
    warningLabel2->x = 4;
    warningLabel2->y = 28;
    warningLabel2->width = SCREEN_WIDTH - 8;
    warningLabel2->height = 12;
    warningLabel2->label = "所有数据将被清空!";
    warningLabel2->textAlign = CENTER;
    warningLabel2->verticalAlign = MIDDLE;
    warningLabel2->textFont = u8g2_font_wqy12_t_gb2312;
    addWidget(warningLabel2);
    
    // 警告文字第3行
    warningLabel3 = new UILabel();
    warningLabel3->x = 4;
    warningLabel3->y = 41;
    warningLabel3->width = SCREEN_WIDTH - 8;
    warningLabel3->height = 12;
    warningLabel3->label = "按确定继续";
    warningLabel3->textAlign = CENTER;
    warningLabel3->verticalAlign = MIDDLE;
    warningLabel3->textFont = u8g2_font_wqy12_t_gb2312;
    addWidget(warningLabel3);
    
    // 导航栏
    navBar = new UINavBar(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH, 12);
    navBar->setLeftButtonText("返回");
    navBar->setRightButtonText("确定");
    addWidget(navBar);
    
    // 输入提示标签
    inputPromptLabel = new UILabel();
    inputPromptLabel->x = 0;
    inputPromptLabel->y = 0;
    inputPromptLabel->width = SCREEN_WIDTH;
    inputPromptLabel->height = 12;
    inputPromptLabel->label = "输入YES确认";
    inputPromptLabel->textAlign = CENTER;
    inputPromptLabel->verticalAlign = MIDDLE;
    inputPromptLabel->textFont = u8g2_font_wqy12_t_gb2312;
    inputPromptLabel->bVisible = false;
    addWidget(inputPromptLabel);
    
    // 初始化输入框（隐藏）
    yesInput = new UIInput(0, 12, SCREEN_WIDTH, SCREEN_HEIGHT - 24);
    yesInput->setTextAlign(INPUT_CENTER);
    yesInput->setTitle("");
    yesInput->bVisible = false;
    addWidget(yesInput);
    
    // 初始化输入状态下的导航栏（隐藏）
    inputNavBar = new UINavBar(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH, 12);
    inputNavBar->setLeftButtonText("返回");
    inputNavBar->setMiddleButtonText("删除");
    inputNavBar->setRightButtonText("确认");
    inputNavBar->bVisible = false;
    addWidget(inputNavBar);
    
    updateDisplay();
}

void FactoryResetPage::updateDisplay() {
    if (currentState == STATE_CONFIRM) {
        titleLabel->bVisible = true;
        warningLabel1->bVisible = true;
        warningLabel2->bVisible = true;
        warningLabel3->bVisible = true;
        navBar->bVisible = true;
        inputPromptLabel->bVisible = false;
        yesInput->bVisible = false;
        inputNavBar->bVisible = false;
    }
    else if (currentState == STATE_INPUT_YES) {
        titleLabel->bVisible = false;
        warningLabel1->bVisible = false;
        warningLabel2->bVisible = false;
        warningLabel3->bVisible = false;
        navBar->bVisible = false;
        inputPromptLabel->bVisible = true;
        yesInput->bVisible = true;
        inputNavBar->bVisible = true;
    }
}

void FactoryResetPage::performFactoryReset() {
    // 清空所有数据
    dataStore.ClearAllData();
    
    // 延迟一下确保数据写入完成
    delay(500);
    
    // 重启系统
    ESP.restart();
}

void FactoryResetPage::onButtonBack(void* context) {
    if (currentState == STATE_CONFIRM) {
        // 返回键：返回上一页
        navBar->showLeftBlink(1, 80, 80, [this]() {
            uiEngine.navigateBack();
        });
    } else if (currentState == STATE_INPUT_YES) {
        // 返回键：取消输入，返回确认状态
        inputNavBar->showLeftBlink(1, 80, 80, [this]() {
            yesInput->clearText();
            currentState = STATE_CONFIRM;
            updateDisplay();
        });
    }
}

void FactoryResetPage::onButtonEnter(void* context) {
    if (currentState == STATE_CONFIRM) {
        // 确定键：进入输入YES状态
        navBar->showRightBlink(1, 80, 80, [this]() {
            yesInput->clearText();
            currentState = STATE_INPUT_YES;
            updateDisplay();
        });
    } else if (currentState == STATE_INPUT_YES) {
        // 确认键：检查输入是否为YES
        inputNavBar->showRightBlink(1, 80, 80, [this]() {
            String inputText = yesInput->getText();
            if (inputText == "YES") {
                // 输入正确，执行恢复出厂设置
                performFactoryReset();
            } else {
                // 输入错误，清空输入框
                yesInput->clearText();
            }
        });
    }
}

void FactoryResetPage::onButtonMenu(void* context) {
    if (currentState == STATE_INPUT_YES) {
        // 删除字符
        yesInput->deleteChar();
    }
}

void FactoryResetPage::onButton1(void* context) {
    if (currentState == STATE_INPUT_YES) {
        yesInput->onButtonPress(1);
    }
}

void FactoryResetPage::onButton2(void* context) {
    if (currentState == STATE_INPUT_YES) {
        yesInput->onButtonPress(2);
    }
}

void FactoryResetPage::onButton3(void* context) {
    if (currentState == STATE_INPUT_YES) {
        yesInput->onButtonPress(3);
    }
}

void FactoryResetPage::onButton4(void* context) {
    if (currentState == STATE_INPUT_YES) {
        yesInput->onButtonPress(4);
    }
}

void FactoryResetPage::onButton5(void* context) {
    if (currentState == STATE_INPUT_YES) {
        yesInput->onButtonPress(5);
    }
}

void FactoryResetPage::onButton6(void* context) {
    if (currentState == STATE_INPUT_YES) {
        yesInput->onButtonPress(6);
    }
}

void FactoryResetPage::onButton7(void* context) {
    if (currentState == STATE_INPUT_YES) {
        yesInput->onButtonPress(7);
    }
}

void FactoryResetPage::onButton8(void* context) {
    if (currentState == STATE_INPUT_YES) {
        yesInput->onButtonPress(8);
    }
}

void FactoryResetPage::onButton9(void* context) {
    if (currentState == STATE_INPUT_YES) {
        yesInput->onButtonPress(9);
    }
}
