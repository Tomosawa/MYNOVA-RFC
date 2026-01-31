/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// GameListPage.cpp
#include "GameListPage.h"
#include "../GUI/UIEngine.h"
#include "../Pages/FlappyBirdPage.h"
#include "../Pages/SnakePage.h"
#include "../Pages/TetrisPage.h"
#include "../Pages/ArkanoidPage.h"
#include "../Pages/TankBattlePage.h"
#include "../Pages/RacingPage.h"
#include "../Pages/ShooterPage.h"

extern UIEngine uiEngine;

// 游戏菜单项定义
const char* gameMenuItem[] = {"1.Flappy Bird","2.贪吃蛇","3.俄罗斯方块","4.打砖块","5.坦克大战","6.极速赛车","7.雷霆战机"};
const int gameMenuItemCount = 7;

GameListPage::GameListPage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),
menu(new UIMenu(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 5)) {
    initLayout();
}

void GameListPage::initLayout() {
    // 初始化菜单
    menu->bShowBtnTips = true;
    menu->bShowBorder = false;
    menu->bShowTitle = false;
    menu->titleText = "游戏清单";
    // 通过导航栏设置按钮文字
    menu->getNavBar()->setLeftButtonText("返回");
    menu->getNavBar()->setRightButtonText("选择");
    
    for(int i = 0; i < gameMenuItemCount; i++) {
        menu->addMenuItem(gameMenuItem[i]);
    }
    addWidget(menu);
}

void GameListPage::onButtonBack(void* context) {
    // 显示左键闪烁动画，动画完成后执行跳转
    menu->getNavBar()->showLeftBlink(1, 80, 80, [this]() {
        uiEngine.navigateBack();
    });
}

void GameListPage::onButtonMenu(void* context) {
    menu->moveDown();
}

void GameListPage::onButtonEnter(void* context) {
    // 显示右键闪烁动画，动画完成后执行跳转
    int selectedItem = menu->menuSel; // 先保存选中项
    menu->getNavBar()->showRightBlink(1, 80, 80, [this, selectedItem]() {
        switch (selectedItem) {
            case 0:
                uiEngine.navigateTo(new FlappyBirdPage());
                break;
            case 1:
                uiEngine.navigateTo(new SnakePage());
                break;
            case 2:
                uiEngine.navigateTo(new TetrisPage());
                break;
            case 3:
                uiEngine.navigateTo(new ArkanoidPage());
                break;
            case 4:
                uiEngine.navigateTo(new TankBattlePage());
                break;
            case 5:
                uiEngine.navigateTo(new RacingPage());
                break;
            case 6:
                uiEngine.navigateTo(new ShooterPage());
                break;
            default:
                break;
        }
    });
}

void GameListPage::onButton2(void* context) {
    menu->moveUp();
}

void GameListPage::onButton5(void* context) {
    onButtonEnter(context);
}

void GameListPage::onButton8(void* context) {
    menu->moveDown();
}