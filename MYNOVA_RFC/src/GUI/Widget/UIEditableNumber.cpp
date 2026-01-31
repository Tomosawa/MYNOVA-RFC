/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "UIEditableNumber.h"

UIEditableNumber::UIEditableNumber() {
    mode = EDITABLE_DECIMAL;
    value = 0;
    maxDigits = 3;
    cursorPosition = 0;
    selected = false;
    minValue = 0;
    maxValue = 0xFFFFFFFF;  // 默认最大值为unsigned long的最大值
    x = 0;
    y = 0;
    width = 50;
    height = 12;
    bVisible = true;
}

void UIEditableNumber::render(U8G2* u8g2, int offsetX, int offsetY) {
    if (!bVisible) return;
    
    int actualX = offsetX + x;
    int actualY = offsetY + y + 1;
    
    u8g2->setFont(textFont);
    
    // 获取显示字符串
    String displayStr = getDisplayString();
    
    // 计算文字Y坐标（参考UILabel的实现）
    int fontHeight = u8g2->getMaxCharHeight();
    int textY = actualY + (height / 2) + (fontHeight / 2) - 2;
    
    // 如果被选中，绘制整体背景框（可选）
    if (selected) {
        int strWidth = u8g2->getStrWidth(displayStr.c_str());
        // 绘制一个浅色的边框表示选中
        u8g2->drawFrame(actualX - 1, actualY, width, height);
    }
    
    // 绘制文本
    u8g2->drawStr(actualX, textY, displayStr.c_str());
    
    // 如果被选中，绘制当前编辑位的高亮
    if (selected) {
        // 计算当前编辑位的字符位置
        int charIndex = 0;
        if (mode == EDITABLE_HEX) {
            // 十六进制模式：格式如 "7A 3B DC"
            // cursorPosition 0 = 最右边的C (DC的C)
            // cursorPosition 1 = 最右边的D (DC的D)
            // cursorPosition 2 = 3B的B
            // cursorPosition 3 = 3B的3
            // 每个字节2个字符，字节间有空格
            int byteIndex = cursorPosition / 2;  // 第几个字节
            int digitInByte = cursorPosition % 2; // 字节内的位置（0=高位，1=低位）
            
            // 从右往左数字节
            int totalBytes = (maxDigits + 1) / 2;
            int byteFromLeft = totalBytes - 1 - byteIndex;
            
            // 计算字符索引：每个字节3个字符（2个hex + 1个空格），但最后一个字节没有空格
            charIndex = byteFromLeft * 3 + (1 - digitInByte);
            if (byteFromLeft == totalBytes - 1) {
                charIndex = byteFromLeft * 3 + (1 - digitInByte);
            }
        } else {
            // 十进制模式：直接从右往左数
            charIndex = displayStr.length() - 1 - cursorPosition;
        }
        
        if (charIndex >= 0 && charIndex < displayStr.length()) {
            // 计算该字符的位置
            String leftPart = displayStr.substring(0, charIndex);
            String highlightChar = displayStr.substring(charIndex, charIndex + 1);
            
            int leftWidth = u8g2->getStrWidth(leftPart.c_str());
            int charWidth = u8g2->getStrWidth(highlightChar.c_str());
            
            // 反色显示当前字符
            u8g2->setDrawColor(2); // XOR模式
            u8g2->drawBox(actualX + leftWidth, actualY + 1, charWidth + 2, height - 2);
            u8g2->setDrawColor(1); // 恢复正常模式
        }
    }
}

void UIEditableNumber::setMode(EditableNumberMode mode) {
    this->mode = mode;
}

void UIEditableNumber::setValue(unsigned long value) {
    // 确保值在范围内
    if (value < minValue) {
        this->value = minValue;
    } else if (value > maxValue) {
        this->value = maxValue;
    } else {
        this->value = value;
    }
}

unsigned long UIEditableNumber::getValue() {
    return value;
}

void UIEditableNumber::setMaxDigits(int maxDigits) {
    this->maxDigits = maxDigits;
}

void UIEditableNumber::setMinValue(unsigned long minValue) {
    this->minValue = minValue;
    // 确保当前值在范围内
    if (value < minValue) {
        value = minValue;
    }
}

void UIEditableNumber::setMaxValue(unsigned long maxValue) {
    this->maxValue = maxValue;
    // 确保当前值在范围内
    if (value > maxValue) {
        value = maxValue;
    }
}

void UIEditableNumber::setRange(unsigned long minValue, unsigned long maxValue) {
    this->minValue = minValue;
    this->maxValue = maxValue;
    // 确保当前值在范围内
    if (value < minValue) {
        value = minValue;
    } else if (value > maxValue) {
        value = maxValue;
    }
}

void UIEditableNumber::moveCursorLeft() {
    cursorPosition++;
    if (cursorPosition >= maxDigits) {
        cursorPosition = maxDigits - 1;
    }
}

void UIEditableNumber::moveCursorRight() {
    cursorPosition--;
    if (cursorPosition < 0) {
        cursorPosition = 0;
    }
}

void UIEditableNumber::incrementDigit() {
    int digit = getDigitAt(cursorPosition);
    int maxDigitValue = (mode == EDITABLE_HEX) ? 15 : 9;
    
    digit++;
    if (digit > maxDigitValue) {
        digit = 0;
    }
    
    // 临时保存旧值
    unsigned long oldValue = value;
    setDigitAt(cursorPosition, digit);
    
    // 检查是否超出范围
    if (value > maxValue) {
        // 超出最大值，恢复旧值
        value = oldValue;
    }
}

void UIEditableNumber::decrementDigit() {
    int digit = getDigitAt(cursorPosition);
    int maxDigitValue = (mode == EDITABLE_HEX) ? 15 : 9;
    
    digit--;
    if (digit < 0) {
        digit = maxDigitValue;
    }
    
    // 临时保存旧值
    unsigned long oldValue = value;
    setDigitAt(cursorPosition, digit);
    
    // 检查是否超出范围
    if (value < minValue) {
        // 低于最小值，恢复旧值
        value = oldValue;
    }
}

void UIEditableNumber::setSelected(bool selected) {
    this->selected = selected;
}

bool UIEditableNumber::isSelected() {
    return selected;
}

bool UIEditableNumber::isAtLeftBoundary() {
    return cursorPosition >= (maxDigits - 1);
}

bool UIEditableNumber::isAtRightBoundary() {
    return cursorPosition <= 0;
}

int UIEditableNumber::getDigitAt(int position) {
    if (mode == EDITABLE_HEX) {
        // 十六进制，每位是16进制
        unsigned long divisor = 1;
        for (int i = 0; i < position; i++) {
            divisor *= 16;
        }
        return (value / divisor) % 16;
    } else {
        // 十进制
        unsigned long divisor = 1;
        for (int i = 0; i < position; i++) {
            divisor *= 10;
        }
        return (value / divisor) % 10;
    }
}

void UIEditableNumber::setDigitAt(int position, int digit) {
    int oldDigit = getDigitAt(position);
    
    if (mode == EDITABLE_HEX) {
        unsigned long divisor = 1;
        for (int i = 0; i < position; i++) {
            divisor *= 16;
        }
        value = value - (oldDigit * divisor) + (digit * divisor);
    } else {
        unsigned long divisor = 1;
        for (int i = 0; i < position; i++) {
            divisor *= 10;
        }
        value = value - (oldDigit * divisor) + (digit * divisor);
    }
}

String UIEditableNumber::formatDisplay() {
    return getDisplayString();
}

String UIEditableNumber::getDisplayString() {
    if (mode == EDITABLE_HEX) {
        // 十六进制模式，显示为 "7A 3B DC" 格式
        String hex = String(value, HEX);
        hex.toUpperCase();
        
        // 补齐到指定位数
        while (hex.length() < maxDigits) {
            hex = "0" + hex;
        }
        
        // 每两个字符加一个空格
        String formatted = "";
        for (int i = 0; i < hex.length(); i += 2) {
            if (i > 0) formatted += " ";
            formatted += hex.substring(i, i + 2);
        }
        
        return formatted;
    } else {
        // 十进制模式
        String dec = String(value);
        
        // 补齐到指定位数
        while (dec.length() < maxDigits) {
            dec = "0" + dec;
        }
        
        return dec;
    }
}

