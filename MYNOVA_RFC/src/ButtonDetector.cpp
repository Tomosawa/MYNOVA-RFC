/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "ButtonDetector.h"

// 初始化静态成员
ButtonDetector* ButtonDetector::head_handle_ = nullptr;
bool ButtonDetector::fastResponseMode_ = false;
bool ButtonDetector::longPressEnabled_ = false;

/**
 * @brief 构造函数
 * @param button_pin 按钮连接的引脚
 * @param active_level 按钮激活时的电平 (HIGH 或 LOW)
 */
ButtonDetector::ButtonDetector(uint8_t button_pin, uint8_t active_level)
    : button_pin_(button_pin)
    , active_level_(active_level)
    , debouncing_(false)
    , lastTriggerTime_(0)
    , isPressed_(false)
    , pressStartTime_(0)
    , lastLongPressTime_(0)
    , callback_(nullptr)
    , longPressCallback_(nullptr)
    , debounceTimer_(nullptr)
    , longPressTimer_(nullptr)
    , next_(nullptr)
{
    // 创建软件定时器用于消抖
    // 注意：定时器创建为一次性触发（pdFALSE），不自动重载
    debounceTimer_ = xTimerCreate(
        "BtnDebounce",                    // 定时器名称
        pdMS_TO_TICKS(DEBOUNCE_TIME_MS), // 周期
        pdFALSE,                          // 不自动重载
        this,                             // 定时器ID（传递this指针）
        debounceTimerCallback             // 回调函数
    );
    
    // 创建长按检测定时器（周期性触发）
    longPressTimer_ = xTimerCreate(
        "BtnLongPress",                   // 定时器名称
        pdMS_TO_TICKS(50),                // 检测周期50ms
        pdTRUE,                           // 自动重载
        this,                             // 定时器ID
        longPressTimerCallback            // 回调函数
    );
}

/**
 * @brief 析构函数
 */
ButtonDetector::~ButtonDetector() {
    stop();
    
    // 删除定时器
    if (debounceTimer_ != nullptr) {
        xTimerDelete(debounceTimer_, portMAX_DELAY);
        debounceTimer_ = nullptr;
    }
}

/**
 * @brief 附加事件回调函数
 * @param event 事件类型
 * @param callback 回调函数
 */
void ButtonDetector::attach(ButtonEvent event, ButtonDetectorCallback callback) {
    if (event == ButtonEvent::SINGLE_CLICK) {
        callback_ = callback;
    } else if (event == ButtonEvent::LONG_PRESS) {
        longPressCallback_ = callback;
    }
}

/**
 * @brief 启动按钮检测
 * @return true表示成功，false表示已存在
 */
bool ButtonDetector::start() {
    // 检查是否已存在
    ButtonDetector* target = head_handle_;
    while (target) {
        if (target == this) return false;
        target = target->next_;
    }
    
    // 添加到链表
    next_ = head_handle_;
    head_handle_ = this;
    
    // 配置GPIO为输入（已在ButtonHandle::init()中配置）
    // pinMode(button_pin_, INPUT_PULLUP);
    
    // 注册GPIO中断 - 下降沿和上升沿都触发
    attachInterruptArg(
        digitalPinToInterrupt(button_pin_),
        gpioISR,
        (void*)((uint32_t)button_pin_),  // 传递引脚号
        CHANGE  // 电平变化触发（支持按下和释放检测）
    );
    
    return true;
}

/**
 * @brief 停止按钮检测
 */
void ButtonDetector::stop() {
    // 注销中断
    detachInterrupt(digitalPinToInterrupt(button_pin_));
    
    // 停止定时器
    if (debounceTimer_ != nullptr) {
        xTimerStop(debounceTimer_, 0);
    }
    
    // 从链表中移除
    ButtonDetector** curr;
    for (curr = &head_handle_; *curr; ) {
        ButtonDetector* entry = *curr;
        if (entry == this) {
            *curr = entry->next_;
            break;
        } else {
            curr = &entry->next_;
        }
    }
}

/**
 * @brief 读取按钮电平
 * @return 按钮当前电平
 */
uint8_t ButtonDetector::readButtonLevel() const {
    return digitalRead(button_pin_);
}

/**
 * @brief 通过引脚号查找按钮实例
 */
ButtonDetector* ButtonDetector::findByPin(uint8_t pin) {
    ButtonDetector* target = head_handle_;
    while (target) {
        if (target->button_pin_ == pin) {
            return target;
        }
        target = target->next_;
    }
    return nullptr;
}

/**
 * @brief GPIO中断服务程序（ISR）
 * @note 必须放在IRAM中，保证快速执行
 */
void IRAM_ATTR ButtonDetector::gpioISR(void* arg) {
    // 从参数中获取引脚号
    uint8_t pin = (uint8_t)((uint32_t)arg);
    
    // 查找对应的按钮实例
    ButtonDetector* btn = findByPin(pin);
    if (btn == nullptr) {
        return;  // 未找到，忽略
    }
    
    // 读取当前电平
    uint8_t level = digitalRead(pin);
    
    // 按下事件（下降沿）
    if (level == btn->active_level_) {
        // 如果正在消抖中，忽略按下事件（防止重复触发）
        if (btn->debouncing_) {
            return;
        }
        
        btn->isPressed_ = true;
        btn->pressStartTime_ = millis();
        btn->lastLongPressTime_ = btn->pressStartTime_;
        
        // 启动长按定时器（如果启用了长按模式且有长按回调）
        if (longPressEnabled_ && btn->longPressCallback_) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xTimerStartFromISR(btn->longPressTimer_, &xHigherPriorityTaskWoken);
            if (xHigherPriorityTaskWoken) {
                portYIELD_FROM_ISR();
            }
        }
        
        // 标记为正在消抖
        btn->debouncing_ = true;
        
        // 临时禁用该引脚的中断，防止抖动期间重复触发
        detachInterrupt(digitalPinToInterrupt(pin));
        
        // 启动消抖定时器（从ISR中启动）
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xTimerStartFromISR(btn->debounceTimer_, &xHigherPriorityTaskWoken);
        
        // 如果需要，执行任务切换
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    } 
    // 释放事件（上升沿） - 释放事件不管消抖状态都要处理！
    else {
        // 立即设置释放标志 - 这是最关键的
        btn->isPressed_ = false;
        
        // 强制停止长按定时器
        if (btn->longPressTimer_ != nullptr) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            // 直接停止，不管状态
            xTimerStopFromISR(btn->longPressTimer_, &xHigherPriorityTaskWoken);
            if (xHigherPriorityTaskWoken) {
                portYIELD_FROM_ISR();
            }
        }
        
        // 释放事件不需要消抖处理，直接重新启用中断即可
        // 如果中断已被禁用，重新启用
        if (btn->debouncing_) {
            btn->debouncing_ = false;
            attachInterruptArg(
                digitalPinToInterrupt(pin),
                gpioISR,
                (void*)((uint32_t)pin),
                CHANGE
            );
        }
    }
}

/**
 * @brief 消抖定时器回调函数
 */
void ButtonDetector::debounceTimerCallback(TimerHandle_t xTimer) {
    // 从定时器ID中获取ButtonDetector实例指针
    ButtonDetector* btn = static_cast<ButtonDetector*>(pvTimerGetTimerID(xTimer));
    if (btn) {
        btn->handleDebounceComplete();
    }
}

/**
 * @brief 长按定时器回调函数
 */
void ButtonDetector::longPressTimerCallback(TimerHandle_t xTimer) {
    // 从定时器ID中获取ButtonDetector实例指针
    ButtonDetector* btn = static_cast<ButtonDetector*>(pvTimerGetTimerID(xTimer));
    if (btn) {
        btn->handleLongPressCheck();
    }
}

/**
 * @brief 消抖完成处理
 */
void ButtonDetector::handleDebounceComplete() {
    // 再次读取GPIO状态
    uint8_t level = readButtonLevel();
    
    // 获取当前时间
    uint32_t now = millis();
    
    // 根据模式选择冷却时间
    uint32_t cooldownTime = fastResponseMode_ ? COOLDOWN_TIME_MS_FAST : COOLDOWN_TIME_MS_NORMAL;
    
    // 如果确实是按下状态（低电平），触发单击回调
    // 长按模式下也要触发单击，因为有些按键（如旋转、发射）不需要长按
    if (level == active_level_ && callback_) {
        // 如果启用了长按模式且有长按回调，单击回调不需要冷却时间（由长按定时器接管）
        // 如果没有长按模式，则使用冷却时间
        if (longPressEnabled_ && longPressCallback_) {
            // 长按模式下，单击立即触发一次
            callback_();
        } else {
            // 普通模式下，检查冷却时间
            if (now - lastTriggerTime_ >= cooldownTime) {
                lastTriggerTime_ = now;
                callback_();
            }
        }
    } else if (level != active_level_) {
        // 如果消抖后发现是释放状态，确保停止长按定时器
        isPressed_ = false;
        if (longPressTimer_ != nullptr && xTimerIsTimerActive(longPressTimer_)) {
            xTimerStop(longPressTimer_, 0);
        }
    }
    
    // 清除消抖标志
    debouncing_ = false;
    
    // 重新使能中断
    attachInterruptArg(
        digitalPinToInterrupt(button_pin_),
        gpioISR,
        (void*)((uint32_t)button_pin_),
        CHANGE  // 电平变化触发
    );
}

/**
 * @brief 长按检测处理
 */
void ButtonDetector::handleLongPressCheck() {
    // 关键：再次读取实际的GPIO状态，确保按钮真的还在按下
    uint8_t currentLevel = readButtonLevel();
    
    // 如果GPIO显示已释放，或者标志位显示已释放，都不触发
    if (currentLevel != active_level_ || !isPressed_) {
        return;
    }
    
    uint32_t now = millis();
    uint32_t pressDuration = now - pressStartTime_;
    
    // 初始延迟后开始触发
    if (pressDuration >= LONG_PRESS_INITIAL_DELAY) {
        // 检查重复间隔
        if (now - lastLongPressTime_ >= LONG_PRESS_REPEAT_INTERVAL) {
            lastLongPressTime_ = now;
            
            // 触发长按回调前，再次确认按钮状态
            if (longPressCallback_ && readButtonLevel() == active_level_) {
                longPressCallback_();
            }
        }
    }
}

/**
 * @brief 设置快速响应模式
 * @param enabled true为启用快速响应模式，false为普通模式
 */
void ButtonDetector::setFastResponseMode(bool enabled) {
    fastResponseMode_ = enabled;
}

/**
 * @brief 获取当前是否为快速响应模式
 * @return true为快速响应模式，false为普通模式
 */
bool ButtonDetector::isFastResponseMode() {
    return fastResponseMode_;
}

/**
 * @brief 设置长按模式
 * @param enabled true为启用长按模式，false为禁用
 */
void ButtonDetector::setLongPressEnabled(bool enabled) {
    longPressEnabled_ = enabled;
}

/**
 * @brief 获取当前是否启用长按模式
 * @return true为启用长按模式，false为禁用
 */
bool ButtonDetector::isLongPressEnabled() {
    return longPressEnabled_;
}
