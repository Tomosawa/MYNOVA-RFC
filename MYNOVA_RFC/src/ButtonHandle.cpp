/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "ButtonHandle.h"
#include "IOPin.h"
#include "GUI/UIPage.h"
#include "GUI/UIEngine.h"
#include "ButtonDetector.h"
#include "SystemSetting.h"

extern UIEngine uiEngine;
extern SystemSetting systemSetting;

// 全局ButtonHandle实例指针
ButtonHandle* g_buttonHandle = nullptr;
TaskHandle_t ButtonHandle::buttonExecuteTaskHandle = nullptr;

ButtonHandle::ButtonHandle()
{
    g_buttonHandle = this;
    // 创建队列，最多容纳10个按钮事件
    buttonEventQueue = xQueueCreate(10, sizeof(ButtonHandleEvent));
}

void ButtonHandle::init()
{
    // GPIO初始化（配置为输入上拉）
    pinMode(PIN_KEY_BACK, INPUT_PULLUP);
    pinMode(PIN_KEY_MENU, INPUT_PULLUP);
    pinMode(PIN_KEY_ENTER, INPUT_PULLUP);
    pinMode(PIN_KEY_1, INPUT_PULLUP);
    pinMode(PIN_KEY_2, INPUT_PULLUP);
    pinMode(PIN_KEY_3, INPUT_PULLUP);
    pinMode(PIN_KEY_4, INPUT_PULLUP);
    pinMode(PIN_KEY_5, INPUT_PULLUP);
    pinMode(PIN_KEY_6, INPUT_PULLUP);
    pinMode(PIN_KEY_7, INPUT_PULLUP);
    pinMode(PIN_KEY_8, INPUT_PULLUP);
    pinMode(PIN_KEY_9, INPUT_PULLUP);

    // 创建按钮检测器并注册中断
    // active_level = LOW 表示按下时为低电平
    pBtnBack = new ButtonDetector(PIN_KEY_BACK, LOW);
    pBtnBack->attach(ButtonEvent::SINGLE_CLICK, Click_Handle_BtnBack);
    pBtnBack->start();

    pBtnMenu = new ButtonDetector(PIN_KEY_MENU, LOW);
    pBtnMenu->attach(ButtonEvent::SINGLE_CLICK, Click_Handle_BtnMenu);
    pBtnMenu->start();

    pBtnEnter = new ButtonDetector(PIN_KEY_ENTER, LOW);
    pBtnEnter->attach(ButtonEvent::SINGLE_CLICK, Click_Handle_BtnEnter);
    pBtnEnter->start();

    pBtn1 = new ButtonDetector(PIN_KEY_1, LOW);
    pBtn1->attach(ButtonEvent::SINGLE_CLICK, Click_Handle_Btn1);
    pBtn1->attach(ButtonEvent::LONG_PRESS, Click_Handle_Btn1);  // 长按也触发同样的动作
    pBtn1->start();

    pBtn2 = new ButtonDetector(PIN_KEY_2, LOW);
    pBtn2->attach(ButtonEvent::SINGLE_CLICK, Click_Handle_Btn2);
    pBtn2->attach(ButtonEvent::LONG_PRESS, Click_Handle_Btn2);  // 长按也触发同样的动作
    pBtn2->start();

    pBtn3 = new ButtonDetector(PIN_KEY_3, LOW);
    pBtn3->attach(ButtonEvent::SINGLE_CLICK, Click_Handle_Btn3);
    pBtn3->attach(ButtonEvent::LONG_PRESS, Click_Handle_Btn3);
    pBtn3->start();

    pBtn4 = new ButtonDetector(PIN_KEY_4, LOW);
    pBtn4->attach(ButtonEvent::SINGLE_CLICK, Click_Handle_Btn4);
    pBtn4->attach(ButtonEvent::LONG_PRESS, Click_Handle_Btn4);  // 长按也触发同样的动作
    pBtn4->start();

    pBtn5 = new ButtonDetector(PIN_KEY_5, LOW);
    pBtn5->attach(ButtonEvent::SINGLE_CLICK, Click_Handle_Btn5);
    pBtn5->attach(ButtonEvent::LONG_PRESS, Click_Handle_Btn5);
    pBtn5->start();

    pBtn6 = new ButtonDetector(PIN_KEY_6, LOW);
    pBtn6->attach(ButtonEvent::SINGLE_CLICK, Click_Handle_Btn6);
    pBtn6->attach(ButtonEvent::LONG_PRESS, Click_Handle_Btn6);  // 长按也触发同样的动作
    pBtn6->start();

    pBtn7 = new ButtonDetector(PIN_KEY_7, LOW);
    pBtn7->attach(ButtonEvent::SINGLE_CLICK, Click_Handle_Btn7);
    pBtn7->attach(ButtonEvent::LONG_PRESS, Click_Handle_Btn7);  // 长按也触发同样的动作
    pBtn7->start();

    pBtn8 = new ButtonDetector(PIN_KEY_8, LOW);
    pBtn8->attach(ButtonEvent::SINGLE_CLICK, Click_Handle_Btn8);
    pBtn8->attach(ButtonEvent::LONG_PRESS, Click_Handle_Btn8);  // 长按也触发同样的动作
    pBtn8->start();

    pBtn9 = new ButtonDetector(PIN_KEY_9, LOW);
    pBtn9->attach(ButtonEvent::SINGLE_CLICK, Click_Handle_Btn9);
    pBtn9->attach(ButtonEvent::LONG_PRESS, Click_Handle_Btn9);  // 长按也触发同样的动作
    pBtn9->start();

    // 创建按钮执行任务（处理按钮事件队列）
    xTaskCreate(
        buttonExecuteTask,          // 函数名
        "ButtonExecuteTask",        // 任务名
        10240,                       // 任务堆栈大小
        this,                       // 任务参数
        5,                          // 任务优先级
        &buttonExecuteTaskHandle    // 任务句柄
    );

    Serial.println("ButtonHandle init done (GPIO Interrupt Mode)");
}

// ==================== 按钮中断回调函数 ====================
// 这些函数在消抖完成后被调用，将事件放入队列

void ButtonHandle::Click_Handle_BtnBack()
{
    if (g_buttonHandle) {
        ButtonHandleEvent event = BTN_BACK;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(g_buttonHandle->buttonEventQueue, &event, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

void ButtonHandle::Click_Handle_BtnMenu()
{
    if (g_buttonHandle) {
        ButtonHandleEvent event = BTN_MENU;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(g_buttonHandle->buttonEventQueue, &event, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

void ButtonHandle::Click_Handle_BtnEnter()
{
    if (g_buttonHandle) {
        ButtonHandleEvent event = BTN_ENTER;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(g_buttonHandle->buttonEventQueue, &event, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

void ButtonHandle::Click_Handle_Btn1()
{
    if (g_buttonHandle) {
        ButtonHandleEvent event = BTN_1;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(g_buttonHandle->buttonEventQueue, &event, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

void ButtonHandle::Click_Handle_Btn2()
{
    if (g_buttonHandle) {
        ButtonHandleEvent event = BTN_2;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(g_buttonHandle->buttonEventQueue, &event, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

void ButtonHandle::Click_Handle_Btn3()
{
    if (g_buttonHandle) {
        ButtonHandleEvent event = BTN_3;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(g_buttonHandle->buttonEventQueue, &event, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

void ButtonHandle::Click_Handle_Btn4()
{
    if (g_buttonHandle) {
        ButtonHandleEvent event = BTN_4;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(g_buttonHandle->buttonEventQueue, &event, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

void ButtonHandle::Click_Handle_Btn5()
{
    if (g_buttonHandle) {
        ButtonHandleEvent event = BTN_5;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(g_buttonHandle->buttonEventQueue, &event, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

void ButtonHandle::Click_Handle_Btn6()
{
    if (g_buttonHandle) {
        ButtonHandleEvent event = BTN_6;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(g_buttonHandle->buttonEventQueue, &event, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

void ButtonHandle::Click_Handle_Btn7()
{
    if (g_buttonHandle) {
        ButtonHandleEvent event = BTN_7;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(g_buttonHandle->buttonEventQueue, &event, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

void ButtonHandle::Click_Handle_Btn8()
{
    if (g_buttonHandle) {
        ButtonHandleEvent event = BTN_8;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(g_buttonHandle->buttonEventQueue, &event, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

void ButtonHandle::Click_Handle_Btn9()
{
    if (g_buttonHandle) {
        ButtonHandleEvent event = BTN_9;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(g_buttonHandle->buttonEventQueue, &event, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

// ==================== 按钮事件执行任务 ====================
// 从队列中取出事件并分发给对应的页面处理

void ButtonHandle::buttonExecuteTask(void* pvParameters)
{
    ButtonHandleEvent event;
    while(true){
        // 等待队列中接收消息
        if (g_buttonHandle && xQueueReceive(g_buttonHandle->buttonEventQueue, &event, portMAX_DELAY) == pdPASS) {
            // 任何按键按下都重置空闲计时器
            systemSetting.resetIdleTimer();
            
            UIPage* currentPage = uiEngine.getCurrentPage();
            if (currentPage) {
                switch (event) {
                    case BTN_BACK:
                        currentPage->onButtonBack();
                        break;
                    case BTN_MENU:
                        currentPage->onButtonMenu();
                        break;
                    case BTN_ENTER:
                        currentPage->onButtonEnter();
                        break;
                    case BTN_1:
                        currentPage->onButton1();
                        break;
                    case BTN_2:
                        currentPage->onButton2();
                        break;
                    case BTN_3:
                        currentPage->onButton3();
                        break;
                    case BTN_4:
                        currentPage->onButton4();
                        break;
                    case BTN_5:
                        currentPage->onButton5();
                        break;
                    case BTN_6:
                        currentPage->onButton6();
                        break;
                    case BTN_7:
                        currentPage->onButton7();
                        break;
                    case BTN_8:
                        currentPage->onButton8();
                        break;
                    case BTN_9:
                        currentPage->onButton9();
                        break;
                }
            }
        }
    }
}

ButtonHandle::~ButtonHandle()
{
    // 删除按钮检测器（会自动注销中断）
    if(pBtnBack != NULL)
        delete pBtnBack;
    if(pBtnMenu != NULL)
        delete pBtnMenu;
    if(pBtnEnter != NULL)
        delete pBtnEnter;
    if(pBtn1 != NULL)
        delete pBtn1;
    if(pBtn2 != NULL)
        delete pBtn2;
    if(pBtn3 != NULL)
        delete pBtn3;
    if(pBtn4 != NULL)
        delete pBtn4;
    if(pBtn5 != NULL)
        delete pBtn5;
    if(pBtn6 != NULL)
        delete pBtn6;
    if(pBtn7 != NULL)
        delete pBtn7;
    if(pBtn8 != NULL)
        delete pBtn8;
    if(pBtn9 != NULL)
        delete pBtn9;

    // 删除按钮执行任务
    if (buttonExecuteTaskHandle != NULL) {
        vTaskDelete(buttonExecuteTaskHandle);
    }
    
    // 删除队列
    if (buttonEventQueue != NULL) {
        vQueueDelete(buttonEventQueue);
    }
    
    g_buttonHandle = nullptr;
}
