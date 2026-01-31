/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "MessageBoxAnimation.h"
#include "../Widget/UIMessageBox.h"
#include <math.h>

MessageBoxAnimation::MessageBoxAnimation(UIMessageBox* msgBox, MessageBoxAnimationType type, unsigned long duration)
  : Animation(duration) {
  this->messageBox = msgBox;
  this->animType = type;
  this->scaleX = 1.0f;
  this->scaleY = 1.0f;
  this->offsetX = 0;
  this->offsetY = 0;
  this->alpha = 1.0f;
}

MessageBoxAnimation::~MessageBoxAnimation() {
  // 析构函数
}

void MessageBoxAnimation::start() {
  Animation::start();
  
  // 根据动画类型初始化参数
  switch (animType) {
    case MSGBOX_ANIME_ZOOM_CENTER:
    case MSGBOX_ANIME_ZOOM_BOTTOM:
    case MSGBOX_ANIME_BOUNCE_IN:
      scaleX = 0.0f;
      scaleY = 0.0f;
      break;
      
    case MSGBOX_ANIME_FADE_IN:
      alpha = 0.0f;
      break;
      
    case MSGBOX_ANIME_DROP_DOWN:
      offsetY = -50;
      alpha = 0.0f;
      break;
      
    case MSGBOX_ANIME_SLIDE_UP:
      offsetY = 50;
      alpha = 0.8f;
      break;
      
    // 退出动画初始状态（从正常状态开始）
    case MSGBOX_ANIME_ZOOM_OUT:
      scaleX = 1.0f;
      scaleY = 1.0f;
      alpha = 1.0f;
      break;
      
    case MSGBOX_ANIME_FADE_OUT:
      alpha = 1.0f;
      break;
      
    case MSGBOX_ANIME_SLIDE_DOWN:
      offsetY = 0;
      alpha = 1.0f;
      break;
      
    default:
      break;
  }
}

void MessageBoxAnimation::update() {
  Animation::update();
  
  if (state == ANIME_STATE_RUNNING) {
    // 根据动画类型更新参数
    switch (animType) {
      case MSGBOX_ANIME_ZOOM_CENTER:
        updateZoomCenter(progress);
        break;
        
      case MSGBOX_ANIME_ZOOM_BOTTOM:
        updateZoomBottom(progress);
        break;
        
      case MSGBOX_ANIME_FADE_IN:
        updateFadeIn(progress);
        break;
        
      case MSGBOX_ANIME_DROP_DOWN:
        updateDropDown(progress);
        break;
        
      case MSGBOX_ANIME_SLIDE_UP:
        updateSlideUp(progress);
        break;
        
      case MSGBOX_ANIME_BOUNCE_IN:
        updateBounceIn(progress);
        break;
        
      case MSGBOX_ANIME_ZOOM_OUT:
        updateZoomOut(progress);
        break;
        
      case MSGBOX_ANIME_FADE_OUT:
        updateFadeOut(progress);
        break;
        
      case MSGBOX_ANIME_SLIDE_DOWN:
        updateSlideDown(progress);
        break;
        
      default:
        break;
    }
  }
}

void MessageBoxAnimation::setAnimationType(MessageBoxAnimationType type) {
  this->animType = type;
}

// 缓动函数
float MessageBoxAnimation::easeOutCubic(float t) {
  float f = t - 1.0f;
  return f * f * f + 1.0f;
}

float MessageBoxAnimation::easeOutBounce(float t) {
  if (t < (1.0f / 2.75f)) {
    return 7.5625f * t * t;
  } else if (t < (2.0f / 2.75f)) {
    t -= (1.5f / 2.75f);
    return 7.5625f * t * t + 0.75f;
  } else if (t < (2.5f / 2.75f)) {
    t -= (2.25f / 2.75f);
    return 7.5625f * t * t + 0.9375f;
  } else {
    t -= (2.625f / 2.75f);
    return 7.5625f * t * t + 0.984375f;
  }
}

float MessageBoxAnimation::easeOutBack(float t) {
  float c1 = 1.70158f;
  float c3 = c1 + 1.0f;
  return 1.0f + c3 * pow(t - 1.0f, 3.0f) + c1 * pow(t - 1.0f, 2.0f);
}

// 更新各种动画效果
void MessageBoxAnimation::updateZoomCenter(float progress) {
  float easedProgress = easeOutCubic(progress);
  scaleX = easedProgress;
  scaleY = easedProgress;
  alpha = progress;
  offsetX = 0;
  offsetY = 0;
}

void MessageBoxAnimation::updateZoomBottom(float progress) {
  float easedProgress = easeOutBack(progress);
  scaleX = easedProgress;
  scaleY = easedProgress;
  alpha = progress;
  
  // 从底部位置开始
  if (messageBox) {
    // 计算从底部向上的偏移
    offsetY = (int)((1.0f - easedProgress) * 30);
  }
}

void MessageBoxAnimation::updateFadeIn(float progress) {
  alpha = progress;
  scaleX = 1.0f;
  scaleY = 1.0f;
  offsetX = 0;
  offsetY = 0;
}

void MessageBoxAnimation::updateDropDown(float progress) {
  float easedProgress = easeOutBounce(progress);
  offsetY = (int)(-50 * (1.0f - easedProgress));
  alpha = progress;
  scaleX = 1.0f;
  scaleY = 1.0f;
}

void MessageBoxAnimation::updateSlideUp(float progress) {
  float easedProgress = easeOutCubic(progress);
  offsetY = (int)(50 * (1.0f - easedProgress));
  alpha = 0.8f + 0.2f * progress;
  scaleX = 1.0f;
  scaleY = 1.0f;
}

void MessageBoxAnimation::updateBounceIn(float progress) {
  float easedProgress = easeOutBounce(progress);
  scaleX = easedProgress;
  scaleY = easedProgress;
  alpha = progress;
  offsetX = 0;
  offsetY = 0;
}

// 退出动画实现
void MessageBoxAnimation::updateZoomOut(float progress) {
  // 使用easeInCubic效果（反向的easeOutCubic）
  float easedProgress = progress * progress * progress;
  scaleX = 1.0f - easedProgress;
  scaleY = 1.0f - easedProgress;
  alpha = 1.0f - progress;
  offsetX = 0;
  offsetY = 0;
}

void MessageBoxAnimation::updateFadeOut(float progress) {
  alpha = 1.0f - progress;
  scaleX = 1.0f;
  scaleY = 1.0f;
  offsetX = 0;
  offsetY = 0;
}

void MessageBoxAnimation::updateSlideDown(float progress) {
  // 使用easeInCubic效果
  float easedProgress = progress * progress * progress;
  offsetY = (int)(50 * easedProgress);
  alpha = 1.0f - progress;
  scaleX = 1.0f;
  scaleY = 1.0f;
}

