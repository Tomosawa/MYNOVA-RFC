/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// UIIcon.h
#ifndef UIIcon_h
#define UIIcon_h

#include "UIWidget.h"
#include <Arduino.h>
#include <U8g2lib.h>

/**
 * UIIcon - 通用图标组件
 * 用于显示XBM格式的位图图标
 */
class UIIcon : public UIWidget {
public:
    UIIcon();
    void render(U8G2* u8g2, int offsetX = 0, int offsetY = 0) override;
    
    /**
     * 设置图标位图数据
     * @param bitmap XBM位图数据数组
     * @param w 图标宽度（像素）
     * @param h 图标高度（像素）
     */
    void setIcon(const unsigned char* bitmap, int w, int h);
    
public:
    const unsigned char* iconBitmap; // XBM位图数据
    int iconWidth;  // 图标宽度
    int iconHeight; // 图标高度
    bool bCentered; // 是否居中显示
};

#endif

