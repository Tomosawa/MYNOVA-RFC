/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// BrightnessPage.h
#ifndef BrightnessPage_h
#define BrightnessPage_h

#include "../GUI/UIPage.h"
#include "../GUI/Widget/UILabel.h"
#include "../GUI/Widget/UIProgressBar.h"
#include "../GUI/Widget/UINavBar.h"
#include "../GUIRender.h"

class BrightnessPage : public UIPage {
public:
    BrightnessPage();
    
    // 重写按钮事件处理
    void onButtonBack(void* context = nullptr) override;
    void onButtonEnter(void* context = nullptr) override;
    void onButton2(void* context = nullptr) override;
    void onButton4(void* context = nullptr) override;
    void onButton6(void* context = nullptr) override;
    void onButton8(void* context = nullptr) override;
    
private:
    void initLayout(); // 初始化页面布局
    void updateBrightness(); // 更新亮度显示和实际亮度
    void restoreBrightness(); // 恢复原始亮度
    void saveBrightness(); // 保存亮度设置
    
    UILabel* titleLabel;
    UIProgressBar* progressBar;
    UINavBar* navBar;
    
    int currentBrightness; // 当前亮度值
    int originalBrightness; // 原始亮度值（用于取消时恢复）
};

#endif

