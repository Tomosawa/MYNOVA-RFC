/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// MenuPage.h
#ifndef MenuPage_h
#define MenuPage_h

#include "../GUI/UIPage.h"
#include "../GUI/Widget/UIMenu.h"
#include "../GUIRender.h"
#include "SendDataPage.h"

class MenuPage : public UIPage {
public:
    MenuPage();
    
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