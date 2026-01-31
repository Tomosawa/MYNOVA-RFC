/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "UINumberInput.h"

UINumberInput::UINumberInput(int x, int y, int width, int height) {
  this->x = x;
  this->y = y;
  this->width = width;
  this->height = height;
  
  inputMode = NUMBER_MODE_DECIMAL;
  currentValue = 0;
  maxDigits = 6;  // 默认6位
  cursorPosition = 0;
  title = "";
  cursorVisible = true;
  cursorBlinkTime = millis();
}

void UINumberInput::render(U8G2* u8g2, int offsetX, int offsetY) {
  if (!bVisible) return;
  
  int actualX = offsetX + x;
  int actualY = offsetY + y;
  
  // 绘制边框
  u8g2->drawFrame(actualX, actualY, width, height);
  
  // 设置字体
  u8g2->setFont(u8g2_font_wqy12_t_gb2312);
  
  // 绘制标题（如果存在）
  if (title.length() > 0) {
    uint16_t titleWidth = u8g2->getUTF8Width(title.c_str());
    int titleX = actualX + width / 2 - titleWidth / 2;
    int titleY = actualY + 10;
    u8g2->drawUTF8(titleX, titleY, title.c_str());
  }
  
  // 格式化显示的数字
  String displayStr = formatDisplay();
  
  // 计算数字显示位置
  int fontHeight = u8g2->getMaxCharHeight();
  int textBaseY = actualY + height / 2 + fontHeight / 2;
  if (title.length() > 0) {
    textBaseY = actualY + height / 2 + fontHeight / 2 + 8;
  }
  
  uint16_t textWidth = u8g2->getStrWidth(displayStr.c_str());
  int textX = actualX + width / 2 - textWidth / 2;  // 居中显示
  
  // 绘制数字
  u8g2->drawStr(textX, textBaseY, displayStr.c_str());
  
  // 绘制光标（在当前编辑位下方）
  if (cursorVisible) {
    String strForCursor = formatDisplay();
    int digitCount = 0;
    int charPos = strForCursor.length() - 1;
    
    // 根据光标位置找到对应的字符位置（从右往左）
    if (inputMode == NUMBER_MODE_HEX) {
      // 十六进制模式，需要考虑空格
      // 格式如 "00 74 C5 72"，从右往左数
      int spaceCount = 0;
      for (int i = charPos; i >= 0 && digitCount <= cursorPosition; i--) {
        if (strForCursor[i] == ' ') {
          spaceCount++;
          continue;
        }
        if (digitCount == cursorPosition) {
          charPos = i;
          break;
        }
        digitCount++;
      }
    } else {
      // 十进制模式，直接从右往左数
      charPos = strForCursor.length() - 1 - cursorPosition;
      if (charPos < 0) charPos = 0;
    }
    
    // 计算光标下划线的位置
    String leftPart = strForCursor.substring(0, charPos);
    String currentChar = strForCursor.substring(charPos, charPos + 1);
    
    int leftPartWidth = u8g2->getStrWidth(leftPart.c_str());
    int charWidth = u8g2->getStrWidth(currentChar.c_str());
    
    int cursorX = textX + leftPartWidth;
    int cursorY = textBaseY + 2;
    
    // 绘制光标下划线
    u8g2->drawHLine(cursorX, cursorY, charWidth);
  }
}

void UINumberInput::setInputMode(NumberInputMode mode) {
  inputMode = mode;
}

void UINumberInput::setMaxDigits(int maxDigits) {
  this->maxDigits = maxDigits;
  if (this->maxDigits > 10) this->maxDigits = 10;  // 最大支持10位
  if (this->maxDigits < 1) this->maxDigits = 1;
}

void UINumberInput::setValue(unsigned long value) {
  currentValue = value;
  // 确保光标位置有效
  if (cursorPosition >= maxDigits) {
    cursorPosition = maxDigits - 1;
  }
}

unsigned long UINumberInput::getValue() {
  return currentValue;
}

void UINumberInput::setTitle(String title) {
  this->title = title;
}

void UINumberInput::moveCursorLeft() {
  cursorPosition++;
  if (cursorPosition >= maxDigits) {
    cursorPosition = maxDigits - 1;
  }
  cursorVisible = true;
  cursorBlinkTime = millis();
}

void UINumberInput::moveCursorRight() {
  cursorPosition--;
  if (cursorPosition < 0) {
    cursorPosition = 0;
  }
  cursorVisible = true;
  cursorBlinkTime = millis();
}

void UINumberInput::incrementDigit() {
  int digit = getDigitAt(cursorPosition);
  int maxValue = (inputMode == NUMBER_MODE_HEX) ? 15 : 9;
  
  digit++;
  if (digit > maxValue) {
    digit = 0;  // 循环
  }
  
  setDigitAt(cursorPosition, digit);
  cursorVisible = true;
  cursorBlinkTime = millis();
}

void UINumberInput::decrementDigit() {
  int digit = getDigitAt(cursorPosition);
  int maxValue = (inputMode == NUMBER_MODE_HEX) ? 15 : 9;
  
  digit--;
  if (digit < 0) {
    digit = maxValue;  // 循环
  }
  
  setDigitAt(cursorPosition, digit);
  cursorVisible = true;
  cursorBlinkTime = millis();
}

void UINumberInput::update() {
  unsigned long currentTime = millis();
  
  // 更新光标闪烁状态
  if ((currentTime - cursorBlinkTime) > CURSOR_BLINK_INTERVAL) {
    cursorVisible = !cursorVisible;
    cursorBlinkTime = currentTime;
  }
}

String UINumberInput::getFormattedString() {
  if (inputMode == NUMBER_MODE_HEX) {
    String hex = String(currentValue, HEX);
    hex.toUpperCase();
    
    // 补齐为偶数位
    if (hex.length() % 2 != 0) {
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
    return String(currentValue);
  }
}

int UINumberInput::getDigitAt(int position) {
  if (inputMode == NUMBER_MODE_HEX) {
    // 十六进制，每位是16进制
    unsigned long divisor = 1;
    for (int i = 0; i < position; i++) {
      divisor *= 16;
    }
    return (currentValue / divisor) % 16;
  } else {
    // 十进制
    unsigned long divisor = 1;
    for (int i = 0; i < position; i++) {
      divisor *= 10;
    }
    return (currentValue / divisor) % 10;
  }
}

void UINumberInput::setDigitAt(int position, int digit) {
  int oldDigit = getDigitAt(position);
  
  if (inputMode == NUMBER_MODE_HEX) {
    unsigned long divisor = 1;
    for (int i = 0; i < position; i++) {
      divisor *= 16;
    }
    currentValue = currentValue - (oldDigit * divisor) + (digit * divisor);
  } else {
    unsigned long divisor = 1;
    for (int i = 0; i < position; i++) {
      divisor *= 10;
    }
    currentValue = currentValue - (oldDigit * divisor) + (digit * divisor);
  }
}

String UINumberInput::formatDisplay() {
  if (inputMode == NUMBER_MODE_HEX) {
    String hex = String(currentValue, HEX);
    hex.toUpperCase();
    
    // 补齐到指定位数
    int targetLength = maxDigits;
    while (hex.length() < targetLength) {
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
    // 十进制显示
    String dec = String(currentValue);
    
    // 补齐到指定位数
    while (dec.length() < maxDigits) {
      dec = "0" + dec;
    }
    
    return dec;
  }
}

