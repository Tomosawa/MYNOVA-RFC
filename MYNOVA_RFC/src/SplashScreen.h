/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// SplashScreen.h
#ifndef SplashScreen_h
#define SplashScreen_h

#include <Arduino.h>
#include <U8g2lib.h>

// 启动画面类（独立于UIPage系统，仅用于启动时显示）
class SplashScreen {
public:
    SplashScreen();
    
    // 初始化（需要在GUIRender初始化后调用）
    void init(U8G2* u8g2);
    
    // 更新进度（0-100）
    void setProgress(int progress, const char* statusText = nullptr);
    
    // 渲染一帧（内部处理动画）
    void render();
    
    // 完成动画（Logo放大 + 淡出效果）
    void finish();
    
private:
    U8G2* pU8g2;
    int currentProgress;
    const char* currentStatus;
    unsigned long animStartTime;
    int animFrame;
    
    // Logo动画相关
    void drawLogo(int centerY);
    void drawProgressBar(int y, int progress);
    void drawStatus(int y);
    
    // 波浪效果参数
    int getWaveOffset(int charIndex, int frame);
};

#endif
