/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "UIProgressBar.h"
#include <U8g2lib.h>

UIProgressBar::UIProgressBar() {
  value = 0;
  bShowText = true;
  bShowBorder = true;
  textFont = u8g2_font_wqy12_t_gb2312;
}

void UIProgressBar::setValue(int val) {
  if (val < 0) val = 0;
  if (val > 100) val = 100;
  value = val;
}

int UIProgressBar::getValue() const {
  return value;
}

void UIProgressBar::setShowText(bool show) {
  bShowText = show;
}

void UIProgressBar::setShowBorder(bool show) {
  bShowBorder = show;
}

void UIProgressBar::render(U8G2* u8g2, int offsetX, int offsetY) {
  if (!bVisible) return;

  int absX = x + offsetX;
  int absY = y + offsetY;
  
  // 绘制边框
  if (bShowBorder) {
    u8g2->drawFrame(absX, absY, width, height);
  }
  
  // 计算进度条填充宽度（减去边框）
  int innerWidth = width - 4;
  int innerHeight = height - 4;
  int fillWidth = (innerWidth * value) / 100;
  
  // 绘制填充部分
  if (fillWidth > 0) {
    u8g2->drawBox(absX + 2, absY + 2, fillWidth, innerHeight);
  }
  
  // 绘制文字
  if (bShowText) {
    // 当前字体设置
    u8g2->setFont(textFont);
    u8g2->setFontPosCenter();
    
    char text[8];
    sprintf(text, "%d%%", value);
    
    // 计算文字居中位置
    int textWidth = u8g2->getStrWidth(text);
    int textX = absX + (width - textWidth) / 2;
    int textY = absY + height / 2;
    
    // 先画黑色文字
    u8g2->setDrawColor(1);
    u8g2->drawStr(textX, textY, text);
    
    // 如果有填充区域，在填充区域上画白色文字（通过剪裁实现反色效果）
    if (fillWidth > 0) {
      // 设置剪裁区域为填充部分
      u8g2->setClipWindow(absX + 2, absY + 2, absX + 2 + fillWidth, absY + 2 + innerHeight);
      // 画白色文字（实际是擦除，显示为白色）
      u8g2->setDrawColor(0);
      u8g2->drawStr(textX, textY, text);
      // 恢复绘制颜色和取消剪裁
      u8g2->setDrawColor(1);
      u8g2->setMaxClipWindow();
    }
    
    // 恢复字体位置模式为baseline（默认模式）
    u8g2->setFontPosBaseline();
  }
}
