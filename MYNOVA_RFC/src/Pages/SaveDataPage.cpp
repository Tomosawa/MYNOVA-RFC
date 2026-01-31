/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "SaveDataPage.h"
#include "../GUI/Widget/UIMessageBox.h"
#include "../GUI/Animation/MessageBoxAnimation.h"
#include "../GUI/Animation/AnimationEngine.h"
#include "../DataStore.h"
#include "../GUI/UIEngine.h"

extern UIEngine uiEngine;
extern DataStore dataStore;
SaveDataPage::SaveDataPage(RCData rcData) : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),
receivedData(rcData),
currentState(STATE_SELECT) {
    initLayout();
}

SaveDataPage::~SaveDataPage() {
}

void SaveDataPage::initLayout() {
    // 初始化100个存储位置选项
    dataListMenu = new UIMenu(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 5);
    dataListMenu->titleText = "选择存储位置";
    dataListMenu->bShowBtnTips = true;
    dataListMenu->bShowBorder = false;
    dataListMenu->bShowTitle = false;
    dataListMenu->bVisible = true;
    // 通过导航栏设置按钮文字
    dataListMenu->getNavBar()->setLeftButtonText("返回");
    dataListMenu->getNavBar()->setRightButtonText("选择");
    for (int i = 1; i <= 100; i++) {
        String itemName = String(i) + ". ";
        RadioData data = dataStore.ReadData(i);
        if (data.name.length() > 0) {
            itemName += data.name;
        } else {
            itemName += "----------";
        }
        dataListMenu->addMenuItem(itemName);
    }
    addWidget(dataListMenu);

    // 初始化提示框（使用默认居中位置，避开底部NavBar）
    messageBox = new UIMessageBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    messageBox->setMessage("要覆盖数据吗？");
    messageBox->setTitle("确认覆盖");
    messageBox->setButtons("取消", "覆盖");
    addWidget(messageBox);
    
    // 初始化保存成功提示框
    successMessageBox = new UIMessageBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    successMessageBox->setMessage("保存成功！");
    successMessageBox->setTitle("提示");
    successMessageBox->setButtons("返回", "");
    successMessageBox->bVisible = false;
    addWidget(successMessageBox);
    
    // 初始化输入框
    nameInput = new UIInput(0, 0, 128, 50);
    nameInput->setTextAlign(INPUT_LEFT);
    nameInput->setTitle("请输入名称");
    nameInput->bVisible = false;
    addWidget(nameInput);
    
    // 初始化输入状态下的导航栏
    inputNavBar = new UINavBar(0, SCREEN_HEIGHT - 15, SCREEN_WIDTH, 15);
    inputNavBar->setLeftButtonText("返回");
    inputNavBar->setMiddleButtonText("删除");
    inputNavBar->setRightButtonText("确认");
    inputNavBar->bVisible = false;
    addWidget(inputNavBar);
}

void SaveDataPage::render(U8G2* u8g2) {
    UIPage::render(u8g2);
}

// void SaveDataPage::update() {
//     // // 更新动画引擎
//     // animationEngine.update();
    
//     // // 更新消息框（处理自动关闭）
//     // messageBox->update();
    
//     if (currentState == STATE_INPUTNAME) {
//         nameInput->update();
//     }
    
//     if (currentState == STATE_SAVE_SUCCESS) {
//         successMessageBox->update();
//     }
// }

void SaveDataPage::onButtonBack(void* context) {
    if(currentState == STATE_SELECT)
    {
        // 显示左键闪烁动画，动画完成后执行跳转
        dataListMenu->getNavBar()->showLeftBlink(2, 80, 80, [this]() {
            uiEngine.navigateBack();
        });
    }
    else if(currentState == STATE_MESSASGE)
    {
        messageBox->showLeftButtonHighLight([this]() {
            messageBox->hide(true);
            dataListMenu->bVisible = true;
            currentState = STATE_SELECT;
        });
    }
    else if(currentState == STATE_INPUTNAME)
    {
        // 显示左键闪烁动画，动画完成后返回选择状态
        inputNavBar->showLeftBlink(1, 80, 80, [this]() {
            nameInput->bVisible = false;
            inputNavBar->bVisible = false;
            dataListMenu->bVisible = true;
            dataListMenu->getNavBar()->bVisible = true;
            currentState = STATE_SELECT;
        });
    }
    else if(currentState == STATE_SAVE_SUCCESS)
    {
        // 显示左键闪烁动画，动画完成后返回到菜单页面
        successMessageBox->showLeftButtonHighLight([this]() {
            successMessageBox->hide(true);
            // 返回到菜单页面（返回到堆栈中的第2个页面：MenuPage）
            // 页面堆栈：HomePage -> MenuPage -> ReceivePage -> SaveDataPage
            // 返回到 MenuPage 需要返回 2 步
            uiEngine.navigateBackSteps(2);
        });
    }
}

void SaveDataPage::onButtonEnter(void* context) {
    // 当前是选择状态，确认选择当前位置后，判断是否覆盖
    if(currentState == STATE_SELECT)
    {
        int selectedIndex = dataListMenu->menuSel + 1; // 菜单索引从0开始，但存储位置从1开始
        // 显示右键闪烁动画，动画完成后切换状态
        dataListMenu->getNavBar()->showRightBlink(2, 80, 80, [this, selectedIndex]() {
            // 检查该位置是否已有数据
            RadioData data = dataStore.ReadData(selectedIndex);
            if (data.name.length() > 0) {
                // 弹出提示框询问是否覆盖（带缩放动画）
                messageBox->show(MSGBOX_ANIME_ZOOM_CENTER, 300);
                dataListMenu->bVisible = false;
                currentState = STATE_MESSASGE;
            } else {
                // 显示输入框进行名称输入
                dataListMenu->bVisible = false;
                nameInput->bVisible = true;
                inputNavBar->bVisible = true;
                nameInput->setText(generateDefaultName(selectedIndex));
                currentState = STATE_INPUTNAME;
            }
        });
    }else if(currentState == STATE_MESSASGE)
    {
        // 确认覆盖
        messageBox->showRightButtonHighLight([this]() {
            messageBox->hide();
            int selectedIndex = dataListMenu->menuSel + 1;
            RadioData data = dataStore.ReadData(selectedIndex);
            inputNavBar->bVisible = true;
            nameInput->bVisible = true;
            nameInput->setText(data.name);
            currentState = STATE_INPUTNAME;
        });
    }else if(currentState == STATE_INPUTNAME)
    {
        // 显示右键闪烁动画，动画完成后保存数据并返回
        inputNavBar->showRightBlink(1, 80, 80, [this]() {
            saveDataAndReturn();
        });
    }
}

void SaveDataPage::onButtonMenu(void* context) {
    if (currentState == STATE_INPUTNAME) {
        // 删除字符不需要等待动画，立即执行
        nameInput->deleteChar();
    }
}
void SaveDataPage::onButton1(void* context) {
    if (currentState == STATE_INPUTNAME) {
        nameInput->onButtonPress(1);
    }
}

void SaveDataPage::onButton2(void* context) {
    if(currentState == STATE_SELECT)
    {
        dataListMenu->moveUp();
    }
    else if (currentState == STATE_INPUTNAME) {
        nameInput->onButtonPress(2);
    }
}

void SaveDataPage::onButton3(void* context) {
    if (currentState == STATE_INPUTNAME) {
        nameInput->onButtonPress(3);
    }
}

void SaveDataPage::onButton4(void* context) {
    if (currentState == STATE_INPUTNAME) {
        nameInput->onButtonPress(4);
    }
}

void SaveDataPage::onButton5(void* context) {
    if (currentState == STATE_INPUTNAME) {
        nameInput->onButtonPress(5);
    }
}

void SaveDataPage::onButton6(void* context) {
    if (currentState == STATE_INPUTNAME) {
        nameInput->onButtonPress(6);
    }
}

void SaveDataPage::onButton7(void* context) {
    if (currentState == STATE_INPUTNAME) {
        nameInput->onButtonPress(7);
    }
}

void SaveDataPage::onButton8(void* context) {
    if(currentState == STATE_SELECT)
    {
        dataListMenu->moveDown();
    }
    else if (currentState == STATE_INPUTNAME) {
        nameInput->onButtonPress(8);
    }
}

void SaveDataPage::onButton9(void* context) {
    if (currentState == STATE_INPUTNAME) {
        nameInput->onButtonPress(9);
    }
}

String SaveDataPage::generateDefaultName(int index) {
    return "NO NAME" + String(index);
}

void SaveDataPage::saveDataAndReturn() {
    int selectedIndex = dataListMenu->menuSel + 1;
    RadioData radioData;
    radioData.rcData = receivedData;
    radioData.name = nameInput->getText();
    dataStore.SaveData(selectedIndex, radioData);
    
    // 隐藏输入框和导航栏
    nameInput->bVisible = false;
    inputNavBar->bVisible = false;
    
    // 显示保存成功提示
    successMessageBox->show(MSGBOX_ANIME_ZOOM_CENTER, 300);
    currentState = STATE_SAVE_SUCCESS;
}