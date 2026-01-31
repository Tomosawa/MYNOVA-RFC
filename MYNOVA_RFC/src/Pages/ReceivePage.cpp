/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// ReceivePage.cpp
#include "ReceivePage.h"
#include "../GUI/UIEngine.h"
#include "../GUIRender.h"
#include "SaveDataPage.h"

extern UIEngine uiEngine;
extern RadioHelper radioHelper;
extern GUIRender guiRender;

ReceivePage::ReceivePage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),
    statusLabel(nullptr),
    freqLabel(nullptr),
    dataLabel(nullptr),
    bitLengthLabel(nullptr),
    protocolLabel(nullptr),
    currentState(STATE_RECEIVING),
    lastCheckTime(0) {
    initLayout();
}

ReceivePage::~ReceivePage() {
    // 停止接收
    stopReceiving();
}

void ReceivePage::initLayout() {
    // 创建状态标签
    statusLabel = new UILabel();
    statusLabel->x = 0;
    statusLabel->y = 0;
    statusLabel->width = SCREEN_WIDTH;
    statusLabel->height = 12;
    statusLabel->label = "正在监听...";
    statusLabel->textAlign = CENTER;
    statusLabel->verticalAlign = MIDDLE;
    statusLabel->bVisible = true;
    addWidget(statusLabel);
    
    // 创建频率标签
    freqLabel = new UILabel();
    freqLabel->x = 0;
    freqLabel->y = 12;
    freqLabel->width = SCREEN_WIDTH;
    freqLabel->height = 12;
    freqLabel->label = "频率: ---MHz";
    freqLabel->textAlign = LEFT;
    freqLabel->verticalAlign = MIDDLE;
    freqLabel->bVisible = true;
    addWidget(freqLabel);

    // 创建协议标签
    protocolLabel = new UILabel();
    protocolLabel->x = 76;
    protocolLabel->y = 12;
    protocolLabel->width = SCREEN_WIDTH - 4;
    protocolLabel->height = 12;
    protocolLabel->label = "协议: --";
    protocolLabel->textFont = u8g2_font_wqy12_t_gb2312;
    protocolLabel->textAlign = LEFT;
    protocolLabel->verticalAlign = MIDDLE;
    protocolLabel->bVisible = true;
    addWidget(protocolLabel);
    
    
    // 创建位长度标签
    bitLengthLabel = new UILabel();
    bitLengthLabel->x = 0;
    bitLengthLabel->y = 24;
    bitLengthLabel->width = SCREEN_WIDTH - 4;
    bitLengthLabel->height = 12;
    bitLengthLabel->label = "位长: --";
    bitLengthLabel->textFont = u8g2_font_wqy12_t_gb2312;
    bitLengthLabel->textAlign = LEFT;
    bitLengthLabel->verticalAlign = MIDDLE;
    bitLengthLabel->bVisible = true;
    addWidget(bitLengthLabel);
    
    // 创建脉宽标签
    pulseLengthLabel = new UILabel();
    pulseLengthLabel->x = 76;
    pulseLengthLabel->y = 24;
    pulseLengthLabel->width = SCREEN_WIDTH - 4;
    pulseLengthLabel->height = 12;
    pulseLengthLabel->label = "脉宽: --";
    pulseLengthLabel->textFont = u8g2_font_wqy12_t_gb2312;
    pulseLengthLabel->textAlign = LEFT;
    pulseLengthLabel->verticalAlign = MIDDLE;
    pulseLengthLabel->bVisible = true;
    addWidget(pulseLengthLabel);

    // 创建数据标签
    dataLabel = new UILabel();
    dataLabel->x = 0;
    dataLabel->y = 36;
    dataLabel->width = SCREEN_WIDTH - 4;
    dataLabel->height = 12;
    dataLabel->label = "数据: 等待信号...";
    dataLabel->textFont = u8g2_font_wqy12_t_gb2312;
    dataLabel->textAlign = LEFT;
    dataLabel->verticalAlign = MIDDLE;
    dataLabel->bVisible = true;
    addWidget(dataLabel);

    // 创建导航栏
    navBar = new UINavBar(0, 52, SCREEN_WIDTH, 12);
    navBar->setMargin(0, 0);
    navBar->bVisible = true;
    navBar->setLeftButtonText("返回");
    addWidget(navBar);

    startReceiving();
}

void ReceivePage::startReceiving() {
    currentState = STATE_RECEIVING;
    
    // 更新显示状态
    statusLabel->label = "正在监听...";
    freqLabel->label = "频率: ---MHz";
    dataLabel->label = "数据: 等待信号...";
    bitLengthLabel->label = "位长: --";
    protocolLabel->label = "协议: --";
    pulseLengthLabel->label = "脉宽: --";
    navBar->setRightButtonText("");
    
    // 启动接收（同时支持315和433）
    radioHelper.EnableRecive();
    
    lastCheckTime = millis();
}

void ReceivePage::stopReceiving() {
    // 只有在接收模式下才禁用接收
    if (currentState == STATE_RECEIVING) {
        radioHelper.DisableRecive();
    }
}
String ReceivePage::formatHexData(unsigned long data) {
    String hexStr = String(data, HEX);
    hexStr.toUpperCase(); // 转换为大写
    
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
void ReceivePage::updateReceiveStatus() {
    if (currentState != STATE_RECEIVING) {
        return;
    }
    
    // 检查是否接收到数据
    unsigned long currentTime = millis();
    if (currentTime - lastCheckTime > 100) { // 每100ms检查一次
        lastCheckTime = currentTime;
        
        // 检查是否接收到数据
        if (radioHelper.rcData.data != 0) {
            // 接收到数据
            currentState = STATE_RECEIVED;
            statusLabel->label = "接收成功!";
            
            // 更新频率显示（根据实际接收到的频率）
            if (radioHelper.rcData.freqType == FREQ_315) {
                freqLabel->label = "频率: 315MHz";
            } else {
                freqLabel->label = "频率: 433MHz";
            }
            
            // 更新数据显示（如果数据太长，进行截断）
            String dataStr = formatHexData(radioHelper.rcData.data);
            if (dataStr.length() > 10) {
                dataStr = dataStr.substring(0, 10) + "...";
            }
            dataLabel->label = "数据: " + dataStr;
            bitLengthLabel->label = "位长: " + String(radioHelper.rcData.bitLength) + "bit";
            protocolLabel->label = "协议: " + String(radioHelper.rcData.protocal);
            pulseLengthLabel->label = "脉宽: " + String(radioHelper.rcData.pulseLength);
            navBar->setRightButtonText("保存");

            // 停止接收
            stopReceiving();
        }
    }
}

void ReceivePage::showPage() {
    
}

void ReceivePage::render(U8G2* u8g2) {
    // 先更新接收状态
    updateReceiveStatus();
    // 然后调用父类的 render 方法来渲染所有组件
    UIPage::render(u8g2);
}

void ReceivePage::onButtonBack(void* context) {
    if (currentState == STATE_RECEIVING)
    {
        // 显示左键闪烁动画，动画完成后返回上一页
        navBar->showLeftBlink(1, 80, 80, [this]() {
            stopReceiving();
            // 清空数据
            radioHelper.rcData.data = 0;
            // 返回上一页
            uiEngine.navigateBack();
        });
    }
    else if(currentState == STATE_RECEIVED)
    {
        // 显示左键闪烁动画，动画完成后返回上一页
        navBar->showLeftBlink(1, 80, 80, [this]() {
            // 清空数据
            radioHelper.rcData.data = 0;
            // 返回上一页
            uiEngine.navigateBack();
        });
    }
}

void ReceivePage::onButtonEnter(void* context) {
    if (currentState == STATE_RECEIVED) {
        // 显示右键闪烁动画，动画完成后跳转到保存页面
        navBar->showRightBlink(1, 80, 80, [this]() {
            // 跳转到数据列表页面以保存数据
            SaveDataPage* saveDataPage = new SaveDataPage(radioHelper.rcData);
            uiEngine.navigateTo(saveDataPage);
        });
    }
}