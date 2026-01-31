/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef UIInput_h
#define UIInput_h

#include "UIWidget.h"
#include <Arduino.h>
#include <U8g2lib.h>

enum InputTextAlign{
  INPUT_LEFT,
  INPUT_CENTER,
  INPUT_RIGHT
};

class UIInput : public UIWidget {
public:
  UIInput(int x, int y, int width, int height);
  void render(U8G2* u8g2, int offsetX = 0, int offsetY = 0) override;
  
  // 设置初始文本
  void setText(String text);
  
  // 获取当前文本
  String getText();
  
  // 清空文本
  void clearText();
  
  // 删除最后一个字符
  void deleteChar();
  
  // 处理数字按键输入
  void onButtonPress(int buttonNumber); // 1-9的数字按键
  
  // 更新光标闪烁状态
  void update();
  
  // 设置文本对齐方式
  void setTextAlign(InputTextAlign align);
  
  // 设置标题
  void setTitle(String title);

private:
  String currentText;
  String title;
  unsigned long lastKeyPressTime;
  bool cursorVisible;
  unsigned long cursorBlinkTime;
  static const unsigned long KEY_TIMEOUT = 2000; // 2秒无操作视为输入完成
  static const unsigned long CURSOR_BLINK_INTERVAL = 500; // 光标闪烁间隔
  InputTextAlign textAlign;
  
  // T9键盘映射
  static const char* keyMap[9];
  
  int currentKeyIndex; // 当前按键索引 (1-9)
  int currentCharIndex; // 当前字符在按键字符集中的索引
  unsigned long keyPressStartTime; // 当前按键按下的时间
  bool multiTapMode; // 是否处于多次点击同一按键模式
};

#endif