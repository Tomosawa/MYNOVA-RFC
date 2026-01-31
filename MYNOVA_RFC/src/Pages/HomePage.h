/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// HomePage.h
#ifndef HomePage_h
#define HomePage_h

#include "../GUI/UIPage.h"
#include "../GUI/Widget/UITitleBar.h"
#include "../GUI/Widget/UIQuickButton.h"
#include "../GUIRender.h"
#include "../DataStore.h"
#include "../RadioHelper.h"

class HomePage : public UIPage {
public:
    HomePage();
    
    // 重写按钮事件处理
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
    
    void showPage() override;

    void setBattery(float voltage);
    void updateStatus();
    
private:
    void initLayout(); // 初始化页面布局
    void loadQuickKeyData(); // 从 Flash 加载快捷键数据到缓存
    bool sendCachedData(int buttonIndex); // 发送缓存的数据（0-8）
    
    UITitleBar titleBar;
    UIQuickButton quickButtons[9];
    QuickKey quickKey;
    RadioData cachedRadioData[9];  // 缓存的快捷键数据
    
    // 防抖机制
    unsigned long lastSendTime = 0;  // 上次发送时间
    static const unsigned long SEND_DEBOUNCE_MS = 200;  // 防抖延迟200ms
};

#endif