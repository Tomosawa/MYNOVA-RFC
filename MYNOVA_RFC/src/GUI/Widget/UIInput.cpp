/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "UIInput.h"
#include <U8g2lib.h>

// T9键盘映射: 1-9键对应的字符
// 1: 标点符号, 2: ABCabc, 3: DEFdef, etc.
const char* UIInput::keyMap[9] = {
  " .,?!1",     // 1键
  "ABCabc2",   // 2键
  "DEFdef3",   // 3键
  "GHIghi4",   // 4键
  "JKLjkl5",   // 5键
  "MNOmno6",   // 6键
  "PQRSpqrs7", // 7键
  "TUVtuv8",   // 8键
  "WXYZwxyz9"  // 9键
};

UIInput::UIInput(int x, int y, int width, int height) {
  this->x = x;
  this->y = y;
  this->width = width;
  this->height = height;
  
  currentText = "";
  title = "";
  lastKeyPressTime = 0;
  cursorVisible = true;
  cursorBlinkTime = millis();
  currentKeyIndex = -1;
  currentCharIndex = 0;
  keyPressStartTime = 0;
  multiTapMode = false;
  textAlign = INPUT_LEFT;
}

void UIInput::render(U8G2* u8g2, int offsetX, int offsetY) {
  // 检查是否可见
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
    int titleX = actualX + width / 2 - titleWidth / 2; // 居中显示标题
    int titleY = actualY + 12; // 标题在顶部10像素处
    u8g2->drawUTF8(titleX, titleY, title.c_str());
  }
  
  // 计算文本绘制位置
  int fontHeight = u8g2->getMaxCharHeight();
  int textBaseY = actualY + height / 2 + fontHeight / 2;
  // 如果有标题，文本位置需要下移
  if (title.length() > 0) {
    textBaseY = actualY + height / 2 + fontHeight / 2 + 10;
  }
  
  // 获取当前文本宽度
  uint16_t textWidth = u8g2->getUTF8Width(currentText.c_str());
  
  // 根据对齐方式确定文本绘制位置
  int textX = actualX + 3; // 默认LEFT对齐
  if (textAlign == INPUT_CENTER) {
    textX = actualX + width / 2 - textWidth / 2;
  } else if (textAlign == INPUT_RIGHT) {
    textX = actualX + width - textWidth - 3;
  }
  
  // 绘制当前文本
  u8g2->drawUTF8(textX, textBaseY, currentText.c_str());
  
  // 绘制光标（如果可见）
  if (cursorVisible) {
    // 光标在文字底部显示为一条横线
    int cursorY = textBaseY + 1; // 文字底部下方1像素
    
    // 判断是否处于输入状态（有当前按键且未超时）
    bool isInputting = (currentKeyIndex != -1) && 
                      ((millis() - lastKeyPressTime) < KEY_TIMEOUT);
    
    if (isInputting && currentText.length() > 0) {
      // 输入过程中，光标在当前最后一个字符底部
      String lastChar = currentText.substring(currentText.length() - 1);
      int lastCharWidth = u8g2->getUTF8Width(lastChar.c_str());
      
      // 计算最后一个字符的位置
      int lastCharX = textX + textWidth - lastCharWidth;
      
      // 绘制底部横线光标（在当前字符底部）
      u8g2->drawHLine(lastCharX, cursorY, lastCharWidth);
    } else {
      // 输入完成或空文本状态，光标在文本末尾空位
      int cursorWidth = 8; // 默认光标宽度
      
      // 如果有文本，光标宽度可以基于最后一个字符的宽度调整
      if (currentText.length() > 0) {
        String lastChar = currentText.substring(currentText.length() - 1);
        cursorWidth = u8g2->getUTF8Width(lastChar.c_str());
        // 但至少保持8像素宽度，确保可见性
        if (cursorWidth < 8) cursorWidth = 8;
      }
      
      // 光标位置在文本末尾
      int cursorX = textX + textWidth;
      
      // 绘制底部横线光标
      u8g2->drawHLine(cursorX, cursorY, cursorWidth);
    }
  }
}

void UIInput::setText(String text) {
  currentText = text;
  lastKeyPressTime = millis();
}

String UIInput::getText() {
  return currentText;
}

void UIInput::clearText() {
  currentText = "";
  currentKeyIndex = -1;
  currentCharIndex = 0;
  multiTapMode = false;
  lastKeyPressTime = millis();
}

void UIInput::deleteChar() {
  if (currentText.length() > 0) {
    currentText.remove(currentText.length() - 1);
  }
  currentKeyIndex = -1;
  currentCharIndex = 0;
  multiTapMode = false;
  lastKeyPressTime = millis();
}

void UIInput::onButtonPress(int buttonNumber) {
  // 确保按键编号在有效范围内
  if (buttonNumber < 1 || buttonNumber > 9) return;
  
  unsigned long currentTime = millis();
  
  // 检查是否是连续按同一个键
  if (currentKeyIndex == buttonNumber && (currentTime - keyPressStartTime) < 1000) {
    // 同一键位多次点击，切换字符
    multiTapMode = true;
    currentCharIndex = (currentCharIndex + 1) % strlen(keyMap[buttonNumber - 1]);
    
    // 如果是在末尾添加字符，需要先删除之前添加的字符
    if (currentText.length() > 0) {
      currentText.remove(currentText.length() - 1);
    }
  } else {
    // 新的按键或者超时后的按键
    multiTapMode = false;
    currentKeyIndex = buttonNumber;
    currentCharIndex = 0;
  }
  
  // 更新按键时间
  keyPressStartTime = currentTime;
  lastKeyPressTime = currentTime;
  
  // 添加当前字符到文本
  char newChar = keyMap[buttonNumber - 1][currentCharIndex];
  currentText += newChar;
  
  // 重置光标可见性
  cursorVisible = true;
  cursorBlinkTime = currentTime;
}

void UIInput::update() {
  unsigned long currentTime = millis();
  
  // 检查是否超时（2秒无操作）
  if ((currentTime - lastKeyPressTime) > KEY_TIMEOUT) {
    // 超时，重置按键状态
    currentKeyIndex = -1;
    currentCharIndex = 0;
    multiTapMode = false;
  }
  
  // 更新光标闪烁状态
  if ((currentTime - cursorBlinkTime) > CURSOR_BLINK_INTERVAL) {
    cursorVisible = !cursorVisible;
    cursorBlinkTime = currentTime;
  }
}

void UIInput::setTextAlign(InputTextAlign align) {
  textAlign = align;
}

void UIInput::setTitle(String title) {
  this->title = title;
}