/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef UIWidget_h
#define UIWidget_h

#include <Arduino.h>
#include <U8g2lib.h>

class UIWidget {
public:
  UIWidget();
  virtual ~UIWidget();
  virtual void render(U8G2* u8g2,int offsetX = 0, int offsetY = 0);
public:
  int x, y, width, height;
  int margin_left, margin_top, margin_right, margin_bottom;
  int padding_left, padding_top, padding_right, padding_bottom;
  bool bVisible;  // 控制组件是否可见

};

#endif
