/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// FactoryResetPage.h
#ifndef FactoryResetPage_h
#define FactoryResetPage_h

#include "../GUI/UIPage.h"
#include "../GUI/Widget/UILabel.h"
#include "../GUI/Widget/UIInput.h"
#include "../GUI/Widget/UINavBar.h"
#include "../GUIRender.h"

enum FactoryResetPageState {
    STATE_CONFIRM,      // 确认状态
    STATE_INPUT_YES     // 输入YES确认状态
};

class FactoryResetPage : public UIPage {
public:
    FactoryResetPage();
    
    // 重写按钮事件处理
    void onButtonBack(void* context = nullptr) override;
    void onButtonEnter(void* context = nullptr) override;
    void onButtonMenu(void* context = nullptr) override;
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
    void updateDisplay(); // 更新显示
    void performFactoryReset(); // 执行恢复出厂设置
    
    UILabel* titleLabel;
    UILabel* warningLabel1;    // 警告文字第1行
    UILabel* warningLabel2;    // 警告文字第2行
    UILabel* warningLabel3;    // 警告文字第3行
    UINavBar* navBar;
    
    UIInput* yesInput;         // YES输入框
    UILabel* inputPromptLabel; // 输入提示
    UINavBar* inputNavBar;     // 输入状态下的导航栏
    
    FactoryResetPageState currentState; // 当前页面状态
};

#endif
