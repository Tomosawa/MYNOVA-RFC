/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef __BUZZER_H__
#define __BUZZER_H__
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

// 蜂鸣器命令类型
enum BuzzerCommand {
    BUZZER_CMD_BEEP,            // 单次蜂鸣
    BUZZER_CMD_DOUBLE,          // 双短声
    BUZZER_CMD_LONG_SHORT,      // 一长两短
    BUZZER_CMD_SOS,             // SOS模式
    BUZZER_CMD_STOP             // 停止
};

// 蜂鸣器请求结构（固定大小，避免动态分配）
struct BuzzerRequest {
    BuzzerCommand command;
    int duration;               // 用于 BUZZER_CMD_BEEP
};

class Buzzer {
public:
    Buzzer(int pin);
    ~Buzzer();
    
    // 初始化（创建常驻任务）
    void init();
    
    // 非阻塞式蜂鸣方法
    void beep(int duration = 100);
    void beepDouble();           // 双短声
    void beepLongShort();        // 一长两短
    void beepSOS();              // SOS模式
    void stop();                 // 停止当前蜂鸣
    
    void on();
    void off();
    
private:
    int pin;
    bool initialized;
    
    // FreeRTOS相关
    TaskHandle_t buzzerTaskHandle;
    QueueHandle_t buzzerQueue;      // 蜂鸣请求队列
    volatile bool stopRequested;    // 停止标志
    
    // 常驻蜂鸣任务函数
    static void buzzerTask(void* pvParameters);
    
    // 内部方法
    void executeBeep(int duration);
    void executeDouble();
    void executeLongShort();
    void executeSOS();
};

#endif