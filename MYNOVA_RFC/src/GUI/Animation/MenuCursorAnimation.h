/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef MenuCursorAnimation_h
#define MenuCursorAnimation_h

#include "AnimationEngine.h"
#include "../Widget/UIMenu.h"

// 菜单光标动画类
class MenuCursorAnimation : public Animation {
public:
  MenuCursorAnimation(UIMenu* menu, int fromPos, int toPos, unsigned long duration = 200);
  ~MenuCursorAnimation();

  void update() override;
  // 获取当前光标位置
  float getCurrentPosition();
  
private:
  UIMenu* menu;         // 菜单对象
  int fromPos;          // 起始位置
  int toPos;            // 目标位置
};

#endif