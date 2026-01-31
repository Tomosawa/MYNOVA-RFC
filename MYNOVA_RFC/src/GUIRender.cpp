/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "GUIRender.h"
#include "IOPin.h"
#include <U8g2lib.h>
#include "GUI/UIEngine.h"
#include "GUI/UIPage.h"
#include "GUI/Widget/UITitleBar.h"
#include "GUI/Widget/UIQuickButton.h"
#include "GUI/Widget/UIMenu.h"
#include "Pages/HomePage.h"
#include "Pages/MenuPage.h"
#include "Pages/SettingPage.h"
// OLED屏幕配置
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, /* clock=*/PIN_OLED_SCL, /* data=*/PIN_OLED_SDA);

UIEngine uiEngine;
HomePage uiPageHome;

GUIRender::GUIRender()
{
    drawGUITaskHandle = nullptr;
}

// 仅初始化显示屏（用于SplashScreen阶段）
void GUIRender::initDisplay()
{
    Serial.println("GUIRender: 初始化显示屏");
    u8g2.begin();
}

// 获取U8G2指针
U8G2* GUIRender::getU8G2()
{
    return &u8g2;
}

// 完整初始化（启动渲染任务）
void GUIRender::init()
{
    Serial.println("GUIRender: 完整初始化");
    // 如果显示屏还没初始化，先初始化
    if (!u8g2.getU8g2()) {
        u8g2.begin();
    }
    
    uiEngine.setCurrentPage(&uiPageHome);
    startRenderTask();
}

// 启动渲染任务
void GUIRender::startRenderTask()
{
    if (drawGUITaskHandle != nullptr) {
        Serial.println("GUIRender: 渲染任务已存在");
        return;
    }
    
    xTaskCreate(
        drawGUITask,                // 任务函数
        "DrawGUITask",              // 任务名称
        8192,                       // 任务堆栈大小（8KB）
        this,                       // 任务参数
        3,                          // 任务优先级
        &drawGUITaskHandle          // 任务句柄
    );

    Serial.println("GUIRender: 渲染任务已启动");
}

// 绘制界面
void GUIRender::drawGUITask(void* pvParameters)
{
    while (true)
    {
        u8g2.clearBuffer(); // 清除内部缓冲区
    
        uiEngine.render(&u8g2);

        u8g2.sendBuffer(); // transfer internal memory to the display

        uiEngine.update();

        // 减少延迟以提高界面响应速度
        delay(10);
    }
}


void GUIRender::setBattery(float voltage)
{
    uiPageHome.setBattery(voltage);
}

void GUIRender::setPowerSave(bool bPowerSave)
{
    u8g2.setPowerSave(bPowerSave);
}

void GUIRender::setContrast(int contrast)
{
    u8g2.setContrast(contrast);
}

GUIRender::~GUIRender()
{
    if (drawGUITaskHandle)
    {
        vTaskDelete(drawGUITaskHandle);
    }
}