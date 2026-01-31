/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// UISelectValue.cpp
#include "UISelectValue.h"
#include <U8g2lib.h>

UISelectValue::UISelectValue() {
    value = "";
    bSelected = false;
    bShowBorder = true;
    paddingH = 3;
    paddingV = 2;
}

void UISelectValue::render(U8G2* u8g2, int offsetX, int offsetY) {
    // 检查是否可见
    if (!bVisible) return;
    
    int absX = x + offsetX;
    int absY = y + offsetY + 1;
    
    u8g2->setFont(textFont);
    
    // 如果被选中，绘制反色高亮背景
    if (bSelected) {
        // 绘制填充矩形（白色背景）
        u8g2->setDrawColor(1);
        u8g2->drawBox(absX, absY, width, height);
        
        // 绘制边框（可选）
        if (bShowBorder) {
            u8g2->drawFrame(absX, absY, width, height);
        }
        
        // 设置为黑色文字（反色）
        u8g2->setDrawColor(0);
    } else {
        // 未选中状态，绘制边框（可选）
        if (bShowBorder) {
            u8g2->setDrawColor(1);
            u8g2->drawFrame(absX, absY, width, height);
        }
        
        // 设置为白色文字（正常）
        u8g2->setDrawColor(1);
    }
    
    // 计算文字居中位置
    uint16_t strWidth = u8g2->getUTF8Width(value.c_str());
    int8_t maxCharHeight = u8g2->getMaxCharHeight();
    
    int textX = absX + (width - strWidth) / 2;
    int textY = absY + (height + maxCharHeight) / 2 - 2;
    
    // 绘制文字
    u8g2->drawUTF8(textX, textY, value.c_str());
    
    // 恢复绘制颜色
    u8g2->setDrawColor(1);
}

