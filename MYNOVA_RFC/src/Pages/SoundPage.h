/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// SoundPage.h
#ifndef SoundPage_h
#define SoundPage_h

#include "../GUI/UIPage.h"
#include "../GUI/Widget/UILabel.h"
#include "../GUI/Widget/UIIcon.h"
#include "../GUI/Widget/UINavBar.h"
#include "../GUIRender.h"

class SoundPage : public UIPage {
public:
    SoundPage();
    
    // 重写按钮事件处理
    void onButtonBack(void* context = nullptr) override;
    void onButtonEnter(void* context = nullptr) override;
    void onButton1(void* context = nullptr) override;
    void onButton2(void* context = nullptr) override;
    void onButton3(void* context = nullptr) override;
    void onButton4(void* context = nullptr) override;
    void onButton5(void* context = nullptr) override;
    void onButton6(void* context = nullptr) override;
    void onButton7(void* context = nullptr) override;
    void onButton8(void* context = nullptr) override;
    void onButton9(void* context = nullptr) override;
    
private:
    void initLayout(); // 初始化页面布局
    void updateIcon(); // 更新图标显示
    void toggleSound(); // 切换声音开关
    void saveSettings(); // 保存设置
    
    UILabel* titleLabel;
    UIIcon* soundIcon;
    UILabel* statusLabel; // 显示当前状态文字
    UINavBar* navBar;
    
    bool soundEnabled; // 当前声音开关状态
    bool originalState; // 原始状态（用于取消时恢复）
};

#endif

