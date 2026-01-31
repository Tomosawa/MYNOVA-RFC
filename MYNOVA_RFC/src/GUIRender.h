/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef __GUIRENDER_H_
#define __GUIRENDER_H_
#include <Arduino.h>
#include <U8g2lib.h>
#include "GUI/UIPage.h"
//数据最大数量
#define DATA_LINE_MAX       100

#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT       64

class GUIRender
{
public:
    GUIRender();
    ~GUIRender();
    void init();
    void initDisplay();  // 仅初始化显示屏（用于SplashScreen）
    void startRenderTask();  // 启动渲染任务
    U8G2* getU8G2();  // 获取U8G2指针
    void setBattery(float voltage);
    void setPowerSave(bool bPowerSave);
    void setContrast(int contrast);
    TaskHandle_t drawGUITaskHandle;
private:
    static void drawGUITask(void* pvParameters);

};

#endif