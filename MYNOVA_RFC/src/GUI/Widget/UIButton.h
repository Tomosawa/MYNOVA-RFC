/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef UIButton_h
#define UIButton_h

#include "UIWidget.h"
#include <Arduino.h>
#include <U8g2lib.h>

class UIButton : public UIWidget {
public:
  UIButton();
  void render(U8G2* u8g2, int offsetX = 0, int offsetY = 0) override;

private:
  String label;
};

#endif
