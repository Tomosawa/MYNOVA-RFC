/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef UIEditableNumber_h
#define UIEditableNumber_h

#include "UIWidget.h"
#include <Arduino.h>
#include <U8g2lib.h>

enum EditableNumberMode {
    EDITABLE_DECIMAL,    // 十进制模式
    EDITABLE_HEX         // 十六进制模式（带空格分隔，如 7A 3B DC）
};

/**
 * UIEditableNumber - 可逐位编辑的数字组件
 * 支持十进制和十六进制，可以逐位选中和修改
 */
class UIEditableNumber : public UIWidget {
public:
    UIEditableNumber();
    void render(U8G2* u8g2, int offsetX = 0, int offsetY = 0) override;
    
    // 设置模式
    void setMode(EditableNumberMode mode);
    
    // 设置/获取值
    void setValue(unsigned long value);
    unsigned long getValue();
    
    // 设置最大位数（十进制）或字节数（十六进制）
    void setMaxDigits(int maxDigits);
    
    // 设置最大值和最小值
    void setMinValue(unsigned long minValue);
    void setMaxValue(unsigned long maxValue);
    void setRange(unsigned long minValue, unsigned long maxValue);
    
    // 光标移动
    void moveCursorLeft();
    void moveCursorRight();
    
    // 增减当前位的值
    void incrementDigit();
    void decrementDigit();
    
    // 设置是否被选中（用于字段切换）
    void setSelected(bool selected);
    bool isSelected();
    
    // 检查光标是否在边界
    bool isAtLeftBoundary();   // 光标是否在最左边（最高位）
    bool isAtRightBoundary();  // 光标是否在最右边（最低位）
    
public:
    const uint8_t* textFont = u8g2_font_wqy12_t_gb2312;
    
private:
    EditableNumberMode mode;
    unsigned long value;
    int maxDigits;              // 最大位数
    int cursorPosition;         // 光标位置（从右往左，0是个位/最低位）
    bool selected;              // 是否被选中
    unsigned long minValue;     // 最小值
    unsigned long maxValue;     // 最大值
    
    // 获取指定位置的数字
    int getDigitAt(int position);
    
    // 设置指定位置的数字
    void setDigitAt(int position, int digit);
    
    // 格式化显示字符串
    String formatDisplay();
    
    // 获取实际的字符串表示（用于渲染时计算光标位置）
    String getDisplayString();
};

#endif

