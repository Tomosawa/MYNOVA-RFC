/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// MenuPage.cpp
#include "MenuPage.h"
#include "../GUI/UIEngine.h"
#include "../Pages/HomePage.h"
#include "../Pages/SettingPage.h"
#include "../Pages/ReceivePage.h"
#include "../Pages/SendDataPage.h"
#include "../Pages/ManageDataPage.h"
#include "../Pages/GameListPage.h"

extern UIEngine uiEngine;

// 菜单项定义
const char* menuItem[] = {"1.主页","2.发送模式","3.接收模式","4.管理数据","5.系统设置","6.休闲游戏"};
const int menuItemCount = 6;

MenuPage::MenuPage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),
menu(new UIMenu(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 5)) {
    initLayout();
}

void MenuPage::initLayout() {
    // 初始化菜单
    menu->bShowBtnTips = true;
    menu->bShowBorder = false;
    menu->bShowTitle = false;
    // 通过导航栏设置按钮文字
    menu->getNavBar()->setLeftButtonText("返回");
    menu->getNavBar()->setRightButtonText("选择");
    
    for(int i = 0; i < menuItemCount; i++) {
        menu->addMenuItem(menuItem[i]);
    }
    addWidget(menu);
}

void MenuPage::onButtonBack(void* context) {
    // 显示左键闪烁动画，动画完成后执行跳转
    menu->getNavBar()->showLeftBlink(1, 80, 80, [this]() {
        uiEngine.navigateBack();
    });
}

void MenuPage::onButtonMenu(void* context) {
    menu->moveDown();
}

void MenuPage::onButtonEnter(void* context) {
    // 显示右键闪烁动画，动画完成后执行跳转
    int selectedItem = menu->menuSel; // 先保存选中项
    menu->getNavBar()->showRightBlink(1, 80, 80, [this, selectedItem]() {
        switch (selectedItem) {
            case 0:
                uiEngine.navigateBack();
                break;
            case 1:
                uiEngine.navigateTo(new SendDataPage());
                break;
            case 2:
                uiEngine.navigateTo(new ReceivePage());
                break;
            case 3:
                uiEngine.navigateTo(new ManageDataPage());
                break;
            case 4:
                uiEngine.navigateTo(new SettingPage());
                break;
            case 5:
                uiEngine.navigateTo(new GameListPage());
                break;
            default:
                break;
        }
    });
}

void MenuPage::onButton2(void* context) {
    menu->moveUp();
}

void MenuPage::onButton5(void* context) {
    onButtonEnter(context);
}

void MenuPage::onButton8(void* context) {
    menu->moveDown();
}