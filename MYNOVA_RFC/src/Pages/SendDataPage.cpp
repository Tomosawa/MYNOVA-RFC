/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "SendDataPage.h"
#include "../GUI/UIEngine.h"
#include "../GUI/Animation/MessageBoxAnimation.h"
#include "../GUI/Animation/AnimationEngine.h"
#include "../GUIRender.h"
#include "../DataStore.h"

extern UIEngine uiEngine;
extern RadioHelper radioHelper;
extern DataStore dataStore;

SendDataPage::SendDataPage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),
    currentState(STATE_SELECT),
    bAnimating(false),
    selectedDataIndex(0) {
    initLayout();
}

SendDataPage::~SendDataPage() {
}

void SendDataPage::initLayout() {
    // 初始化数据选择菜单
    dataListMenu = new UIMenu(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 5);
    dataListMenu->titleText = "选择发送数据";
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

    // 初始化详情页面的标签
    nameLabel = new UILabel();
    nameLabel->x = 0;
    nameLabel->y = 0;
    nameLabel->width = SCREEN_WIDTH;
    nameLabel->height = 12;
    nameLabel->label = "名称: --";
    nameLabel->textAlign = CENTER;
    nameLabel->verticalAlign = MIDDLE;
    nameLabel->bVisible = false;
    addWidget(nameLabel);
    
    freqLabel = new UILabel();
    freqLabel->x = 0;
    freqLabel->y = 12;
    freqLabel->width = SCREEN_WIDTH;
    freqLabel->height = 12;
    freqLabel->label = "频率: --";
    freqLabel->textAlign = LEFT;
    freqLabel->verticalAlign = MIDDLE;
    freqLabel->bVisible = false;
    addWidget(freqLabel);

    protocolLabel = new UILabel();
    protocolLabel->x = 76;
    protocolLabel->y = 12;
    protocolLabel->width = SCREEN_WIDTH - 4;
    protocolLabel->height = 12;
    protocolLabel->label = "协议: --";
    protocolLabel->textFont = u8g2_font_wqy12_t_gb2312;
    protocolLabel->textAlign = LEFT;
    protocolLabel->verticalAlign = MIDDLE;
    protocolLabel->bVisible = false;
    addWidget(protocolLabel);
    
    bitLengthLabel = new UILabel();
    bitLengthLabel->x = 0;
    bitLengthLabel->y = 24;
    bitLengthLabel->width = SCREEN_WIDTH - 4;
    bitLengthLabel->height = 12;
    bitLengthLabel->label = "位长: --";
    bitLengthLabel->textFont = u8g2_font_wqy12_t_gb2312;
    bitLengthLabel->textAlign = LEFT;
    bitLengthLabel->verticalAlign = MIDDLE;
    bitLengthLabel->bVisible = false;
    addWidget(bitLengthLabel);
    
    pulseLengthLabel = new UILabel();
    pulseLengthLabel->x = 76;
    pulseLengthLabel->y = 24;
    pulseLengthLabel->width = SCREEN_WIDTH - 4;
    pulseLengthLabel->height = 12;
    pulseLengthLabel->label = "脉宽: --";
    pulseLengthLabel->textFont = u8g2_font_wqy12_t_gb2312;
    pulseLengthLabel->textAlign = LEFT;
    pulseLengthLabel->verticalAlign = MIDDLE;
    pulseLengthLabel->bVisible = false;
    addWidget(pulseLengthLabel);

    dataLabel = new UILabel();
    dataLabel->x = 0;
    dataLabel->y = 36;
    dataLabel->width = SCREEN_WIDTH - 4;
    dataLabel->height = 12;
    dataLabel->label = "数据: -- -- --";
    dataLabel->textFont = u8g2_font_wqy12_t_gb2312;
    dataLabel->textAlign = LEFT;
    dataLabel->verticalAlign = MIDDLE;
    dataLabel->bVisible = false;
    addWidget(dataLabel);

    // 初始化详情页面的导航栏
    detailNavBar = new UINavBar(0, 52, SCREEN_WIDTH, 12);
    detailNavBar->setMargin(0, 0);
    detailNavBar->bVisible = false;
    detailNavBar->setLeftButtonText("返回");
    detailNavBar->setMiddleButtonText("设快捷");
    detailNavBar->setRightButtonText("发送");
    addWidget(detailNavBar);
    
    // 初始化提示框（使用默认居中位置，避开底部NavBar）
    messageBox = new UIMessageBox(-1, -1, 100, 50);
    messageBox->setDefaultPosition(SCREEN_WIDTH, SCREEN_HEIGHT, 15);
    messageBox->setMessage("该位置无数据！");
    messageBox->setTitle("提示");
    messageBox->bShowButtons = false;  // 不显示按钮，使用NavBar
    addWidget(messageBox);
    
    // 初始化设置快捷方式提示框
    quickSetBox = new UIMessageBox(-1, -1, 100, 50);
    quickSetBox->setDefaultPosition(SCREEN_WIDTH, SCREEN_HEIGHT, 15);
    quickSetBox->setMessage("请按1-9键设置");
    quickSetBox->setTitle("设置快捷键");
    quickSetBox->bShowButtons = false;
    addWidget(quickSetBox);
    
    // 初始化设置快捷方式的导航栏
    quickSetNavBar = new UINavBar(0, SCREEN_HEIGHT - 15, SCREEN_WIDTH, 15);
    quickSetNavBar->setLeftButtonText("返回");
    quickSetNavBar->bVisible = false;
    addWidget(quickSetNavBar);
}

void SendDataPage::render(U8G2* u8g2) {
    UIPage::render(u8g2);
}

// void SendDataPage::update() {
 
//     quickSetBox->update();
    
// }

void SendDataPage::showDataDetail(int index) {
    RadioData data = dataStore.ReadData(index);
    if (data.name.length() > 0) {
        // 显示详情
        dataListMenu->bVisible = false;
        currentState = STATE_DETAIL;
        selectedDataIndex = index;
        currentData = data;
        
        // 显示详情信息
        nameLabel->label = "名称: " + data.name;
        nameLabel->bVisible = true;
        
        if (data.rcData.freqType == FREQ_315) {
            freqLabel->label = "频率: 315MHz";
        } else {
            freqLabel->label = "频率: 433MHz";
        }
        freqLabel->bVisible = true;
        
        protocolLabel->label = "协议: " + String(data.rcData.protocal);
        protocolLabel->bVisible = true;
        
        bitLengthLabel->label = "位长: " + String(data.rcData.bitLength) + "bit";
        bitLengthLabel->bVisible = true;
        
        pulseLengthLabel->label = "脉宽: " + String(data.rcData.pulseLength);
        pulseLengthLabel->bVisible = true;
        
        String dataStr = formatHexData(data.rcData.data);
        if (dataStr.length() > 10) {
            dataStr = dataStr.substring(0, 10) + "...";
        }
        dataLabel->label = "数据: " + dataStr;
        dataLabel->bVisible = true;
        
        detailNavBar->bVisible = true;
    } else {
        // 弹出提示框提示无数据（带弹跳动画，3秒后自动关闭）
        messageBox->show(MSGBOX_ANIME_BOUNCE_IN, 400);
        messageBox->setAutoClose(3000);
        dataListMenu->bVisible = false;
        currentState = STATE_SENDING; // 临时状态
    }
}

String SendDataPage::formatHexData(unsigned long data) {
    String hexStr = String(data, HEX);
    hexStr.toUpperCase();
    
    // 如果长度为奇数，在前面补0
    if (hexStr.length() % 2 != 0) {
        hexStr = "0" + hexStr;
    }
    
    // 按字节添加空格
    String formattedStr = "";
    for (int i = 0; i < hexStr.length(); i += 2) {
        if (i > 0) {
            formattedStr += " ";
        }
        formattedStr += hexStr.substring(i, i + 2);
    }
    
    return formattedStr;
}

void SendDataPage::setAsQuickKey(int keyIndex) {
    // 这里应该保存快捷键设置
    QuickKey quickKey = dataStore.LoadQuickKey();
    
    switch(keyIndex) {
        case 1: quickKey.key1 = selectedDataIndex; break;
        case 2: quickKey.key2 = selectedDataIndex; break;
        case 3: quickKey.key3 = selectedDataIndex; break;
        case 4: quickKey.key4 = selectedDataIndex; break;
        case 5: quickKey.key5 = selectedDataIndex; break;
        case 6: quickKey.key6 = selectedDataIndex; break;
        case 7: quickKey.key7 = selectedDataIndex; break;
        case 8: quickKey.key8 = selectedDataIndex; break;
        case 9: quickKey.key9 = selectedDataIndex; break;
    }
    
    dataStore.SaveQuickKey(quickKey);
    
    // 返回详情状态
    quickSetBox->hide();
    quickSetNavBar->bVisible = false;
    detailNavBar->bVisible = true;
    currentState = STATE_DETAIL;
}

void SendDataPage::onButtonBack(void* context) {
    switch (currentState) {
        case STATE_SELECT:
            // 防止重复触发动画
            if (bAnimating) {
                return;
            }
            bAnimating = true;
            
            // 显示左键闪烁动画，动画完成后执行跳转
            dataListMenu->getNavBar()->showLeftBlink(1, 80, 80, [this]() {
                uiEngine.navigateBack();
                bAnimating = false;  // 动画完成，重置标志
            });
            break;
        case STATE_DETAIL:
            // 防止重复触发动画
            if (bAnimating) {
                return;
            }
            bAnimating = true;
            
            // 显示左键闪烁动画，动画完成后返回选择状态
            detailNavBar->showLeftBlink(1, 80, 80, [this]() {
                nameLabel->bVisible = false;
                freqLabel->bVisible = false;
                protocolLabel->bVisible = false;
                bitLengthLabel->bVisible = false;
                pulseLengthLabel->bVisible = false;
                dataLabel->bVisible = false;
                detailNavBar->bVisible = false;
                dataListMenu->bVisible = true;
                currentState = STATE_SELECT;
                bAnimating = false;  // 动画完成，重置标志
            });
            break;
        case STATE_SENDING:
            messageBox->hide();
            messageBox->cancelAutoClose();
            dataListMenu->bVisible = true;
            currentState = STATE_SELECT;
            break;
        case STATE_SET_QUICK:
            // 防止重复触发动画
            if (bAnimating) {
                return;
            }
            bAnimating = true;
            
            // 显示左键闪烁动画，动画完成后返回详情状态
            quickSetNavBar->showLeftBlink(1, 80, 80, [this]() {
                quickSetBox->hide();
                quickSetNavBar->bVisible = false;
                detailNavBar->bVisible = true;
                currentState = STATE_DETAIL;
                bAnimating = false;  // 动画完成，重置标志
            });
            break;
    }
}

void SendDataPage::onButtonEnter(void* context) {
    switch (currentState) {
        case STATE_SELECT: {
            // 防止重复触发动画
            if (bAnimating) {
                return;
            }
            bAnimating = true;
            
            // 显示右键闪烁动画，动画完成后标记待显示详情
            int selectedIndex = dataListMenu->menuSel + 1;
            dataListMenu->getNavBar()->showRightBlink(1, 80, 80, [this, selectedIndex]() {
                showDataDetail(selectedIndex);
                bAnimating = false;  // 动画完成，重置标志
            });
            break;
        }
        case STATE_DETAIL:
            // 防止重复触发动画
            if (bAnimating) {
                return;
            }
            bAnimating = true;
            
            // 显示发送状态
            nameLabel->label = "发送中...";
            // 显示右键闪烁动画，动画完成后标记待发送
            detailNavBar->showRightBlink(1, 80, 80, [this]() {
                // 发送数据
                radioHelper.SendData(currentData.rcData);
                // 显示发送状态
                nameLabel->label = "发送成功";
                bAnimating = false;  // 动画完成，重置标志
            });
            break;
        case STATE_SENDING:
            messageBox->hide();
            messageBox->cancelAutoClose();
            dataListMenu->bVisible = true;
            currentState = STATE_SELECT;
            break;
        case STATE_SET_QUICK:
            // 确认设置快捷键，这里默认设置为快捷键1
            setAsQuickKey(1);
            break;
    }
}

void SendDataPage::onButtonMenu(void* context) {
    switch (currentState) {
        case STATE_SELECT:
            dataListMenu->moveDown();
            break;
        case STATE_DETAIL:
            // 防止重复触发动画
            if (bAnimating) {
                return;
            }
            bAnimating = true;
            
            // 显示中键闪烁动画，动画完成后显示快捷键设置界面
            detailNavBar->showMiddleBlink(1, 80, 80, [this]() {
                // 显示设置快捷方式界面（从底部滑入动画）
                quickSetBox->show(MSGBOX_ANIME_SLIDE_UP, 300);
                detailNavBar->bVisible = false;
                quickSetNavBar->bVisible = true;
                currentState = STATE_SET_QUICK;
                bAnimating = false;  // 动画完成，重置标志
            });
            break;
    }
}

void SendDataPage::onButton1(void* context) {
    if (currentState == STATE_SELECT) {
        dataListMenu->moveUp();
    } else if (currentState == STATE_SET_QUICK) {
        setAsQuickKey(1);
    }
}

void SendDataPage::onButton2(void* context) {
    if (currentState == STATE_SELECT) {
        dataListMenu->moveUp();
    } else if (currentState == STATE_SET_QUICK) {
        setAsQuickKey(2);
    }
}

void SendDataPage::onButton3(void* context) {
    if (currentState == STATE_SET_QUICK) {
        setAsQuickKey(3);
    }
}

void SendDataPage::onButton4(void* context) {
    if (currentState == STATE_SET_QUICK) {
        setAsQuickKey(4);
    }
}

void SendDataPage::onButton5(void* context) {
    onButtonEnter(context);
}

void SendDataPage::onButton6(void* context) {
    if (currentState == STATE_SET_QUICK) {
        setAsQuickKey(6);
    }
}

void SendDataPage::onButton7(void* context) {
    if (currentState == STATE_SET_QUICK) {
        setAsQuickKey(7);
    }
}

void SendDataPage::onButton8(void* context) {
    if (currentState == STATE_SELECT) {
        dataListMenu->moveDown();
    } else if (currentState == STATE_SET_QUICK) {
        setAsQuickKey(8);
    }
}

void SendDataPage::onButton9(void* context) {
    if (currentState == STATE_SET_QUICK) {
        setAsQuickKey(9);
    }
}