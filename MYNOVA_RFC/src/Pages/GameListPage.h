/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// GameListPage.h
#ifndef GameListPage_h
#define GameListPage_h

#include "../GUI/UIPage.h"
#include "../GUI/Widget/UIMenu.h"
#include "../GUIRender.h"

class GameListPage : public UIPage {
public:
    GameListPage();
    
    // 重写按钮事件处理
    void onButtonBack(void* context = nullptr) override;
    void onButtonMenu(void* context = nullptr) override;
    void onButtonEnter(void* context = nullptr) override;
    void onButton2(void* context = nullptr) override;
    void onButton5(void* context = nullptr) override;
    void onButton8(void* context = nullptr) override;
    
private:
    void initLayout(); // 初始化页面布局
    
    UIMenu *menu;
};

#endif