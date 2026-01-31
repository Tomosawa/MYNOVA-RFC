/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef UIQuickButton_h
#define UIQuickButton_h

#include "UIWidget.h"
#include <Arduino.h>
#include <U8g2lib.h>

class UIQuickButton : public UIWidget {
public:
  UIQuickButton();
  void render(U8G2* u8g2,int offsetX = 0, int offsetY = 0) override;
  void showHighLight();
  void showBlink(int blinkCount = 3, int onDuration = 200, int offDuration = 200);
  
public:
  String keyNum;
  String label;
  int roundCorner = 2;
  bool bHighLight = false;  // 将此变量改为public，以便动画类可以访问

private:
  // 移除内部动画相关的变量
};

#endif