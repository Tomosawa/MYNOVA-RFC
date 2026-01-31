/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// SettingPage.cpp
#include "SettingPage.h"
#include "../GUI/UIEngine.h"
#include "VersionPage.h"
#include "BrightnessPage.h"
#include "SoundPage.h"
#include "PowerSavePage.h"
#include "RepeatTransmitPage.h"
#include "APModePage.h"
#include "WiFiModePage.h"
#include "FactoryResetPage.h"
#include "OTAPage.h"

extern UIEngine uiEngine;

// 设置项定义
const char* SettingPage::settingItem[] = {"1.屏幕亮度","2.声音提示","3.节能设置","4.重复发送","5.AP模式","6.WIFI模式","7.恢复出厂","8.系统版本", "9.固件更新"};
const int SettingPage::settingItemCount = 9;

SettingPage::SettingPage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),
settingMenu(new UIMenu(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 5)) {
    initLayout();
}


void SettingPage::initLayout() {
    // 初始化设置菜单
    settingMenu->bShowBtnTips = true;
    settingMenu->getNavBar()->setLeftButtonText("返回");
    settingMenu->getNavBar()->setRightButtonText("确定");
    settingMenu->bShowBorder = false;
    settingMenu->bShowTitle = false;

    for(int i = 0; i < settingItemCount; i++) {
        settingMenu->addMenuItem(settingItem[i]);
    }

    addWidget(settingMenu);
}

void SettingPage::onButtonBack(void* context) {
    // 显示左键闪烁动画，动画完成后执行跳转
    settingMenu->getNavBar()->showLeftBlink(1, 80, 80, [this]() {
        uiEngine.navigateBack();
    });
}

void SettingPage::onButtonEnter(void* context) {
    int selectedItem = settingMenu->menuSel;
    // 显示右键闪烁动画，动画完成后处理设置
    settingMenu->getNavBar()->showRightBlink(1, 80, 80, [this, selectedItem]() {
        // 处理设置选项选择
        switch (selectedItem) {
            case 0:
                // 屏幕亮度设置
                uiEngine.navigateTo(new BrightnessPage());
                break;
            case 1:
                // 声音提示设置
                uiEngine.navigateTo(new SoundPage());
                break;
            case 2:
                // 节能设置
                uiEngine.navigateTo(new PowerSavePage());
                break;
            case 3:
                // 重复发送设置
                uiEngine.navigateTo(new RepeatTransmitPage());
                break;
            case 4:
                // AP模式设置
                uiEngine.navigateTo(new APModePage());
                break;
            case 5:
                // WIFI模式设置
                uiEngine.navigateTo(new WiFiModePage());
                break;
            case 6:
                // 恢复出厂设置
                uiEngine.navigateTo(new FactoryResetPage());
                break;
            case 7:
                // 系统版本查看
                uiEngine.navigateTo(new VersionPage());
                break;
            case 8:
                // 固件更新
                uiEngine.navigateTo(new OTAPage());
                break;
            default:
                break;
        }
    });
}

void SettingPage::onButton2(void* context) {
    settingMenu->moveUp();
}

void SettingPage::onButton8(void* context) {
    settingMenu->moveDown();
}