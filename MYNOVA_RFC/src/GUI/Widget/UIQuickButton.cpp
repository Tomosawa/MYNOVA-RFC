/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "UIQuickButton.h"
#include <U8g2lib.h>
#include "src/GUI/Animation/QuickButtonAnimation.h"
#include "src/GUI/Animation/AnimationEngine.h"

UIQuickButton::UIQuickButton() {
  // 初始化按钮的标签
  padding_top = 1;
  padding_bottom = 1;
}

void UIQuickButton::render(U8G2* u8g2,int offsetX, int offsetY) {
  // 检查是否可见
  if (!bVisible) return;
  
  //画边框
  int drawX = margin_left + x + offsetX;
  int drawY = margin_top + y + offsetY;
  if(bHighLight)
  { 
    u8g2->drawRBox(drawX, drawY, width, height, roundCorner);
  }
  else
    u8g2->drawRFrame(drawX, drawY, width, height, roundCorner);

  //按钮文字
  u8g2->setFont(u8g2_font_4x6_tf);
  u8g2->setFontMode(1);
  u8g2->setDrawColor(2);
  int charHeight = u8g2->getMaxCharHeight();
  int strWidth = u8g2->getStrWidth(keyNum.c_str());
  drawX = margin_left + x + offsetX + width / 2 - strWidth / 2;
  drawY = margin_top + y + offsetY + padding_top + charHeight;
  u8g2->drawStr(drawX,drawY,keyNum.c_str());

  strWidth = u8g2->getStrWidth(label.c_str());
  drawX = margin_left + x + offsetX + width / 2 - strWidth / 2;
  drawY = margin_top + y + offsetY + height - padding_bottom;
  u8g2->drawStr(drawX,drawY,label.c_str());

  u8g2->setFontMode(0);
  u8g2->setDrawColor(1);
}

void UIQuickButton::showHighLight()
{
  // 设置高亮状态并启动动画（默认持续高亮）
  bHighLight = true;
  
  // 创建高亮动画并添加到动画引擎
  QuickButtonAnimation* highlightAnim = new QuickButtonAnimation(this, 300);
  animationEngine.addAnimation(highlightAnim);
}

void UIQuickButton::showBlink(int blinkCount, int onDuration, int offDuration)
{
  // 设置高亮状态并启动闪烁动画
  bHighLight = true;
  
  // 创建闪烁动画并添加到动画引擎
  QuickButtonAnimation* blinkAnim = new QuickButtonAnimation(this, (onDuration + offDuration) * blinkCount);
  blinkAnim->setBlinkMode(blinkCount, onDuration, offDuration);
  animationEngine.addAnimation(blinkAnim);
}