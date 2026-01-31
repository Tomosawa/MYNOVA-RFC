/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef __SAVEDATAPAGE_H__
#define __SAVEDATAPAGE_H__

#include <Arduino.h>
#include "../GUI/UIPage.h"
#include "../GUI/Widget/UIMenu.h"
#include "../GUI/Widget/UIMessageBox.h"
#include "../GUI/Widget/UIInput.h"
#include "../GUI/Widget/UINavBar.h"
#include "../GUIRender.h"
#include "../DataStore.h"
#include "../RadioHelper.h"

enum SaveDataPageState {
    STATE_SELECT,      // 选择位置状态
    STATE_MESSASGE,    // 确认覆盖状态
    STATE_INPUTNAME,   // 输入名字状态
    STATE_SAVE_SUCCESS // 保存成功状态
};

class SaveDataPage : public UIPage {
public:
    SaveDataPage(RCData rcData);
    ~SaveDataPage();
    
    void initLayout();
    void render(U8G2* u8g2) override;
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
    UIMessageBox* successMessageBox;  // 保存成功提示框
    UIInput* nameInput;
    UINavBar* inputNavBar;
    RCData receivedData;
    SaveDataPageState currentState;  // 当前页面状态
    String generateDefaultName(int index);
    void saveDataAndReturn();
};

#endif