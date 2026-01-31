/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "ElementAnimation.h"

ElementAnimation::ElementAnimation(AnimationType type, uint16_t duration) {
    this->type = type;
    this->duration = duration;
    this->running = false;
    this->progress = 0.0f;
}

void ElementAnimation::start() {
    this->startTime = millis();
    this->running = true;
    this->progress = 0.0f;
}

void ElementAnimation::update() {
    if (!running) return;
    
    uint32_t currentTime = millis();
    uint32_t elapsedTime = currentTime - startTime;
    
    if (elapsedTime >= duration) {
        progress = 1.0f;
        running = false;
    } else {
        progress = (float)elapsedTime / duration;
    }
    
    float easedProgress = easeInOut(progress);
    // 在这里使用 easedProgress 进行动画计算
}

void ElementAnimation::stop() {
    running = false;
}

float ElementAnimation::easeInOut(float t) {
    // 默认的缓动函数实现
    return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

void ElementAnimation::setType(AnimationType type) {
    this->type = type;
}

void ElementAnimation::setDuration(uint16_t duration) {
    this->duration = duration;
}

AnimationType ElementAnimation::getType() {
    return type;
}

uint16_t ElementAnimation::getDuration() {
    return duration;
}

bool ElementAnimation::isRunning() {
    return running;
}

float ElementAnimation::getProgress() {
    return progress;
}