/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "Buzzer.h"
#include "SystemSetting.h"

extern SystemSetting systemSetting;

Buzzer::Buzzer(int pin)
{
    this->pin = pin;
    this->buzzerTaskHandle = nullptr;
    this->buzzerQueue = nullptr;
    this->stopRequested = false;
    this->initialized = false;
}

Buzzer::~Buzzer() {
    // 停止常驻任务
    if (buzzerTaskHandle != nullptr) {
        vTaskDelete(buzzerTaskHandle);
        buzzerTaskHandle = nullptr;
    }
    
    // 删除队列
    if (buzzerQueue != nullptr) {
        vQueueDelete(buzzerQueue);
        buzzerQueue = nullptr;
    }
}

void Buzzer::init() {
    if (initialized) return;
    
    // 初始化GPIO
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    
    // 创建请求队列（最多缓存5个请求）
    buzzerQueue = xQueueCreate(5, sizeof(BuzzerRequest));
    
    // 创建常驻蜂鸣器任务
    xTaskCreate(
        buzzerTask,
        "BuzzerTask",
        2048,           // 常驻任务不需要太大堆栈
        this,           // 传递Buzzer实例
        2,              // 较低优先级
        &buzzerTaskHandle
    );
    
    initialized = true;
    Serial.println("Buzzer: 初始化完成（常驻任务模式）");
}

void Buzzer::beep(int duration)
{
    if (!initialized || !systemSetting.getBuzzerEnable()) return;
    
    BuzzerRequest request;
    request.command = BUZZER_CMD_BEEP;
    request.duration = duration;
    
    // 发送到队列（不等待，队列满则丢弃）
    xQueueSend(buzzerQueue, &request, 0);
}

void Buzzer::beepDouble()
{
    if (!initialized || !systemSetting.getBuzzerEnable()) return;
    
    BuzzerRequest request;
    request.command = BUZZER_CMD_DOUBLE;
    request.duration = 0;
    
    xQueueSend(buzzerQueue, &request, 0);
}

void Buzzer::beepLongShort()
{
    if (!initialized || !systemSetting.getBuzzerEnable()) return;
    
    BuzzerRequest request;
    request.command = BUZZER_CMD_LONG_SHORT;
    request.duration = 0;
    
    xQueueSend(buzzerQueue, &request, 0);
}

void Buzzer::beepSOS()
{
    if (!initialized || !systemSetting.getBuzzerEnable()) return;
    
    BuzzerRequest request;
    request.command = BUZZER_CMD_SOS;
    request.duration = 0;
    
    xQueueSend(buzzerQueue, &request, 0);
}

void Buzzer::stop()
{
    if (!initialized) return;
    
    stopRequested = true;
    
    // 清空队列
    xQueueReset(buzzerQueue);
    
    BuzzerRequest request;
    request.command = BUZZER_CMD_STOP;
    request.duration = 0;
    
    xQueueSend(buzzerQueue, &request, 0);
}

void Buzzer::on()
{
    if (!initialized || !systemSetting.getBuzzerEnable()) return;
    digitalWrite(pin, HIGH);
}

void Buzzer::off()
{
    if (!initialized) return;
    digitalWrite(pin, LOW);
}

// ==================== 常驻蜂鸣器任务 ====================
void Buzzer::buzzerTask(void* pvParameters)
{
    Buzzer* buzzer = static_cast<Buzzer*>(pvParameters);
    BuzzerRequest request;
    
    while (true) {
        // 等待队列中的请求（永久等待）
        if (xQueueReceive(buzzer->buzzerQueue, &request, portMAX_DELAY) == pdPASS) {
            buzzer->stopRequested = false;
            
            switch (request.command) {
                case BUZZER_CMD_BEEP:
                    buzzer->executeBeep(request.duration);
                    break;
                case BUZZER_CMD_DOUBLE:
                    buzzer->executeDouble();
                    break;
                case BUZZER_CMD_LONG_SHORT:
                    buzzer->executeLongShort();
                    break;
                case BUZZER_CMD_SOS:
                    buzzer->executeSOS();
                    break;
                case BUZZER_CMD_STOP:
                    digitalWrite(buzzer->pin, LOW);
                    break;
            }
        }
    }
}

// ==================== 内部执行函数 ====================
void Buzzer::executeBeep(int duration)
{
    digitalWrite(pin, HIGH);
    vTaskDelay(pdMS_TO_TICKS(duration));
    digitalWrite(pin, LOW);
}

void Buzzer::executeDouble()
{
    // 双短声: 滴-滴
    for (int i = 0; i < 2 && !stopRequested; i++) {
        digitalWrite(pin, HIGH);
        vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(pin, LOW);
        if (i < 1) vTaskDelay(pdMS_TO_TICKS(80));
    }
}

void Buzzer::executeLongShort()
{
    // 一长两短: 嘟---滴-滴
    if (stopRequested) return;
    
    // 长声
    digitalWrite(pin, HIGH);
    vTaskDelay(pdMS_TO_TICKS(400));
    digitalWrite(pin, LOW);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 两短声
    for (int i = 0; i < 2 && !stopRequested; i++) {
        digitalWrite(pin, HIGH);
        vTaskDelay(pdMS_TO_TICKS(100));
        digitalWrite(pin, LOW);
        if (i < 1) vTaskDelay(pdMS_TO_TICKS(80));
    }
}

void Buzzer::executeSOS()
{
    // SOS: ... --- ... (短短短-长长长-短短短)
    
    // S (3个短音)
    for (int i = 0; i < 3 && !stopRequested; i++) {
        digitalWrite(pin, HIGH);
        vTaskDelay(pdMS_TO_TICKS(150));
        digitalWrite(pin, LOW);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    if (stopRequested) return;
    vTaskDelay(pdMS_TO_TICKS(200)); // 字母间隔
    
    // O (3个长音)
    for (int i = 0; i < 3 && !stopRequested; i++) {
        digitalWrite(pin, HIGH);
        vTaskDelay(pdMS_TO_TICKS(400));
        digitalWrite(pin, LOW);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    if (stopRequested) return;
    vTaskDelay(pdMS_TO_TICKS(200)); // 字母间隔
    
    // S (3个短音)
    for (int i = 0; i < 3 && !stopRequested; i++) {
        digitalWrite(pin, HIGH);
        vTaskDelay(pdMS_TO_TICKS(150));
        digitalWrite(pin, LOW);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}