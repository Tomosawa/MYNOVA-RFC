/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "EditDataPage.h"
#include "../GUI/UIEngine.h"
#include "../DataStore.h"
#include "../GUIRender.h"

extern UIEngine uiEngine;
extern DataStore dataStore;

EditDataPage::EditDataPage(int dataIndex, bool isNew) : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),
    dataIndex(dataIndex),
    isNewData(isNew),
    currentState(EDIT_FIELDS),
    currentFieldIndex(FIELD_FREQ),
    currentFreqIndex(0),
    currentProtocolIndex(0),
    currentBitLengthIndex(0) {
    
    // 加载或初始化数据
    if (!isNew) {
        currentData = dataStore.ReadData(dataIndex);
        
        // 设置当前选项索引
        currentFreqIndex = (currentData.rcData.freqType == FREQ_315) ? 0 : 1;
        currentProtocolIndex = currentData.rcData.protocal - 1;
        if (currentProtocolIndex < 0) currentProtocolIndex = 0;
        if (currentProtocolIndex >= PROTOCOL_COUNT) currentProtocolIndex = PROTOCOL_COUNT - 1;
        
        // 查找位长索引
        currentBitLengthIndex = 5; // 默认24位
        for (int i = 0; i < BITLENGTH_COUNT; i++) {
            if (bitLengthOptions[i] == currentData.rcData.bitLength) {
                currentBitLengthIndex = i;
                break;
            }
        }
    } else {
        // 新建数据，初始化为默认值
        currentData.name = generateDefaultName();
        currentData.rcData.freqType = FREQ_315;
        currentData.rcData.protocal = 1;
        currentData.rcData.bitLength = 24;
        currentData.rcData.pulseLength = 100;
        currentData.rcData.data = 0xFAFAFA; // 示例数据
        currentBitLengthIndex = 5; // 24位
    }
    
    initLayout();
}

EditDataPage::~EditDataPage() {
}

void EditDataPage::initLayout() {
    // ========== 字段编辑状态的UI（参考ReceivePage布局）==========
    
    // 状态标签
    statusLabel = new UILabel();
    statusLabel->x = 0;
    statusLabel->y = 0;
    statusLabel->width = SCREEN_WIDTH;
    statusLabel->height = 12;
    statusLabel->label = isNewData ? "新建数据" : "编辑数据";
    statusLabel->textAlign = CENTER;
    statusLabel->verticalAlign = MIDDLE;
    statusLabel->bVisible = true;
    addWidget(statusLabel);
    
    // 第一行：频率 + 协议
    freqLabel = new UILabel();
    freqLabel->x = 0;
    freqLabel->y = 12;
    freqLabel->width = 40;
    freqLabel->height = 12;
    freqLabel->label = "频率:";
    freqLabel->textFont = u8g2_font_wqy12_t_gb2312;
    freqLabel->textAlign = LEFT;
    freqLabel->verticalAlign = MIDDLE;
    freqLabel->bVisible = true;
    addWidget(freqLabel);
    
    freqSelect = new UISelectValue();
    freqSelect->x = 32;
    freqSelect->y = 12;
    freqSelect->width = 42;
    freqSelect->height = 12;
    freqSelect->value = (currentFreqIndex == 0) ? "315MHz" : "433MHz";
    freqSelect->bSelected = true;
    freqSelect->bShowBorder = false;
    freqSelect->paddingH = 0;
    freqSelect->paddingV = 0;
    freqSelect->textFont = u8g2_font_wqy12_t_gb2312;
    freqSelect->bVisible = true;
    addWidget(freqSelect);
    
    protocolLabel = new UILabel();
    protocolLabel->x = 76;
    protocolLabel->y = 12;
    protocolLabel->width = 30;
    protocolLabel->height = 12;
    protocolLabel->label = "协议:";
    protocolLabel->textFont = u8g2_font_wqy12_t_gb2312;
    protocolLabel->textAlign = LEFT;
    protocolLabel->verticalAlign = MIDDLE;
    protocolLabel->bVisible = true;
    addWidget(protocolLabel);
    
    protocolSelect = new UISelectValue();
    protocolSelect->x = 104;
    protocolSelect->y = 12;
    protocolSelect->width = 22;
    protocolSelect->height = 12;
    protocolSelect->value = String(currentData.rcData.protocal);
    protocolSelect->bSelected = false;
    protocolSelect->bShowBorder = false;
    protocolSelect->paddingH = 0;
    protocolSelect->paddingV = 0;
    protocolSelect->textFont = u8g2_font_wqy12_t_gb2312;
    protocolSelect->bVisible = true;
    addWidget(protocolSelect);
    
    // 第二行：位长 + 脉宽
    bitLengthLabel = new UILabel();
    bitLengthLabel->x = 0;
    bitLengthLabel->y = 24;
    bitLengthLabel->width = 30;
    bitLengthLabel->height = 12;
    bitLengthLabel->label = "位长:";
    bitLengthLabel->textFont = u8g2_font_wqy12_t_gb2312;
    bitLengthLabel->textAlign = LEFT;
    bitLengthLabel->verticalAlign = MIDDLE;
    bitLengthLabel->bVisible = true;
    addWidget(bitLengthLabel);
    
    bitLengthSelect = new UISelectValue();
    bitLengthSelect->x = 32;
    bitLengthSelect->y = 24;
    bitLengthSelect->width = 42;
    bitLengthSelect->height = 12;
    bitLengthSelect->value = String(currentData.rcData.bitLength) + "bit";
    bitLengthSelect->bSelected = false;
    bitLengthSelect->bShowBorder = false;
    bitLengthSelect->paddingH = 0;
    bitLengthSelect->paddingV = 0;
    bitLengthSelect->textFont = u8g2_font_wqy12_t_gb2312;
    bitLengthSelect->bVisible = true;
    addWidget(bitLengthSelect);
    
    pulseLengthLabel = new UILabel();
    pulseLengthLabel->x = 76;
    pulseLengthLabel->y = 24;
    pulseLengthLabel->width = 30;
    pulseLengthLabel->height = 12;
    pulseLengthLabel->label = "脉宽:";
    pulseLengthLabel->textFont = u8g2_font_wqy12_t_gb2312;
    pulseLengthLabel->textAlign = LEFT;
    pulseLengthLabel->verticalAlign = MIDDLE;
    pulseLengthLabel->bVisible = true;
    addWidget(pulseLengthLabel);
    
    pulseLengthEdit = new UIEditableNumber();
    pulseLengthEdit->x = 106;
    pulseLengthEdit->y = 24;
    pulseLengthEdit->width = 22;
    pulseLengthEdit->height = 12;
    pulseLengthEdit->setMode(EDITABLE_DECIMAL);
    pulseLengthEdit->setMaxDigits(3);
    pulseLengthEdit->setRange(0, 999); 
    pulseLengthEdit->setValue(currentData.rcData.pulseLength);
    pulseLengthEdit->setSelected(false);
    pulseLengthEdit->textFont = u8g2_font_wqy12_t_gb2312;
    pulseLengthEdit->bVisible = true;
    addWidget(pulseLengthEdit);
    
    // 第三行：数据
    dataLabel = new UILabel();
    dataLabel->x = 0;
    dataLabel->y = 36;
    dataLabel->width = 30;
    dataLabel->height = 12;
    dataLabel->label = "数据:";
    dataLabel->textFont = u8g2_font_wqy12_t_gb2312;
    dataLabel->textAlign = LEFT;
    dataLabel->verticalAlign = MIDDLE;
    dataLabel->bVisible = true;
    addWidget(dataLabel);
    
    dataEdit = new UIEditableNumber();
    dataEdit->x = 30;
    dataEdit->y = 36;
    dataEdit->width = 98;
    dataEdit->height = 12;
    dataEdit->setMode(EDITABLE_HEX);
    dataEdit->setSelected(false);
    dataEdit->textFont = u8g2_font_wqy12_t_gb2312;
    dataEdit->bVisible = true;
    addWidget(dataEdit);
    
    // 根据当前位长设置数据字段的范围
    updateDataRangeByBitLength();
    
    // 导航栏
    navBar = new UINavBar(0, 52, SCREEN_WIDTH, 12);
    navBar->setMargin(0, 0);
    navBar->setLeftButtonText("返回");
    navBar->setMiddleButtonText("4/6切换");
    navBar->setRightButtonText("下一步");
    navBar->bVisible = true;
    addWidget(navBar);
    
    // ========== 名称编辑状态的UI ==========
    
    // 名称输入框
    nameInput = new UIInput(0, 0, 128, 50);
    nameInput->setTitle("请输入名称");
    nameInput->setText(currentData.name);
    nameInput->setTextAlign(INPUT_LEFT);
    nameInput->bVisible = false;
    addWidget(nameInput);
    
    // 名称编辑的导航栏
    nameNavBar = new UINavBar(0, SCREEN_HEIGHT - 15, SCREEN_WIDTH, 15);
    nameNavBar->setLeftButtonText("返回");
    nameNavBar->setMiddleButtonText("删除");
    nameNavBar->setRightButtonText("保存");
    nameNavBar->bVisible = false;
    addWidget(nameNavBar);
    
    // 初始更新显示
    updateFieldDisplay();
}

void EditDataPage::updateFieldDisplay() {
    // 更新所有字段的选中状态
    freqSelect->bSelected = (currentFieldIndex == FIELD_FREQ);
    protocolSelect->bSelected = (currentFieldIndex == FIELD_PROTOCOL);
    bitLengthSelect->bSelected = (currentFieldIndex == FIELD_BITLENGTH);
    pulseLengthEdit->setSelected(currentFieldIndex == FIELD_PULSELENGTH);
    dataEdit->setSelected(currentFieldIndex == FIELD_DATA);
    
    // 更新值显示
    freqSelect->value = (currentFreqIndex == 0) ? "315MHz" : "433MHz";
    protocolSelect->value = String(currentProtocolIndex + 1);
    bitLengthSelect->value = String(bitLengthOptions[currentBitLengthIndex]) + "bit";
    pulseLengthEdit->setValue(currentData.rcData.pulseLength);
    dataEdit->setValue(currentData.rcData.data);
}

void EditDataPage::updateDataRangeByBitLength() {
    int bitLength = bitLengthOptions[currentBitLengthIndex];
    
    // 计算十六进制位数（每4位二进制=1位十六进制）
    int hexDigits = (bitLength + 3) / 4;  // 向上取整
    
    // 计算最大值
    unsigned long maxValue;
    if (bitLength >= 32) {
        // 32位及以上，使用unsigned long的最大值
        maxValue = 0xFFFFFFFF;
    } else {
        // 小于32位，计算实际最大值
        maxValue = (1UL << bitLength) - 1;
    }
    
    // 更新数据字段的设置
    dataEdit->setMaxDigits(hexDigits);
    dataEdit->setRange(0, maxValue);
    
    // 确保当前数据值不超过新的最大值
    if (currentData.rcData.data > maxValue) {
        currentData.rcData.data = maxValue;
    }
    dataEdit->setValue(currentData.rcData.data);
}

void EditDataPage::moveFieldUp() {
    switch (currentFieldIndex) {
        case FIELD_FREQ:
            currentFreqIndex = (currentFreqIndex == 0) ? 1 : 0;
            break;
        case FIELD_PROTOCOL:
            currentProtocolIndex--;
            if (currentProtocolIndex < 0) currentProtocolIndex = PROTOCOL_COUNT - 1;
            break;
        case FIELD_BITLENGTH:
            currentBitLengthIndex--;
            if (currentBitLengthIndex < 0) currentBitLengthIndex = BITLENGTH_COUNT - 1;
            updateDataRangeByBitLength();  // 更新数据范围
            break;
        case FIELD_PULSELENGTH:
            pulseLengthEdit->incrementDigit();
            currentData.rcData.pulseLength = pulseLengthEdit->getValue();
            return; // 不需要调用updateFieldDisplay
        case FIELD_DATA:
            dataEdit->incrementDigit();
            currentData.rcData.data = dataEdit->getValue();
            return; // 不需要调用updateFieldDisplay
    }
    updateFieldDisplay();
}

void EditDataPage::moveFieldDown() {
    switch (currentFieldIndex) {
        case FIELD_FREQ:
            currentFreqIndex = (currentFreqIndex == 0) ? 1 : 0;
            break;
        case FIELD_PROTOCOL:
            currentProtocolIndex++;
            if (currentProtocolIndex >= PROTOCOL_COUNT) currentProtocolIndex = 0;
            break;
        case FIELD_BITLENGTH:
            currentBitLengthIndex++;
            if (currentBitLengthIndex >= BITLENGTH_COUNT) currentBitLengthIndex = 0;
            updateDataRangeByBitLength();  // 更新数据范围
            break;
        case FIELD_PULSELENGTH:
            pulseLengthEdit->decrementDigit();
            currentData.rcData.pulseLength = pulseLengthEdit->getValue();
            return; // 不需要调用updateFieldDisplay
        case FIELD_DATA:
            dataEdit->decrementDigit();
            currentData.rcData.data = dataEdit->getValue();
            return; // 不需要调用updateFieldDisplay
    }
    updateFieldDisplay();
}

void EditDataPage::switchToNameEdit() {
    currentState = EDIT_NAME;
    
    // 保存字段到数据
    currentData.rcData.freqType = (currentFreqIndex == 0) ? FREQ_315 : FREQ_433;
    currentData.rcData.protocal = currentProtocolIndex + 1;
    currentData.rcData.bitLength = bitLengthOptions[currentBitLengthIndex];
    currentData.rcData.pulseLength = pulseLengthEdit->getValue();
    currentData.rcData.data = dataEdit->getValue();
    
    // 隐藏字段编辑UI
    statusLabel->bVisible = false;
    freqLabel->bVisible = false;
    protocolLabel->bVisible = false;
    bitLengthLabel->bVisible = false;
    pulseLengthLabel->bVisible = false;
    dataLabel->bVisible = false;
    freqSelect->bVisible = false;
    protocolSelect->bVisible = false;
    bitLengthSelect->bVisible = false;
    pulseLengthEdit->bVisible = false;
    dataEdit->bVisible = false;
    navBar->bVisible = false;
    
    // 显示名称编辑UI
    nameInput->bVisible = true;
    nameInput->setText(currentData.name);
    nameNavBar->bVisible = true;
}

void EditDataPage::saveAndReturn() {
    // 保存名称
    currentData.name = nameInput->getText();
    
    // 保存到存储
    dataStore.SaveData(dataIndex, currentData);
    
    // 返回上一页
    uiEngine.navigateBack();
}

String EditDataPage::generateDefaultName() {
    return "NO NAME " + String(dataIndex);
}

void EditDataPage::render(U8G2* u8g2) {
    UIPage::render(u8g2);
}

void EditDataPage::update() {
    // 更新当前活动的输入组件
    if (currentState == EDIT_NAME) {
        nameInput->update();
    }
}

void EditDataPage::onButtonBack(void* context) {
    if (currentState == EDIT_FIELDS) {
        navBar->showLeftBlink(1, 80, 80, [this]() {
            uiEngine.navigateBack();
        });
    } else if (currentState == EDIT_NAME) {
        nameNavBar->showLeftBlink(1, 80, 80, [this]() {
            // 返回字段编辑
            currentState = EDIT_FIELDS;
            nameInput->bVisible = false;
            nameNavBar->bVisible = false;
            
            statusLabel->bVisible = true;
            freqLabel->bVisible = true;
            protocolLabel->bVisible = true;
            bitLengthLabel->bVisible = true;
            pulseLengthLabel->bVisible = true;
            dataLabel->bVisible = true;
            freqSelect->bVisible = true;
            protocolSelect->bVisible = true;
            bitLengthSelect->bVisible = true;
            pulseLengthEdit->bVisible = true;
            dataEdit->bVisible = true;
            navBar->bVisible = true;
        });
    }
}

void EditDataPage::onButtonEnter(void* context) {
    if (currentState == EDIT_FIELDS) {
        navBar->showRightBlink(1, 80, 80, [this]() {
            switchToNameEdit();
        });
    } else if (currentState == EDIT_NAME) {
        nameNavBar->showRightBlink(1, 80, 80, [this]() {
            saveAndReturn();
        });
    }
}

void EditDataPage::onButtonMenu(void* context) {
    if (currentState == EDIT_NAME) {
        nameInput->deleteChar();
    }
}

void EditDataPage::onButton1(void* context) {
    if (currentState == EDIT_NAME) {
        nameInput->onButtonPress(1);
    }
}

void EditDataPage::onButton2(void* context) {
    if (currentState == EDIT_FIELDS) {
        moveFieldUp();
    } else if (currentState == EDIT_NAME) {
        nameInput->onButtonPress(2);
    }
}

void EditDataPage::onButton3(void* context) {
    if (currentState == EDIT_NAME) {
        nameInput->onButtonPress(3);
    }
}

void EditDataPage::onButton4(void* context) {
    if (currentState == EDIT_FIELDS) {
        // 4键功能：对于可编辑字段，左移光标；如果已在最左边，则切换到上一个字段
        if (currentFieldIndex == FIELD_PULSELENGTH) {
            if (pulseLengthEdit->isAtLeftBoundary()) {
                // 已经在最左边，切换到上一个字段
                currentFieldIndex--;
                if (currentFieldIndex < 0) currentFieldIndex = FIELD_COUNT - 1;
                updateFieldDisplay();
            } else {
                pulseLengthEdit->moveCursorLeft();
            }
        } else if (currentFieldIndex == FIELD_DATA) {
            if (dataEdit->isAtLeftBoundary()) {
                // 已经在最左边，切换到上一个字段
                currentFieldIndex--;
                if (currentFieldIndex < 0) currentFieldIndex = FIELD_COUNT - 1;
                updateFieldDisplay();
            } else {
                dataEdit->moveCursorLeft();
            }
        } else {
            // 对于选择型字段，直接切换到上一个字段
            currentFieldIndex--;
            if (currentFieldIndex < 0) currentFieldIndex = FIELD_COUNT - 1;
            updateFieldDisplay();
        }
    } else if (currentState == EDIT_NAME) {
        nameInput->onButtonPress(4);
    }
}

void EditDataPage::onButton5(void* context) {
    if (currentState == EDIT_NAME) {
        nameInput->onButtonPress(5);
    } else {
        onButtonEnter(context);
    }
}

void EditDataPage::onButton6(void* context) {
    if (currentState == EDIT_FIELDS) {
        // 6键功能：对于可编辑字段，右移光标；如果已在最右边，则切换到下一个字段
        if (currentFieldIndex == FIELD_PULSELENGTH) {
            if (pulseLengthEdit->isAtRightBoundary()) {
                // 已经在最右边，切换到下一个字段
                currentFieldIndex++;
                if (currentFieldIndex >= FIELD_COUNT) currentFieldIndex = 0;
                updateFieldDisplay();
            } else {
                pulseLengthEdit->moveCursorRight();
            }
        } else if (currentFieldIndex == FIELD_DATA) {
            if (dataEdit->isAtRightBoundary()) {
                // 已经在最右边，切换到下一个字段
                currentFieldIndex++;
                if (currentFieldIndex >= FIELD_COUNT) currentFieldIndex = 0;
                updateFieldDisplay();
            } else {
                dataEdit->moveCursorRight();
            }
        } else {
            // 对于选择型字段，直接切换到下一个字段
            currentFieldIndex++;
            if (currentFieldIndex >= FIELD_COUNT) currentFieldIndex = 0;
            updateFieldDisplay();
        }
    } else if (currentState == EDIT_NAME) {
        nameInput->onButtonPress(6);
    }
}

void EditDataPage::onButton7(void* context) {
    if (currentState == EDIT_NAME) {
        nameInput->onButtonPress(7);
    }
}

void EditDataPage::onButton8(void* context) {
    if (currentState == EDIT_FIELDS) {
        moveFieldDown();
    } else if (currentState == EDIT_NAME) {
        nameInput->onButtonPress(8);
    }
}

void EditDataPage::onButton9(void* context) {
    if (currentState == EDIT_NAME) {
        nameInput->onButtonPress(9);
    }
}
