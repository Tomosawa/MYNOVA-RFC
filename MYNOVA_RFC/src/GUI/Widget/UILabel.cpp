/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "UILabel.h"
#include <U8g2lib.h>

UILabel::UILabel() {
  // 初始化按钮的标签
}

void UILabel::render(U8G2* u8g2,int offsetX, int offsetY) {
  // 检查是否可见
  if (!bVisible) return;
  
  int8_t maxCharHeight = u8g2->getMaxCharHeight();
  if(textAlign == LEFT && verticalAlign == TOP)
  {
    u8g2->drawUTF8(x + margin_left, y + margin_top + maxCharHeight, label.c_str());
  }
  else if(textAlign == LEFT && verticalAlign == MIDDLE)
  {
    u8g2->drawUTF8(x + margin_left, y + height / 2 + maxCharHeight / 2, label.c_str());
  }
  else if(textAlign == LEFT && verticalAlign == BOTTOM)
  {
    u8g2->drawUTF8(x + margin_left, y + height - margin_bottom, label.c_str());
  }
  else if(textAlign == CENTER && verticalAlign == TOP)
  {
    uint16_t strWidth = u8g2->getUTF8Width(label.c_str());
    u8g2->drawUTF8(x + width / 2 - strWidth / 2, y + margin_top + maxCharHeight, label.c_str());
  }
  else if(textAlign == CENTER && verticalAlign == MIDDLE)
  {
    uint16_t strWidth = u8g2->getUTF8Width(label.c_str());
    u8g2->drawUTF8(x + width / 2 - strWidth / 2, y + height / 2 + maxCharHeight / 2, label.c_str());
  }
  else if(textAlign == CENTER && verticalAlign == BOTTOM)
  {
    uint16_t strWidth = u8g2->getUTF8Width(label.c_str());
    u8g2->drawUTF8(x + width / 2 - strWidth / 2, y + height - margin_bottom, label.c_str());
  }
  else if(textAlign == RIGHT && verticalAlign == TOP)
  {
    uint16_t strWidth = u8g2->getUTF8Width(label.c_str());
    u8g2->drawUTF8(x + width - margin_right - strWidth, y + margin_top + maxCharHeight, label.c_str());
  }
  else if(textAlign == RIGHT && verticalAlign == MIDDLE)
  {
    uint16_t strWidth = u8g2->getUTF8Width(label.c_str());
    u8g2->drawUTF8(x + width - margin_right - strWidth, y + height / 2 + maxCharHeight / 2, label.c_str());
  }
  else if(textAlign == RIGHT && verticalAlign == BOTTOM)
  {
    uint16_t strWidth = u8g2->getUTF8Width(label.c_str());
    u8g2->drawUTF8(x + width - margin_right - strWidth, y + height - margin_bottom, label.c_str());
  }
}
