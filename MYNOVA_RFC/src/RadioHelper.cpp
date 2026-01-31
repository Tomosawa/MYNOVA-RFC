/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "RadioHelper.h"
#include "IOPin.h"
#include "Buzzer.h"
#include "Lib/RCSwitchA.h"
#include "Lib/RCSwitchB.h"
#include "SystemSetting.h"
#include "driver/gpio.h"

RCSwitchA radioA = RCSwitchA();
RCSwitchB radioB = RCSwitchB();
Buzzer buzzer(PIN_BUZZER);
extern SystemSetting systemSetting;

// 标记GPIO ISR服务是否已安装
static bool gpioIsrServiceInstalled = false;

// 用于保护接收状态的互斥锁
static SemaphoreHandle_t receiveMutex = nullptr;

RadioHelper::RadioHelper(): 
bReciveMode(false),
receiveStartSemaphore(nullptr),
radioReceiveTaskHandle(nullptr)
{
    pinMode(PIN_RX_315, INPUT);
    pinMode(PIN_RX_433, INPUT);
}

void RadioHelper::init()
{ 
    Serial.println("RadioHelper::init");
    
    // 初始化蜂鸣器（常驻任务模式）
    buzzer.init();
    
    // 创建互斥锁
    if (receiveMutex == nullptr) {
        receiveMutex = xSemaphoreCreateMutex();
    }
    
    // 从SystemSetting读取重复发送次数配置
    int repeatTransmit = systemSetting.getRepeatTransmit();
    if (repeatTransmit > 0) {
        SetRepeatTransmit(repeatTransmit);
        Serial.print("RadioHelper: 从配置读取重复发送次数: ");
        Serial.println(repeatTransmit);
    } else {
        // 如果配置为0或未设置，使用默认值15
        SetRepeatTransmit(15);
        Serial.println("RadioHelper: 使用默认重复发送次数: 15");
    }
    
    // 创建信号量，用于触发接收任务开始接收
    receiveStartSemaphore = xSemaphoreCreateBinary();
    
    // 创建接收任务（堆栈大小增加以防止同时处理两个中断时栈溢出）
    xTaskCreate(
        radioReceiveTask,           // 任务函数
        "RadioReceiveTask",         // 任务名称
        4096,                       // 任务堆栈大小（增加到4KB）
        this,                       // 任务参数
        2,                          // 任务优先级
        &radioReceiveTaskHandle     // 任务句柄
    );
}

void RadioHelper::EnableRecive()
{
    Serial.println("EnableRecive315&433");
    
    // 获取互斥锁，确保线程安全
    if (receiveMutex != nullptr) {
        xSemaphoreTake(receiveMutex, portMAX_DELAY);
    }
    
    // 如果已经在接收模式，先完全禁用
    if (bReciveMode) {
        radioA.disableReceive();
        radioB.disableReceive();
        bReciveMode = false;
        // 给一点时间让中断完全停止
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    // 清空之前的数据
    memset(&rcData, 0, sizeof(rcData));
    radioA.resetAvailable();
    radioB.resetAvailable();
   
    // 启用接收
    radioA.enableReceive(PIN_RX_315);
    radioB.enableReceive(PIN_RX_433);

    bReciveMode = true;
    
    // 释放互斥锁
    if (receiveMutex != nullptr) {
        xSemaphoreGive(receiveMutex);
    }
    
    // 发送信号量，通知接收任务开始工作
    // 使用 xSemaphoreGive 前先尝试清空信号量，避免累积
    if (receiveStartSemaphore != nullptr) {
        xSemaphoreTake(receiveStartSemaphore, 0); // 非阻塞清空
        xSemaphoreGive(receiveStartSemaphore);
    }
}

void RadioHelper::DisableRecive()
{
    Serial.println("DisableRecive");
    
    // 获取互斥锁，确保线程安全
    if (receiveMutex != nullptr) {
        xSemaphoreTake(receiveMutex, portMAX_DELAY);
    }
    
    if (bReciveMode) {
        bReciveMode = false;
        radioA.disableReceive();
        radioB.disableReceive();
    }
    
    // 释放互斥锁
    if (receiveMutex != nullptr) {
        xSemaphoreGive(receiveMutex);
    }
}

void RadioHelper::SetRepeatTransmit(int nRepeatTransmit)
{
    radioA.setRepeatTransmit(nRepeatTransmit);
    radioB.setRepeatTransmit(nRepeatTransmit);
}

void RadioHelper::SendData(RCData data)
{
    Serial.println("SendData");
    
    // 先禁用接收模式，避免与发送冲突
    bool wasReceiving = bReciveMode;
    if (wasReceiving) {
        DisableRecive();
    }

    if(data.freqType == FREQ_315){
        Serial.println("enableTransmit315");
        radioA.enableTransmit(PIN_TX_315);
        radioA.setProtocol(data.protocal, data.pulseLength);
        Serial.println("send315");
        radioA.send(data.data, data.bitLength);
        radioA.disableTransmit();  // 发送完成后禁用发送器
    }
    else{
        Serial.println("enableTransmit433");
        radioB.enableTransmit(PIN_TX_433);
        radioB.setProtocol(data.protocal, data.pulseLength);
        Serial.println("send433");
        radioB.send(data.data, data.bitLength);
        radioB.disableTransmit();  // 发送完成后禁用发送器
    }
    
    Serial.println("SendData complete");

    // 使用非阻塞方式启动蜂鸣器，避免阻塞按键任务
    buzzer.beep(100);
}

// 接收任务函数
void RadioHelper::radioReceiveTask(void* pvParameters)
{
    RadioHelper* radioHelper = static_cast<RadioHelper*>(pvParameters);
    
    while (true) {
        // 等待信号量，表示需要开始接收
        if (xSemaphoreTake(radioHelper->receiveStartSemaphore, portMAX_DELAY) == pdTRUE) {
            // 循环等待接收数据
            while (radioHelper->bReciveMode) {
                // 使用互斥锁保护对 radioA/radioB 的访问
                bool dataReceived = false;
                
                if (receiveMutex != nullptr) {
                    // 使用短超时，避免长时间阻塞
                    if (xSemaphoreTake(receiveMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
                        // 再次检查接收模式，因为在等待锁时可能已被禁用
                        if (!radioHelper->bReciveMode) {
                            xSemaphoreGive(receiveMutex);
                            break;
                        }
                        
                        // 在任务中执行解码（而不是在ISR中）
                        radioA.tryDecode();
                        radioB.tryDecode();
                        
                        if (radioA.available()) {
                            Serial.print("Received 315");
                            radioHelper->rcData.freqType = FREQ_315;
                            Serial.print( radioHelper->rcData.data = radioA.getReceivedValue() );
                            Serial.print(" / ");
                            Serial.print( radioHelper->rcData.bitLength = radioA.getReceivedBitlength() );
                            Serial.print("bit ");
                            Serial.print("Protocol: ");
                            Serial.println( radioHelper->rcData.protocal = radioA.getReceivedProtocol() );
                            Serial.print("ReceivedDelay:");
                            Serial.println(radioHelper->rcData.pulseLength = radioA.getReceivedDelay());
                            radioA.resetAvailable();
                            
                            // 停止接收
                            radioHelper->bReciveMode = false;
                            radioA.disableReceive();
                            radioB.disableReceive();
                            dataReceived = true;
                        }
                        else if (radioB.available()) {
                            Serial.print("Received 433");
                            radioHelper->rcData.freqType = FREQ_433;
                            Serial.print( radioHelper->rcData.data = radioB.getReceivedValue() );
                            Serial.print(" / ");
                            Serial.print( radioHelper->rcData.bitLength = radioB.getReceivedBitlength() );
                            Serial.print("bit ");
                            Serial.print("Protocol: ");
                            Serial.println( radioHelper->rcData.protocal = radioB.getReceivedProtocol() );
                            Serial.print("ReceivedDelay:");
                            Serial.println(radioHelper->rcData.pulseLength = radioB.getReceivedDelay());
                            radioB.resetAvailable();
                            
                            // 停止接收
                            radioHelper->bReciveMode = false;
                            radioA.disableReceive();
                            radioB.disableReceive();
                            dataReceived = true;
                        }
                        
                        xSemaphoreGive(receiveMutex);
                    }
                }
                
                if (dataReceived) {
                    // 蜂鸣器（在互斥锁外调用）
                    buzzer.beep(500);
                    break; // 跳出循环，等待下一个信号量
                }
                
                // 短暂延迟以避免过度占用CPU
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
    }
}