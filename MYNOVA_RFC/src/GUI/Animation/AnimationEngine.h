/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef AnimationEngine_h
#define AnimationEngine_h

#include <Arduino.h>
#include <U8g2lib.h>
#include "Animation.h"
#include <vector>

// 动画引擎类
class AnimationEngine {
public:
  AnimationEngine();
  ~AnimationEngine();
  
  // 更新所有动画
  void update();
  // 添加动画
  void addAnimation(Animation* animation);
  // 移除动画
  void removeAnimation(Animation* animation);
  // 清除所有动画
  void clearAnimations();
  // 是否有动画正在运行
  bool isAnimating();
  
private:
  std::vector<Animation*> animations; // 当前动画
};

extern AnimationEngine animationEngine;

#endif