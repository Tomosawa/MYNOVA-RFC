/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// SettingPage.h
#ifndef SettingPage_h
#define SettingPage_h

#include "../GUI/UIPage.h"
#include "../GUI/Widget/UIMenu.h"
#include "../GUIRender.h"

class SettingPage : public UIPage {
public:
    SettingPage();
    
    // 重写按钮事件处理
    void onButtonBack(void* context = nullptr) override;
    void onButtonEnter(void* context = nullptr) override;
    void onButton2(void* context = nullptr) override;
    void onButton8(void* context = nullptr) override;
    
private:
    void initLayout(); // 初始化页面布局
    
    UIMenu *settingMenu;
    static const char* settingItem[];
    static const int settingItemCount;
};

#endif