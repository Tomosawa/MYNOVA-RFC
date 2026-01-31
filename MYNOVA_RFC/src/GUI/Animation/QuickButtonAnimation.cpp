/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "QuickButtonAnimation.h"

QuickButtonAnimation::QuickButtonAnimation(UIQuickButton* button, unsigned long duration)
  : Animation(duration) {
  this->button = button;
  this->animType = QUICK_BUTTON_ANIMATION_CONSTANT;  // 默认为持续高亮模式
  this->blinkCount = 3;
  this->onDuration = 200;
  this->offDuration = 200;
  this->currentBlink = 0;
  this->isOn = true;
  this->stateStartTime = 0;
}

QuickButtonAnimation::~QuickButtonAnimation() {
  // 析构函数
}

void QuickButtonAnimation::update() {
  if (this->state != ANIME_STATE_RUNNING) {
    return;
  }
  
  unsigned long currentTime = millis();
  
  switch (animType) {
    case QUICK_BUTTON_ANIMATION_CONSTANT:
      // 持续高亮模式，直接使用基类的update逻辑
      Animation::update();
      
      // 如果动画已完成，重置按钮的高亮状态
      if (this->state == ANIME_STATE_FINISHED) {
        if (this->button != nullptr) {
          this->button->bHighLight = false;
        }
      }
      break;
      
    case QUICK_BUTTON_ANIMATION_BLINK:
      // 闪烁模式
      if (stateStartTime == 0) {
        // 初始化状态开始时间
        stateStartTime = currentTime;
        if (this->button != nullptr) {
          this->button->bHighLight = isOn;
        }
      }
      
      unsigned long elapsedTime = currentTime - stateStartTime;
      unsigned long currentStateDuration = isOn ? onDuration : offDuration;
      
      // 检查是否需要切换状态
      if (elapsedTime >= currentStateDuration) {
        if (isOn) {
          // 从高亮切换到熄灭
          isOn = false;
          currentBlink++;
        } else {
          // 从熄灭切换到高亮
          isOn = true;
        }
        
        // 更新按钮状态
        if (this->button != nullptr) {
          this->button->bHighLight = isOn;
        }
        
        // 更新状态开始时间
        stateStartTime = currentTime;
        
        // 检查是否已完成所有闪烁
        if (currentBlink >= blinkCount) {
          this->state = ANIME_STATE_FINISHED;
          // 确保最终状态为熄灭
          if (this->button != nullptr) {
            this->button->bHighLight = false;
          }
        }
      }
      break;
  }
}

void QuickButtonAnimation::setConstantMode() {
  this->animType = QUICK_BUTTON_ANIMATION_CONSTANT;
}

void QuickButtonAnimation::setBlinkMode(int blinkCount, unsigned long onDuration, unsigned long offDuration) {
  this->animType = QUICK_BUTTON_ANIMATION_BLINK;
  this->blinkCount = blinkCount;
  this->onDuration = onDuration;
  this->offDuration = offDuration;
  this->currentBlink = 0;
  this->isOn = true;
  this->stateStartTime = 0;
}