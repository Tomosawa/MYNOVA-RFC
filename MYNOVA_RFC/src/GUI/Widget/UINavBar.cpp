/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// UINavBar.cpp
#include "UINavBar.h"
#include "src/GUI/Animation/NavBarAnimation.h"
#include "src/GUI/Animation/AnimationEngine.h"

UINavBar::UINavBar(int x, int y, int width, int height) {
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->marginLeft = 3;
    this->marginRight = 3;
    this->lineHeight = height; // 默认行高为整个控件高度
    this->bShowBorder = false;
    this->bLeftHighLight = false;
    this->bMiddleHighLight = false;
    this->bRightHighLight = false;
}

void UINavBar::render(U8G2* u8g2, int offsetX, int offsetY) {
    // 检查是否可见
    if (!bVisible) return;
    
    // 绘制边框
    if(bShowBorder) {
        u8g2->drawFrame(offsetX + x, offsetY + y, width, height);
    }
    
    // 绘制按钮提示
    u8g2->setFont(buttonFont);
    int fontHeight = u8g2->getMaxCharHeight();
    int fontY = y + height - (lineHeight / 2 - fontHeight / 2);
    if(fontY >= y + height)
        fontY = y + height - 2;
    
    // 绘制左键文字及高亮效果
    if(leftButtonText.length() > 0) {
        uint16_t strWidth = u8g2->getUTF8Width(leftButtonText.c_str());
        int textX = offsetX + x + marginLeft;
        int textY = offsetY + fontY;
        
        if(bLeftHighLight) {
            // 高亮模式：绘制填充矩形背景
            u8g2->setDrawColor(1);
            u8g2->drawRBox(textX - 2, offsetY + y + 1, strWidth + 4, height - 2, 2);
            // 反色绘制文字
            u8g2->setFontMode(1);
            u8g2->setDrawColor(2);
            u8g2->drawUTF8(textX, textY, leftButtonText.c_str());
            u8g2->setFontMode(0);
            u8g2->setDrawColor(1);
        } else {
            // 正常模式
            u8g2->drawUTF8(textX, textY, leftButtonText.c_str());
        }
    }
    
    // 绘制中键文字及高亮效果
    if(middleButtonText.length() > 0) {
        uint16_t strWidth = u8g2->getUTF8Width(middleButtonText.c_str());
        int textX = offsetX + x + (width / 2 - strWidth / 2);
        int textY = offsetY + fontY;
        
        if(bMiddleHighLight) {
            // 高亮模式：绘制填充矩形背景
            u8g2->setDrawColor(1);
            u8g2->drawRBox(textX - 2, offsetY + y + 1, strWidth + 4, height - 2, 2);
            // 反色绘制文字
            u8g2->setFontMode(1);
            u8g2->setDrawColor(2);
            u8g2->drawUTF8(textX, textY, middleButtonText.c_str());
            u8g2->setFontMode(0);
            u8g2->setDrawColor(1);
        } else {
            // 正常模式
            u8g2->drawUTF8(textX, textY, middleButtonText.c_str());
        }
    }
    
    // 绘制右键文字及高亮效果
    if(rightButtonText.length() > 0) {
        uint16_t strWidth = u8g2->getUTF8Width(rightButtonText.c_str());
        int textX = offsetX + x + width - marginRight - strWidth;
        int textY = offsetY + fontY;
        
        if(bRightHighLight) {
            // 高亮模式：绘制填充矩形背景
            u8g2->setDrawColor(1);
            u8g2->drawRBox(textX - 2, offsetY + y + 1, strWidth + 4, height - 2, 2);
            // 反色绘制文字
            u8g2->setFontMode(1);
            u8g2->setDrawColor(2);
            u8g2->drawUTF8(textX, textY, rightButtonText.c_str());
            u8g2->setFontMode(0);
            u8g2->setDrawColor(1);
        } else {
            // 正常模式
            u8g2->drawUTF8(textX, textY, rightButtonText.c_str());
        }
    }
}

void UINavBar::setLeftButtonText(String text) {
    leftButtonText = text;
}

void UINavBar::setMiddleButtonText(String text) {
    middleButtonText = text;
}

void UINavBar::setRightButtonText(String text) {
    rightButtonText = text;
}

void UINavBar::setFont(const uint8_t* font) {
    buttonFont = font;
}

void UINavBar::setMargin(int left, int right) {
    marginLeft = left;
    marginRight = right;
}

void UINavBar::setShowBorder(bool show) {
    bShowBorder = show;
}

// 左键持续高亮
void UINavBar::showLeftHighLight(std::function<void()> onComplete) {
    bLeftHighLight = true;
    
    // 创建高亮动画并添加到动画引擎
    NavBarAnimation* highlightAnim = new NavBarAnimation(this, NAV_BUTTON_LEFT, 300);
    if (onComplete) {
        highlightAnim->setOnComplete(onComplete);
    }
    animationEngine.addAnimation(highlightAnim);
}

// 中键持续高亮
void UINavBar::showMiddleHighLight(std::function<void()> onComplete) {
    bMiddleHighLight = true;
    
    // 创建高亮动画并添加到动画引擎
    NavBarAnimation* highlightAnim = new NavBarAnimation(this, NAV_BUTTON_MIDDLE, 300);
    if (onComplete) {
        highlightAnim->setOnComplete(onComplete);
    }
    animationEngine.addAnimation(highlightAnim);
}

// 右键持续高亮
void UINavBar::showRightHighLight(std::function<void()> onComplete) {
    bRightHighLight = true;
    
    // 创建高亮动画并添加到动画引擎
    NavBarAnimation* highlightAnim = new NavBarAnimation(this, NAV_BUTTON_RIGHT, 300);
    if (onComplete) {
        highlightAnim->setOnComplete(onComplete);
    }
    animationEngine.addAnimation(highlightAnim);
}

// 左键闪烁
void UINavBar::showLeftBlink(int blinkCount, int onDuration, int offDuration, std::function<void()> onComplete) {
    bLeftHighLight = true;
    
    // 创建闪烁动画并添加到动画引擎
    NavBarAnimation* blinkAnim = new NavBarAnimation(this, NAV_BUTTON_LEFT, (onDuration + offDuration) * blinkCount);
    blinkAnim->setBlinkMode(blinkCount, onDuration, offDuration);
    if (onComplete) {
        blinkAnim->setOnComplete(onComplete);
    }
    animationEngine.addAnimation(blinkAnim);
}

// 中键闪烁
void UINavBar::showMiddleBlink(int blinkCount, int onDuration, int offDuration, std::function<void()> onComplete) {
    bMiddleHighLight = true;
    
    // 创建闪烁动画并添加到动画引擎
    NavBarAnimation* blinkAnim = new NavBarAnimation(this, NAV_BUTTON_MIDDLE, (onDuration + offDuration) * blinkCount);
    blinkAnim->setBlinkMode(blinkCount, onDuration, offDuration);
    if (onComplete) {
        blinkAnim->setOnComplete(onComplete);
    }
    animationEngine.addAnimation(blinkAnim);
}

// 右键闪烁
void UINavBar::showRightBlink(int blinkCount, int onDuration, int offDuration, std::function<void()> onComplete) {
    bRightHighLight = true;
    
    // 创建闪烁动画并添加到动画引擎
    NavBarAnimation* blinkAnim = new NavBarAnimation(this, NAV_BUTTON_RIGHT, (onDuration + offDuration) * blinkCount);
    blinkAnim->setBlinkMode(blinkCount, onDuration, offDuration);
    if (onComplete) {
        blinkAnim->setOnComplete(onComplete);
    }
    animationEngine.addAnimation(blinkAnim);
}