/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "BatteryManager.h"
#include "IOPin.h"
#include "GUIRender.h"

// 声明外部GUIRender实例
extern GUIRender guiRender;

/* VBAT */
const float battery_min = 3.0; // (V) minimum voltage of battery before shutdown
const float battery_max = 4.2; // (V) maximum voltage of battery

// ADC 采样次数（用于平均滤波）
#define ADC_SAMPLES 20

const float resistanceLow = 100000.0; // (Ω) resistance of R1
const float resistanceHigh = 300000.0;// (Ω) resistance of R2
const float offsetVoltage = 0; // (V) voltage offset of the battery
// Flag which will be set in ISR when conversion is done
volatile bool adc_coversion_done = false;

// Result structure for ADC Continuous reading
adc_continuous_data_t *result = NULL;

// ISR Function that will be triggered when ADC conversion is done
void ARDUINO_ISR_ATTR adcComplete() {
  adc_coversion_done = true;
}

extern String deviceId;
extern bool isGetConfigSuccess;

BatteryManager::BatteryManager():
batteryMonitorTaskHandle(nullptr)
{
}

BatteryManager::~BatteryManager() {
    if (batteryMonitorTaskHandle) {
        vTaskDelete(batteryMonitorTaskHandle);
    }
}

void BatteryManager::init() {
    Serial.println("BatteryManager init");
    pinMode(PIN_BATTERY, INPUT);
    
    analogSetAttenuation(ADC_11db);  // 设置衰减，支持 0-3.3V 输入范围
    analogReadResolution(12);         // 12位分辨率 (0-4095)
    
    // 创建电池监测任务
    xTaskCreate(
        batteryMonitorTask,             // 任务函数
        "BatteryMonitorTask",           // 任务名称
        4096,                           // 任务堆栈大小，增加到4096以避免栈溢出
        this,                           // 任务参数
        1,                              // 任务优先级，设置为较低优先级
        &batteryMonitorTaskHandle       // 任务句柄
    );
    Serial.println("BatteryMonitorTask started");
}

uint32_t BatteryManager::readADC()
{
   uint32_t sum = 0;
    
    // 多次采样取平均值，减少噪声
    for (int i = 0; i < ADC_SAMPLES; i++) {
        sum += analogReadMilliVolts(PIN_BATTERY);
        vTaskDelay(pdMS_TO_TICKS(1));  // 短暂延迟，让 ADC 稳定
    }
    
    uint32_t avgMilliVolts = sum / ADC_SAMPLES;
    return avgMilliVolts;  // 单位(mV)
}

float BatteryManager::readVoltage() {
    /*通过R1&R2推算电池电压*/
    float battery_voltage = (resistanceHigh + resistanceLow) / resistanceLow * readADC() + offsetVoltage;
    Serial.printf("Battery voltage: %f mV\n", battery_voltage);

    return battery_voltage / 1000.0;
}

float BatteryManager::calculateBatteryPercent(float voltage) {
    float percent = (voltage - battery_min) / (battery_max - battery_min) * 100.0;
    if (percent < 0)
        percent = 0;
    else if (percent > 100.0)
        percent = 100.0;
    
    return percent;
}

// 电池监测任务函数
void BatteryManager::batteryMonitorTask(void* pvParameters)
{
    BatteryManager* batteryManager = static_cast<BatteryManager*>(pvParameters);
    
    while (true) {
        // 读取电池电压
        float voltage = batteryManager->readVoltage();
        
        // 更新到GUI显示
        guiRender.setBattery(voltage);
        
        // 每10秒执行一次
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}