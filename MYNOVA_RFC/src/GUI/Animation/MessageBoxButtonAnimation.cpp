/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// MessageBoxButtonAnimation.cpp
#include "MessageBoxButtonAnimation.h"
#include "../Widget/UIMessageBox.h"

MessageBoxButtonAnimation::MessageBoxButtonAnimation(UIMessageBox* msgBox, MessageBoxButtonPosition position, unsigned long duration)
  : Animation(duration) {
  this->msgBox = msgBox;
  this->buttonPosition = position;
}

MessageBoxButtonAnimation::~MessageBoxButtonAnimation() {
  // 析构函数
}

void MessageBoxButtonAnimation::update() {
  if (this->state != ANIME_STATE_RUNNING) {
    return;
  }
  
  // 持续高亮模式，直接使用基类的update逻辑
  Animation::update();
  
  // 如果动画已完成，重置按钮的高亮状态
  if (this->state == ANIME_STATE_FINISHED) {
    if (this->msgBox != nullptr) {
      switch (buttonPosition) {
        case MSGBOX_BUTTON_LEFT:
          this->msgBox->bLeftButtonHighLight = false;
          break;
        case MSGBOX_BUTTON_RIGHT:
          this->msgBox->bRightButtonHighLight = false;
          break;
      }
    }
  }
}

