/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef __RADIOHELPER_H__
#define __RADIOHELPER_H__
#include <Arduino.h>

enum FreqType{
    FREQ_315 = 0,
    FREQ_433 = 1
};

struct RCData{
    unsigned long data;//数据
    unsigned int bitLength;//位长度
    unsigned int protocal;//接收协议
    uint16_t pulseLength;//脉冲宽度
    FreqType freqType;
};

class RadioHelper
{
public:
    RadioHelper();
    void init();
    void EnableRecive();
    void DisableRecive();
    void SetRepeatTransmit(int nRepeatTransmit);
    void SendData(RCData data);
    
public:
    RCData rcData;
    
private:
    bool bReciveMode;
    
    // FreeRTOS相关成员
    SemaphoreHandle_t receiveStartSemaphore;  // 接收开始信号量
    TaskHandle_t radioReceiveTaskHandle;      // 接收任务句柄
    
    // 接收任务函数
    static void radioReceiveTask(void* pvParameters);
};

#endif