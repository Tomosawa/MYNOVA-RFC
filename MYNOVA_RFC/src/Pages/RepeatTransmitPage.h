/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// RepeatTransmitPage.h
#ifndef RepeatTransmitPage_h
#define RepeatTransmitPage_h

#include "../GUI/UIPage.h"
#include "../GUI/Widget/UILabel.h"
#include "../GUI/Widget/UIEditableNumber.h"
#include "../GUI/Widget/UINavBar.h"
#include "../GUIRender.h"

class RepeatTransmitPage : public UIPage {
public:
    RepeatTransmitPage();
    
    // 重写按钮事件处理
    void onButtonBack(void* context = nullptr) override;
    void onButtonEnter(void* context = nullptr) override;
    void onButton2(void* context = nullptr) override;
    void onButton4(void* context = nullptr) override;
    void onButton6(void* context = nullptr) override;
    void onButton8(void* context = nullptr) override;
    
private:
    void initLayout(); // 初始化页面布局
    void updateRepeatTransmit(); // 更新重复发送次数显示和实际值
    void restoreRepeatTransmit(); // 恢复原始值
    void saveRepeatTransmit(); // 保存重复发送次数设置
    
    UILabel* prefixLabel;  // 显示"重复发送"
    UIEditableNumber* numberEdit;  // 可编辑数字组件
    UILabel* suffixLabel;  // 显示"次"
    UINavBar* navBar;
    
    int currentRepeatTransmit; // 当前重复发送次数
    int originalRepeatTransmit; // 原始值（用于取消时恢复）
};

#endif

