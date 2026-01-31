/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef _BUTTON_DETECTOR_H_
#define _BUTTON_DETECTOR_H_

#include <Arduino.h>
#include <functional>

// 按钮事件枚举
enum class ButtonEvent {
    PRESS_DOWN = 0,      // 按下
    PRESS_UP,            // 释放
    SINGLE_CLICK,        // 单击（消抖后确认的点击）
    LONG_PRESS,          // 长按（持续触发）
    NONE_PRESS
};

// 回调函数类型
using ButtonDetectorCallback = std::function<void()>;

/**
 * @brief 基于GPIO中断的按钮检测器
 * 
 * 特性：
 * - 使用GPIO中断，CPU占用极低
 * - 软件定时器消抖，可靠性高
 * - 自动管理中断使能/禁用
 * - 支持快速响应模式（用于游戏等需要高频按键的场景）
 */
class ButtonDetector {
private:
    // 时间配置（毫秒）
    static constexpr uint32_t DEBOUNCE_TIME_MS = 20;   // 消抖时间
    static constexpr uint32_t COOLDOWN_TIME_MS_NORMAL = 150;  // 普通模式冷却时间
    static constexpr uint32_t COOLDOWN_TIME_MS_FAST = 30;     // 快速模式冷却时间（用于游戏）
    static constexpr uint32_t LONG_PRESS_INITIAL_DELAY = 300; // 长按初始延迟
    static constexpr uint32_t LONG_PRESS_REPEAT_INTERVAL = 80; // 长按重复间隔
    
    // 全局快速响应模式标志
    static bool fastResponseMode_;
    
    // 长按支持
    static bool longPressEnabled_;
    
    // 按钮状态
    uint8_t button_pin_;           // GPIO引脚号
    uint8_t active_level_;         // 激活电平 (HIGH/LOW)
    volatile bool debouncing_;     // 是否正在消抖
    volatile uint32_t lastTriggerTime_;  // 上次触发时间（毫秒）
    volatile bool isPressed_;      // 当前是否按下
    volatile uint32_t pressStartTime_;   // 按下开始时间
    volatile uint32_t lastLongPressTime_; // 上次长按触发时间
    
    // 回调函数
    ButtonDetectorCallback callback_;  // 单击回调
    ButtonDetectorCallback longPressCallback_;  // 长按回调
    
    // 软件定时器句柄
    TimerHandle_t debounceTimer_;
    TimerHandle_t longPressTimer_;  // 长按检测定时器
    
    // 静态链表管理所有按钮实例
    static ButtonDetector* head_handle_;
    ButtonDetector* next_;
    
    // 查找按钮实例（通过引脚号）
    static ButtonDetector* findByPin(uint8_t pin);
    
    // 定时器回调（静态）
    static void debounceTimerCallback(TimerHandle_t xTimer);
    static void longPressTimerCallback(TimerHandle_t xTimer);
    
    // 实际的消抖处理
    void handleDebounceComplete();
    
    // 长按检测处理
    void handleLongPressCheck();
    
public:
    // 构造函数
    ButtonDetector(uint8_t button_pin, uint8_t active_level = LOW);
    
    // 删除拷贝构造函数和赋值运算符
    ButtonDetector(const ButtonDetector&) = delete;
    ButtonDetector& operator=(const ButtonDetector&) = delete;
    
    // 析构函数
    ~ButtonDetector();
    
    // 附加事件回调
    void attach(ButtonEvent event, ButtonDetectorCallback callback);
    
    // 启动按钮检测（注册中断）
    bool start();
    
    // 停止按钮检测（注销中断）
    void stop();
    
    // 读取按钮电平
    uint8_t readButtonLevel() const;
    
    // GPIO中断处理函数（静态，IRAM_ATTR）
    static void IRAM_ATTR gpioISR(void* arg);
    static void IRAM_ATTR gpioReleaseISR(void* arg);
    
    // 设置快速响应模式（用于游戏等需要高频按键的场景）
    static void setFastResponseMode(bool enabled);
    
    // 获取当前是否为快速响应模式
    static bool isFastResponseMode();
    
    // 设置长按模式（启用后支持按住不放连续触发）
    static void setLongPressEnabled(bool enabled);
    
    // 获取当前是否启用长按模式
    static bool isLongPressEnabled();
};

#endif // _BUTTON_DETECTOR_H_
