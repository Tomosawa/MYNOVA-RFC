/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// UINavBar.h
#ifndef UINavBar_h
#define UINavBar_h

#include "UIWidget.h"
#include <Arduino.h>
#include <functional>

class UINavBar : public UIWidget {
public:
    UINavBar(int x, int y, int width, int height);
    
    void render(U8G2* u8g2, int offsetX = 0, int offsetY = 0) override;
    
    // 设置按钮提示文字
    void setLeftButtonText(String text);
    void setMiddleButtonText(String text);
    void setRightButtonText(String text);
    
    // 设置字体
    void setFont(const uint8_t* font);
    
    // 设置边距
    void setMargin(int left, int right);
    
    // 显示/隐藏边框
    void setShowBorder(bool show);
    
    // 高亮动画方法
    void showLeftHighLight(std::function<void()> onComplete = nullptr);
    void showMiddleHighLight(std::function<void()> onComplete = nullptr);
    void showRightHighLight(std::function<void()> onComplete = nullptr);
    
    void showLeftBlink(int blinkCount = 3, int onDuration = 100, int offDuration = 100, std::function<void()> onComplete = nullptr);
    void showMiddleBlink(int blinkCount = 3, int onDuration = 100, int offDuration = 100, std::function<void()> onComplete = nullptr);
    void showRightBlink(int blinkCount = 3, int onDuration = 100, int offDuration = 100, std::function<void()> onComplete = nullptr);

public:
    String leftButtonText;
    String middleButtonText;
    String rightButtonText;
    const uint8_t* buttonFont = u8g2_font_wqy12_t_gb2312;
    bool bShowBorder;
    int marginLeft;
    int marginRight;
    
    // 高亮状态 (public 以便动画类访问)
    bool bLeftHighLight = false;
    bool bMiddleHighLight = false;
    bool bRightHighLight = false;

private:
    int lineHeight;
};

#endif