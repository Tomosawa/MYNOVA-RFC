/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// APModePage.cpp
#include "APModePage.h"
#include "../GUI/UIEngine.h"
#include "../SystemSetting.h"
#include "../GUIRender.h"
#include "../Pages/HomePage.h"
#include "../WiFiManager.h"

extern UIEngine uiEngine;
extern SystemSetting systemSetting;
extern HomePage uiPageHome;
extern WiFiManager wifiManager;

APModePage::APModePage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) {
    // 从SystemSetting读取当前设置
    SystemConfig config = systemSetting.getConfig();
    apEnabled = config.APEnabled;
    apName = config.APName.length() > 0 ? config.APName : "MYNOVA_RFC";
    apPassword = config.APPassword.length() > 0 ? config.APPassword : "MYNOVA123";
    
    // 保存原始值
    originalAPEnabled = apEnabled;
    originalAPPassword = apPassword;
    
    // 默认选中第一项（状态）
    selectedItem = 0;
    currentState = STATE_VIEW;
    lastUpdateTime = 0;
    lastAPStatus = apEnabled;
    
    initLayout();
}

void APModePage::initLayout() {
    // 标题
    // titleLabel = new UILabel();
    // titleLabel->x = 0;
    // titleLabel->y = 0;
    // titleLabel->width = SCREEN_WIDTH;
    // titleLabel->height = 12;
    // titleLabel->label = "AP模式设置";
    // titleLabel->textAlign = CENTER;
    // titleLabel->verticalAlign = MIDDLE;
    // addWidget(titleLabel);
    
    // 第一行：AP名称
    apNameLabel = new UILabel();
    apNameLabel->x = 0;
    apNameLabel->y = 0;
    apNameLabel->width = 40;
    apNameLabel->height = 12;
    apNameLabel->label = "名称:";
    apNameLabel->textAlign = LEFT;
    apNameLabel->verticalAlign = MIDDLE;
    apNameLabel->textFont = u8g2_font_wqy12_t_gb2312;
    addWidget(apNameLabel);
    
    apNameValue = new UILabel();
    apNameValue->x = 40;
    apNameValue->y = 0;
    apNameValue->width = 88;
    apNameValue->height = 12;
    apNameValue->textAlign = LEFT;
    apNameValue->verticalAlign = MIDDLE;
    apNameValue->textFont = u8g2_font_wqy12_t_gb2312;
    addWidget(apNameValue);

    // AP IP
    apIPLabel = new UILabel();
    apIPLabel->x = 0;
    apIPLabel->y = 12;
    apIPLabel->width = 40;
    apIPLabel->height = 12;
    apIPLabel->label = "IP:";
    apIPLabel->textAlign = LEFT;
    apIPLabel->verticalAlign = MIDDLE;
    apIPLabel->textFont = u8g2_font_wqy12_t_gb2312;
    addWidget(apIPLabel);
    
    apIPValue = new UILabel();
    apIPValue->x = 40;
    apIPValue->y = 12;
    apIPValue->width = 88;
    apIPValue->height = 12;
    apIPValue->textAlign = LEFT;
    apIPValue->label = wifiManager.getAPIP();
    apIPValue->verticalAlign = MIDDLE;
    apIPValue->textFont = u8g2_font_wqy12_t_gb2312;
    apIPValue->bVisible = false;
    addWidget(apIPValue);
    
    // 第二行：密码
    passwordLabel = new UILabel();
    passwordLabel->x = 0;
    passwordLabel->y = 24;
    passwordLabel->width = 40;
    passwordLabel->height = 12;
    passwordLabel->label = "密码:";
    passwordLabel->textAlign = LEFT;
    passwordLabel->verticalAlign = MIDDLE;
    passwordLabel->textFont = u8g2_font_wqy12_t_gb2312;
    addWidget(passwordLabel);
    
    passwordValue = new UISelectValue();
    passwordValue->x = 40;
    passwordValue->y = 24;
    passwordValue->width = 88;
    passwordValue->height = 12;
    passwordValue->bShowBorder = true;
    passwordValue->textFont = u8g2_font_wqy12_t_gb2312;
    addWidget(passwordValue);
    
    // 第三行：状态
    statusLabel = new UILabel();
    statusLabel->x = 0;
    statusLabel->y = 36;
    statusLabel->width = 40;
    statusLabel->height = 12;
    statusLabel->label = "状态:";
    statusLabel->textAlign = LEFT;
    statusLabel->verticalAlign = MIDDLE;
    statusLabel->textFont = u8g2_font_wqy12_t_gb2312;
    addWidget(statusLabel);
    
    statusValue = new UISelectValue();
    statusValue->x = 40;
    statusValue->y = 36;
    statusValue->width = 88;
    statusValue->height = 12;
    statusValue->bShowBorder = true;
    statusValue->textFont = u8g2_font_wqy12_t_gb2312;
    addWidget(statusValue);
    
    // 导航栏
    navBar = new UINavBar(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH, 12);
    navBar->setLeftButtonText("返回");
    navBar->setRightButtonText("切换");
    addWidget(navBar);
    
    // 初始化输入框（隐藏）
    passwordInput = new UIInput(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - 14);
    passwordInput->setTextAlign(INPUT_LEFT);
    passwordInput->setTitle("请输入密码");
    passwordInput->bVisible = false;
    addWidget(passwordInput);
    
    // 初始化输入状态下的导航栏（隐藏）
    inputNavBar = new UINavBar(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH, 12);
    inputNavBar->setLeftButtonText("返回");
    inputNavBar->setMiddleButtonText("删除");
    inputNavBar->setRightButtonText("确认");
    inputNavBar->bVisible = false;
    addWidget(inputNavBar);
    
    // 更新显示
    updateDisplay();
}

void APModePage::updateDisplay() {
    // 更新AP名称显示
    apNameValue->label = apName;

    passwordValue->value = apPassword;
    passwordValue->bSelected = (selectedItem == 1);
    
    // 更新状态显示
    statusValue->value = apEnabled ? "开启" : "关闭";
    statusValue->bSelected = (selectedItem == 0);

    // 更新状态界面显示
    if (currentState == STATE_VIEW) {
        //titleLabel->bVisible = true;
        apNameLabel->bVisible = true;
        apNameValue->bVisible = true;
        apIPLabel->bVisible = true;
        apIPValue->bVisible = true;
        passwordLabel->bVisible = true;
        passwordValue->bVisible = true;
        statusLabel->bVisible = true;
        statusValue->bVisible = true;
        navBar->bVisible = true;
        inputNavBar->bVisible = false;
        passwordInput->bVisible = false;
    }
    else if (currentState == STATE_EDIT_PASSWORD) {
        //titleLabel->bVisible = false;
        apNameLabel->bVisible = false;
        apNameValue->bVisible = false;
        apIPLabel->bVisible = false;
        apIPValue->bVisible = false;
        passwordLabel->bVisible = false;
        passwordValue->bVisible = false;
        statusLabel->bVisible = false;
        statusValue->bVisible = false;
        navBar->bVisible = false;
        inputNavBar->bVisible = true;
        passwordInput->bVisible = true;
    }

    if(apEnabled)
    {
        apIPValue->label = wifiManager.getAPIP();
        apIPValue->bVisible = true;
    }
    else
    {
        apIPValue->bVisible = false;
    }
}

void APModePage::toggleAP() {
    apEnabled = !apEnabled;
    // 立即应用设置（不保存）
    systemSetting.setAPConfig(apName, apPassword, false);
    systemSetting.setAPEnabled(apEnabled, true);
    updateHomePageAPStatus();
    updateDisplay();
}

void APModePage::saveSettings() {
    // 保存设置
    updateHomePageAPStatus();
}

void APModePage::updateHomePageAPStatus() {
    // 更新主页的AP状态显示
    uiPageHome.updateStatus();
}

void APModePage::onButtonBack(void* context) {
    if (currentState == STATE_VIEW) {
        // 返回键
        navBar->showLeftBlink(1, 80, 80, [this]() {
            saveSettings();
            updateHomePageAPStatus();
            uiEngine.navigateBack();
        });
    } else if (currentState == STATE_EDIT_PASSWORD) {
        // 返回键：取消编辑密码，恢复到编辑前的密码
        inputNavBar->showLeftBlink(1, 80, 80, [this]() {
            passwordInput->bVisible = false;
            inputNavBar->bVisible = false;
            navBar->bVisible = true;
            currentState = STATE_VIEW;
            // 恢复到编辑前的密码
            apPassword = editingPassword;
            updateDisplay();
        });
    }
}

void APModePage::onButtonEnter(void* context) {
    if (currentState == STATE_VIEW) {
        if (selectedItem == 0) {
            toggleAP();
            updateDisplay();
        } else if (selectedItem == 1) {
            // 进入编辑密码状态
            // 保存当前密码作为编辑前的临时备份
            editingPassword = apPassword;
            passwordInput->setText(apPassword);
            currentState = STATE_EDIT_PASSWORD;

            updateDisplay();
        } 
    } else if (currentState == STATE_EDIT_PASSWORD) {
        // 确认键：保存密码（不保存到flash，只在确定时保存）
        inputNavBar->showRightBlink(1, 80, 80, [this]() {
            apPassword = passwordInput->getText();
            if (apPassword.length() == 0) {
                apPassword = "MYNOVA123"; // 默认密码
            }
            currentState = STATE_VIEW;
            updateDisplay();
        });
    }
}

void APModePage::onButtonMenu(void* context) {
    if (currentState == STATE_EDIT_PASSWORD) {
        // 删除字符不需要等待动画，立即执行
        passwordInput->deleteChar();
    }
}

void APModePage::onButton1(void* context) {
    if (currentState == STATE_EDIT_PASSWORD) {
        passwordInput->onButtonPress(1);
    }
}

void APModePage::onButton2(void* context) {
    if (currentState == STATE_VIEW) {
        // 上移选择
        selectedItem--;
        if (selectedItem < 0) {
            selectedItem = 1; // 循环到最后一项
        }
        if (selectedItem == 0) {
            navBar->setRightButtonText("切换");
        } else {
            navBar->setRightButtonText("修改");
        }
        updateDisplay();
    } else if (currentState == STATE_EDIT_PASSWORD) {
        passwordInput->onButtonPress(2);
    }
}

void APModePage::onButton3(void* context) {
    if (currentState == STATE_EDIT_PASSWORD) {
        passwordInput->onButtonPress(3);
    }
}

void APModePage::onButton4(void* context) {
    if (currentState == STATE_VIEW && selectedItem == 0) {
        // 切换AP状态为关闭
        if (apEnabled) {
            toggleAP();
        }
    } else if (currentState == STATE_EDIT_PASSWORD) {
        passwordInput->onButtonPress(4);
    }
}

void APModePage::onButton5(void* context) {
    if (currentState == STATE_EDIT_PASSWORD) {
        passwordInput->onButtonPress(5);
    }
}

void APModePage::onButton6(void* context) {
    if (currentState == STATE_VIEW && selectedItem == 0) {
        // 切换AP状态为开启
        if (!apEnabled) {
            toggleAP();
        }
    } else if (currentState == STATE_EDIT_PASSWORD) {
        passwordInput->onButtonPress(6);
    }
}

void APModePage::onButton7(void* context) {
    if (currentState == STATE_EDIT_PASSWORD) {
        passwordInput->onButtonPress(7);
    }
}

void APModePage::onButton8(void* context) {
    if (currentState == STATE_VIEW) {
        // 下移选择
        selectedItem++;
        if (selectedItem > 1) {
            selectedItem = 0; // 循环到第一项
        }
        if (selectedItem == 0) {
            navBar->setRightButtonText("切换");
        } else {
            navBar->setRightButtonText("修改");
        }
        updateDisplay();
    } else if (currentState == STATE_EDIT_PASSWORD) {
        passwordInput->onButtonPress(8);
    }
}

void APModePage::onButton9(void* context) {
    if (currentState == STATE_EDIT_PASSWORD) {
        passwordInput->onButtonPress(9);
    }
}

void APModePage::update() {
    // 每500毫秒检查一次AP状态
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime >= 500) {
        lastUpdateTime = currentTime;
        
        // 获取当前AP状态
        SystemConfig config = systemSetting.getConfig();
        bool currentAPStatus = config.APEnabled;
        
        // 如果AP状态发生变化，更新显示
        if (currentAPStatus != lastAPStatus) {
            lastAPStatus = currentAPStatus;
            apEnabled = currentAPStatus;
            updateDisplay();
        }
    }
}

