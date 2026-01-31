/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef UINumberInput_h
#define UINumberInput_h

#include "UIWidget.h"
#include <Arduino.h>
#include <U8g2lib.h>

enum NumberInputMode {
  NUMBER_MODE_DECIMAL,    // 纯数字模式 0-9
  NUMBER_MODE_HEX         // 十六进制模式 0-F
};

/**
 * UINumberInput - 数字输入组件
 * 支持纯数字和十六进制输入
 * 左右按键移动光标位，上下按键调整数值
 */
class UINumberInput : public UIWidget {
public:
  UINumberInput(int x, int y, int width, int height);
  void render(U8G2* u8g2, int offsetX = 0, int offsetY = 0) override;
  
  // 设置输入模式
  void setInputMode(NumberInputMode mode);
  
  // 设置最大位数
  void setMaxDigits(int maxDigits);
  
  // 设置当前值
  void setValue(unsigned long value);
  
  // 获取当前值
  unsigned long getValue();
  
  // 设置标题
  void setTitle(String title);
  
  // 左移光标
  void moveCursorLeft();
  
  // 右移光标
  void moveCursorRight();
  
  // 增加当前位数值
  void incrementDigit();
  
  // 减少当前位数值
  void decrementDigit();
  
  // 更新光标闪烁状态
  void update();
  
  // 获取格式化的字符串（十六进制时带空格，如 "74 C5 72"）
  String getFormattedString();

private:
  NumberInputMode inputMode;
  unsigned long currentValue;
  int maxDigits;              // 最大位数
  int cursorPosition;         // 当前光标位置（从右到左，0是个位）
  String title;
  bool cursorVisible;
  unsigned long cursorBlinkTime;
  static const unsigned long CURSOR_BLINK_INTERVAL = 500;
  
  // 获取指定位置的数字（十六进制或十进制）
  int getDigitAt(int position);
  
  // 设置指定位置的数字
  void setDigitAt(int position, int digit);
  
  // 格式化显示字符串
  String formatDisplay();
};

#endif

