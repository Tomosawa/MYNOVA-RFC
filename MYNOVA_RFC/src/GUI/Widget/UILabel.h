/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef UILabel_h
#define UILabel_h

#include "UIWidget.h"
#include <Arduino.h>
#include <U8g2lib.h>

enum TextAlign{
  LEFT,
  RIGHT,
  CENTER
};

enum VerticalAlign{
  TOP,
  MIDDLE,
  BOTTOM
};

class UILabel : public UIWidget {
public:
  UILabel();
  void render(U8G2* u8g2,int offsetX = 0, int offsetY = 0) override;

public:
  String label;
  const uint8_t* textFont = u8g2_font_wqy12_t_gb2312;//字体
  TextAlign textAlign = CENTER;
  VerticalAlign verticalAlign = MIDDLE;
};

#endif
