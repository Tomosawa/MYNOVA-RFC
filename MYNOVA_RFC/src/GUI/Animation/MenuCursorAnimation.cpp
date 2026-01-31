/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "MenuCursorAnimation.h"

MenuCursorAnimation::MenuCursorAnimation(UIMenu* menu, int fromPos, int toPos, unsigned long duration)
  : Animation(duration) {
  this->menu = menu;
  this->fromPos = fromPos;
  this->toPos = toPos;
}

MenuCursorAnimation::~MenuCursorAnimation() {
  // 析构函数
}

float MenuCursorAnimation::getCurrentPosition() {
  return this->fromPos + (this->toPos - this->fromPos) * this->progress;
}

void MenuCursorAnimation::update() {
  Animation::update();

  if (progress >= 1.0f) {
    // 动画结束，更新菜单的光标位置
    menu->menuSelY = toPos;
    stop();
  } else {
    menu->menuSelY = getCurrentPosition();
  }
}