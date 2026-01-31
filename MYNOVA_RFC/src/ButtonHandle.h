/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef __BUTTONHANDLE_H_
#define __BUTTONHANDLE_H_
#pragma once
#include <Arduino.h>
#include "ButtonDetector.h"


// 按钮事件枚举
enum ButtonHandleEvent {
    BTN_BACK,
    BTN_MENU,
    BTN_ENTER,
    BTN_1,
    BTN_2,
    BTN_3,
    BTN_4,
    BTN_5,
    BTN_6,
    BTN_7,
    BTN_8,
    BTN_9
};

class ButtonHandle
{
public:
    ButtonHandle();
    ~ButtonHandle();
    void init();
    
private:
    // 按钮中断回调函数（无需参数）
    static void Click_Handle_BtnBack();
    static void Click_Handle_BtnMenu();
    static void Click_Handle_BtnEnter();
    static void Click_Handle_Btn1();
    static void Click_Handle_Btn2();
    static void Click_Handle_Btn3();
    static void Click_Handle_Btn4();
    static void Click_Handle_Btn5();
    static void Click_Handle_Btn6();
    static void Click_Handle_Btn7();
    static void Click_Handle_Btn8();
    static void Click_Handle_Btn9();
 
private:
    // 按钮检测器实例
    ButtonDetector* pBtnBack;
    ButtonDetector* pBtnMenu;
    ButtonDetector* pBtnEnter;
    ButtonDetector* pBtn1;
    ButtonDetector* pBtn2;
    ButtonDetector* pBtn3;
    ButtonDetector* pBtn4;
    ButtonDetector* pBtn5;
    ButtonDetector* pBtn6;
    ButtonDetector* pBtn7;
    ButtonDetector* pBtn8;
    ButtonDetector* pBtn9;
    
    // 按钮事件队列
    QueueHandle_t buttonEventQueue;
    
    // FreeRTOS任务句柄（只需要执行任务）
    static TaskHandle_t buttonExecuteTaskHandle;
    
    // 按钮事件处理任务函数
    static void buttonExecuteTask(void* pvParameters);
};

#endif
