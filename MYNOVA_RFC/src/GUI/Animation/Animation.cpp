/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "Animation.h"

Animation::Animation(unsigned long duration) {
  this->duration = duration;
  this->state = ANIME_STATE_IDLE;
  this->startTime = 0;
  this->pauseTime = 0;
  this->progress = 0.0f;
  this->onComplete = nullptr;
}

Animation::~Animation() {
  // 析构函数
}

void Animation::update() {
  if (this->state == ANIME_STATE_RUNNING) {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - this->startTime;
    
    if (elapsedTime >= this->duration) {
      // 动画完成
      this->progress = 1.0f;
      this->state = ANIME_STATE_FINISHED;
      
      // 调用完成回调
      if (this->onComplete) {
        this->onComplete();
      }
    } else {
      // 更新进度
      this->progress = (float)elapsedTime / (float)this->duration;
    }
  }
}

void Animation::start() {
  this->startTime = millis();
  this->state = ANIME_STATE_RUNNING;
  this->progress = 0.0f;
}

void Animation::pause() {
  if (this->state == ANIME_STATE_RUNNING) {
    this->pauseTime = millis();
    this->state = ANIME_STATE_PAUSED;
  }
}

void Animation::resume() {
  if (this->state == ANIME_STATE_PAUSED) {
    // 调整开始时间，考虑暂停的时间
    unsigned long pauseDuration = millis() - this->pauseTime;
    this->startTime += pauseDuration;
    this->state = ANIME_STATE_RUNNING;
  }
}

void Animation::stop() {
  this->state = ANIME_STATE_IDLE;
}

void Animation::reset() {
  this->state = ANIME_STATE_IDLE;
  this->progress = 0.0f;
}

AnimationState Animation::getState() const {
  return this->state;
}

float Animation::getProgress() const {
  return this->progress;
}

void Animation::setOnComplete(std::function<void()> callback) {
  this->onComplete = callback;
}