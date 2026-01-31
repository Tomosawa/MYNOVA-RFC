/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef QuickButtonAnimation_h
#define QuickButtonAnimation_h

#include "Animation.h"
#include "../Widget/UIQuickButton.h"

// 按钮动画类型枚举
enum QuickButtonAnimationType {
  QUICK_BUTTON_ANIMATION_CONSTANT,  // 持续高亮
  QUICK_BUTTON_ANIMATION_BLINK      // 闪烁效果
};

// 按钮高亮动画类
class QuickButtonAnimation : public Animation {
public:
  QuickButtonAnimation(UIQuickButton* button, unsigned long duration = 300);
  ~QuickButtonAnimation();
  
  void update() override;
  
  // 设置为持续高亮模式
  void setConstantMode();
  
  // 设置为闪烁模式
  void setBlinkMode(int blinkCount = 3, unsigned long onDuration = 200, unsigned long offDuration = 200);
  
private:
  UIQuickButton* button;  // 按钮对象
  QuickButtonAnimationType animType;  // 动画类型
  
  // 闪烁相关参数
  int blinkCount;         // 闪烁次数
  int currentBlink;       // 当前已闪烁次数
  unsigned long onDuration;   // 高亮持续时间
  unsigned long offDuration;  // 熄灭持续时间
  bool isOn;              // 当前状态（高亮/熄灭）
  unsigned long stateStartTime;  // 当前状态开始时间
};

#endif