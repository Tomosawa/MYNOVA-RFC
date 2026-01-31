/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef UIProgressBar_h
#define UIProgressBar_h

#include "UIWidget.h"
#include <Arduino.h>
#include <U8g2lib.h>

class UIProgressBar : public UIWidget {
public:
  UIProgressBar();
  void render(U8G2* u8g2,int offsetX = 0, int offsetY = 0) override;
  
  void setValue(int val); // 设置进度值 0-100
  int getValue() const;   // 获取当前进度值
  void setShowText(bool show); // 设置是否显示文字
  void setShowBorder(bool show); // 设置是否显示边框

public:
  int value;              // 进度值 0-100
  bool bShowText;         // 是否显示文字百分比
  bool bShowBorder;       // 是否显示边框
  const uint8_t* textFont; // 文字字体
};

#endif
