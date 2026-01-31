/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef APModePage_h
#define APModePage_h

#include "../GUI/UIPage.h"
#include "../GUI/Widget/UILabel.h"
#include "../GUI/Widget/UISelectValue.h"
#include "../GUI/Widget/UIInput.h"
#include "../GUI/Widget/UINavBar.h"
#include "../GUIRender.h"

enum APModePageState {
    STATE_VIEW,      // 查看状态
    STATE_EDIT_PASSWORD  // 编辑密码状态
};

class APModePage : public UIPage {
public:
    APModePage();
    
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
    
    // 重写update方法，用于定时刷新AP状态
    void update() override;
    
private:
    void initLayout(); // 初始化页面布局
    void updateDisplay(); // 更新显示
    void toggleAP(); // 切换AP状态
    void saveSettings(); // 保存设置
    void updateHomePageAPStatus(); // 更新主页AP状态
    
    UILabel* titleLabel;
    UILabel* apNameLabel;      // "AP名称:"
    UILabel* apNameValue;      // 显示AP名称
    UILabel* apIPLabel;     // "AP IP:"
    UILabel* apIPValue;     // 显示AP IP
    UILabel* passwordLabel;    // "密码:"
    UISelectValue* passwordValue;    // 显示密码（可点击编辑，使用UISelectValue以支持选中状态）
    UILabel* statusLabel;      // "状态:"
    UISelectValue* statusValue; // 显示开启/关闭状态
    UINavBar* navBar;
    
    UIInput* passwordInput;    // 密码输入框
    UINavBar* inputNavBar;     // 输入状态下的导航栏
    
    APModePageState currentState; // 当前页面状态
    bool apEnabled;              // AP是否启用
    String apName;               // AP名称
    String apPassword;           // AP密码
    bool originalAPEnabled;      // 原始AP状态（页面进入时的状态）
    String originalAPPassword;   // 原始密码（页面进入时的密码）
    String editingPassword;      // 编辑密码时的临时备份
    int selectedItem;            // 当前选中的项 (0=状态, 1=密码)
    unsigned long lastUpdateTime; // 上次更新时间（毫秒）
    bool lastAPStatus;           // 上次AP状态
};

#endif

