/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "SelectionAnimation.h"

SelectionAnimation::SelectionAnimation(AnimationType type, uint16_t duration)
  : ElementAnimation(type, duration) {
  // 构造函数
}

SelectionAnimation::~SelectionAnimation() {
  // 析构函数
}

bool SelectionAnimation::getBlinkState() {
  // 根据进度返回闪烁状态
  return sin(this->progress * 6.0f * 3.14159f) > 0;
}

float SelectionAnimation::easeInOut(float t) {
  // 使用正弦函数实现闪烁效果
  return (sin(t * 6.0f * 3.14159f) + 1.0f) * 0.5f;
}