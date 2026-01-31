/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// UISelectValue.h
#ifndef UISelectValue_h
#define UISelectValue_h

#include "UIWidget.h"
#include <Arduino.h>
#include <U8g2lib.h>

/**
 * UISelectValue - 可选择数值组件
 * 用于显示可修改的数值，支持选中高亮显示
 */
class UISelectValue : public UIWidget {
public:
    UISelectValue();
    void render(U8G2* u8g2, int offsetX = 0, int offsetY = 0) override;
    
public:
    String value;                                    // 显示的值
    const uint8_t* textFont = u8g2_font_wqy12_t_gb2312; // 字体
    bool bSelected;                                   // 是否被选中
    bool bShowBorder;                                 // 是否显示边框
    int paddingH;                                     // 水平内边距
    int paddingV;                                     // 垂直内边距
};

#endif

