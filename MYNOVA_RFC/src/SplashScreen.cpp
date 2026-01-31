/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// SplashScreen.cpp
#include "SplashScreen.h"

// Logo文字
static const char* LOGO_TEXT = "MYNOVA";
static const int LOGO_CHAR_COUNT = 6;

// 屏幕尺寸
static const int SCREEN_W = 128;
static const int SCREEN_H = 64;

SplashScreen::SplashScreen() {
    pU8g2 = nullptr;
    currentProgress = 0;
    currentStatus = "Starting...";
    animStartTime = 0;
    animFrame = 0;
}

void SplashScreen::init(U8G2* u8g2) {
    pU8g2 = u8g2;
    animStartTime = millis();
    animFrame = 0;
}

void SplashScreen::setProgress(int progress, const char* statusText) {
    currentProgress = progress;
    if (statusText) {
        currentStatus = statusText;
    }
    // 每次更新进度时渲染一帧
    render();
}

int SplashScreen::getWaveOffset(int charIndex, int frame) {
    // 正弦波浪效果，每个字符相位不同
    float phase = (float)charIndex * 0.8f + (float)frame * 0.15f;
    return (int)(sin(phase) * 4.0f);
}

void SplashScreen::drawLogo(int centerY) {
    if (!pU8g2) return;
    
    // 使用大号字体绘制Logo
    pU8g2->setFont(u8g2_font_helvB14_tr);  // 粗体14号字体
    
    // 计算Logo总宽度
    int logoWidth = pU8g2->getStrWidth(LOGO_TEXT);
    int startX = (SCREEN_W - logoWidth) / 2;
    
    // 计算当前动画帧
    unsigned long elapsed = millis() - animStartTime;
    animFrame = elapsed / 50;  // 50ms一帧
    
    // 绘制阴影（伪3D效果）
    pU8g2->setDrawColor(1);
    int charX = startX;
    for (int i = 0; i < LOGO_CHAR_COUNT; i++) {
        char c[2] = {LOGO_TEXT[i], '\0'};
        int waveY = getWaveOffset(i, animFrame);
        
        // 阴影（右下偏移2像素）
        pU8g2->drawStr(charX + 2, centerY + waveY + 2, c);
        charX += pU8g2->getStrWidth(c);
    }
    
    // 清除阴影区域的主文字部分（创建镂空效果）
    pU8g2->setDrawColor(0);
    charX = startX;
    for (int i = 0; i < LOGO_CHAR_COUNT; i++) {
        char c[2] = {LOGO_TEXT[i], '\0'};
        int waveY = getWaveOffset(i, animFrame);
        pU8g2->drawStr(charX, centerY + waveY, c);
        charX += pU8g2->getStrWidth(c);
    }
    
    // 绘制主文字
    pU8g2->setDrawColor(1);
    charX = startX;
    for (int i = 0; i < LOGO_CHAR_COUNT; i++) {
        char c[2] = {LOGO_TEXT[i], '\0'};
        int waveY = getWaveOffset(i, animFrame);
        pU8g2->drawStr(charX, centerY + waveY, c);
        charX += pU8g2->getStrWidth(c);
    }
    
    // 绘制副标题
    pU8g2->setFont(u8g2_font_5x7_tr);
    const char* subtitle = "RF Controller";
    int subWidth = pU8g2->getStrWidth(subtitle);
    pU8g2->drawStr((SCREEN_W - subWidth) / 2, centerY + 12, subtitle);
}

void SplashScreen::drawProgressBar(int y, int progress) {
    if (!pU8g2) return;
    
    // 进度条参数
    const int barX = 14;
    const int barWidth = 100;
    const int barHeight = 8;
    
    // 绘制边框
    pU8g2->setDrawColor(1);
    pU8g2->drawFrame(barX, y, barWidth, barHeight);
    
    // 绘制填充（带动画效果）
    int fillWidth = (barWidth - 4) * progress / 100;
    if (fillWidth > 0) {
        // 主填充
        pU8g2->drawBox(barX + 2, y + 2, fillWidth, barHeight - 4);
        
        // 闪烁效果（进度条头部）
        if (progress < 100 && (animFrame % 4) < 2) {
            int headX = barX + 2 + fillWidth - 2;
            if (headX > barX + 2) {
                pU8g2->setDrawColor(0);
                pU8g2->drawBox(headX, y + 2, 2, barHeight - 4);
                pU8g2->setDrawColor(1);
            }
        }
    }
    
    // 绘制百分比
    char percentText[8];
    sprintf(percentText, "%d%%", progress);
    pU8g2->setFont(u8g2_font_5x7_tr);
    int textWidth = pU8g2->getStrWidth(percentText);
    pU8g2->drawStr(barX + barWidth + 4, y + 7, percentText);
}

void SplashScreen::drawStatus(int y) {
    if (!pU8g2 || !currentStatus) return;
    
    pU8g2->setFont(u8g2_font_5x7_tr);
    
    // 计算文字宽度并居中
    int textWidth = pU8g2->getStrWidth(currentStatus);
    int x = (SCREEN_W - textWidth) / 2;
    
    // 加载动画点点点
    char dots[4] = {'.', '.', '.', '\0'};
    int dotCount = (animFrame / 4) % 4;
    dots[dotCount] = '\0';
    
    // 绘制状态文字
    pU8g2->drawStr(x, y, currentStatus);
    
    // 绘制动画点（如果状态文字不以...结尾）
    if (currentProgress < 100) {
        pU8g2->drawStr(x + textWidth, y, dots);
    }
}

void SplashScreen::render() {
    if (!pU8g2) return;
    
    pU8g2->clearBuffer();
    
    // 绘制顶部装饰线
    pU8g2->setDrawColor(1);
    pU8g2->drawHLine(0, 0, SCREEN_W);
    pU8g2->drawHLine(0, 1, SCREEN_W);
    
    // 绘制Logo（垂直居中偏上）
    drawLogo(28);
    
    // 绘制进度条
    drawProgressBar(44, currentProgress);
    
    // 绘制状态文字
    drawStatus(62);
    
    // 绘制底部装饰线
    pU8g2->drawHLine(0, 63, SCREEN_W);
    
    pU8g2->sendBuffer();
}

void SplashScreen::finish() {
    if (!pU8g2) return;
    
    // 完成动画：Logo放大效果
    for (int i = 0; i < 5; i++) {
        pU8g2->clearBuffer();
        
        // 简单的闪烁完成效果
        if (i % 2 == 0) {
            pU8g2->setFont(u8g2_font_helvB14_tr);
            int logoWidth = pU8g2->getStrWidth(LOGO_TEXT);
            pU8g2->drawStr((SCREEN_W - logoWidth) / 2, 32, LOGO_TEXT);
        }
        
        pU8g2->sendBuffer();
        delay(80);
    }
    
    // 最终淡出（从中心向外扩展清除）
    for (int r = 0; r <= 80; r += 10) {
        pU8g2->clearBuffer();
        
        // 绘制Logo
        pU8g2->setFont(u8g2_font_helvB14_tr);
        int logoWidth = pU8g2->getStrWidth(LOGO_TEXT);
        pU8g2->drawStr((SCREEN_W - logoWidth) / 2, 32, LOGO_TEXT);
        
        // 从中心清除圆形区域（模拟淡出）
        pU8g2->setDrawColor(0);
        pU8g2->drawDisc(SCREEN_W / 2, SCREEN_H / 2, r);
        pU8g2->setDrawColor(1);
        
        pU8g2->sendBuffer();
        delay(30);
    }
}
