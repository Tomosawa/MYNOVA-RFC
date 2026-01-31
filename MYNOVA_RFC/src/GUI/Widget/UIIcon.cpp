/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// UIIcon.cpp
#include "UIIcon.h"
#include <U8g2lib.h>

UIIcon::UIIcon() {
    iconBitmap = nullptr;
    iconWidth = 0;
    iconHeight = 0;
    bCentered = true; // 默认居中显示
}

void UIIcon::setIcon(const unsigned char* bitmap, int w, int h) {
    iconBitmap = bitmap;
    iconWidth = w;
    iconHeight = h;
}

void UIIcon::render(U8G2* u8g2, int offsetX, int offsetY) {
    // 检查是否可见
    if (!bVisible) return;
    
    // 检查是否设置了图标
    if (iconBitmap == nullptr || iconWidth == 0 || iconHeight == 0) return;
    
    int drawX = x + offsetX;
    int drawY = y + offsetY;
    
    // 如果需要居中显示
    if (bCentered) {
        drawX += (width - iconWidth) / 2;
        drawY += (height - iconHeight) / 2;
    }
    
    // 绘制XBM位图
    u8g2->drawXBMP(drawX, drawY, iconWidth, iconHeight, iconBitmap);
}

