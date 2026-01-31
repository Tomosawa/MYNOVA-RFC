/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef __SENDDATAPAGE_H__
#define __SENDDATAPAGE_H__

#include <Arduino.h>
#include "../GUI/UIPage.h"
#include "../GUI/Widget/UIMenu.h"
#include "../GUI/Widget/UIMessageBox.h"
#include "../GUI/Widget/UILabel.h"
#include "../GUI/Widget/UINavBar.h"
#include "../DataStore.h"
#include "../RadioHelper.h"

enum SendDataPageState {
    STATE_SELECT,      // 选择数据状态
    STATE_DETAIL,      // 显示详情状态
    STATE_SENDING,     // 发送中状态
    STATE_SET_QUICK    // 设置快捷方式状态
};

class SendDataPage : public UIPage {
public:
    SendDataPage();
    ~SendDataPage();
    
    void initLayout();
    void render(U8G2* u8g2) override;
    //void update();
    
    // 按钮事件处理
    void onButtonBack(void* context) override;
    void onButtonEnter(void* context) override;
    void onButtonMenu(void* context) override;
    void onButton1(void* context) override;
    void onButton2(void* context) override;
    void onButton3(void* context) override;
    void onButton4(void* context) override;
    void onButton5(void* context) override;
    void onButton6(void* context) override;
    void onButton7(void* context) override;
    void onButton8(void* context) override;
    void onButton9(void* context) override;

private:
    UIMenu* dataListMenu;
    UIMessageBox* messageBox;
    UIMessageBox* quickSetBox;
    UILabel* freqLabel;
    UILabel* dataLabel;
    UILabel* bitLengthLabel;
    UILabel* pulseLengthLabel;
    UILabel* protocolLabel;
    UILabel* nameLabel;
    UINavBar* detailNavBar;
    UINavBar* quickSetNavBar;
    
    SendDataPageState currentState;
    int selectedDataIndex;
    RadioData currentData;
    bool bAnimating;
    
    void showDataDetail(int index);
    void setAsQuickKey(int keyIndex);
    String formatHexData(unsigned long data);
};

#endif