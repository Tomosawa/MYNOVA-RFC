/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// PowerSavePage.h
#ifndef PowerSavePage_h
#define PowerSavePage_h

#include "../GUI/UIPage.h"
#include "../GUI/Widget/UILabel.h"
#include "../GUI/Widget/UISelectValue.h"
#include "../GUI/Widget/UINavBar.h"
#include "../GUIRender.h"

class PowerSavePage : public UIPage {
public:
    PowerSavePage();
    
    // 重写按钮事件处理
    void onButtonBack(void* context = nullptr) override;
    void onButtonEnter(void* context = nullptr) override;
    void onButton2(void* context = nullptr) override;  // 上移选择
    void onButton8(void* context = nullptr) override;  // 下移选择
    void onButton4(void* context = nullptr) override;  // 减少数值
    void onButton6(void* context = nullptr) override;  // 增加数值
    
private:
    void initLayout(); // 初始化页面布局
    void updateDisplay(); // 更新显示
    void selectPrevItem(); // 选择上一项
    void selectNextItem(); // 选择下一项
    void decreaseValue(); // 减少当前选中项的值
    void increaseValue(); // 增加当前选中项的值
    void saveSettings(); // 保存设置
    void restoreSettings(); // 恢复原始设置
    String formatTime(long timeMs); // 格式化时间显示
    int findCurrentIndex(int selectedItem); // 查找当前值在预设列表中的索引
    
    UILabel* titleLabel;
    UILabel* sleepTimeLabel;      // 左侧固定文字："休眠时间:"
    UISelectValue* sleepTimeValue; // 右侧可选值："5分钟"
    UILabel* screenOffTimeLabel;   // 左侧固定文字："息屏时间:"
    UISelectValue* screenOffTimeValue; // 右侧可选值："30秒"
    UINavBar* navBar;
    
    int selectedItem; // 当前选中的设置项 (0=休眠时间, 1=息屏时间)
    long sleepTime;   // 当前休眠时间（毫秒）
    long screenOffTime; // 当前息屏时间（毫秒）
    long originalSleepTime; // 原始休眠时间
    long originalScreenOffTime; // 原始息屏时间
    
    // 预设时间选项（毫秒）
    static const int sleepTimeOptionsCount = 7;
    static const long sleepTimeOptions[sleepTimeOptionsCount];
    static const char* sleepTimeLabels[sleepTimeOptionsCount];
    
    static const int screenOffTimeOptionsCount = 6;
    static const long screenOffTimeOptions[screenOffTimeOptionsCount];
    static const char* screenOffTimeLabels[screenOffTimeOptionsCount];
};

#endif

