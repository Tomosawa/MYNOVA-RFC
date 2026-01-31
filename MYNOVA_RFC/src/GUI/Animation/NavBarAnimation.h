/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// NavBarAnimation.h
#ifndef NavBarAnimation_h
#define NavBarAnimation_h

#include "Animation.h"
#include "../Widget/UINavBar.h"

// 导航栏按钮位置枚举
enum NavBarButtonPosition {
  NAV_BUTTON_LEFT,    // 左键
  NAV_BUTTON_MIDDLE,  // 中键
  NAV_BUTTON_RIGHT    // 右键
};

// 导航栏动画类型枚举
enum NavBarAnimationType {
  NAV_BAR_ANIMATION_CONSTANT,  // 持续高亮
  NAV_BAR_ANIMATION_BLINK      // 闪烁效果
};

// 导航栏按钮高亮动画类
class NavBarAnimation : public Animation {
public:
  NavBarAnimation(UINavBar* navBar, NavBarButtonPosition position, unsigned long duration = 300);
  ~NavBarAnimation();
  
  void update() override;
  
  // 设置为持续高亮模式
  void setConstantMode();
  
  // 设置为闪烁模式
  void setBlinkMode(int blinkCount = 3, unsigned long onDuration = 200, unsigned long offDuration = 200);
  
private:
  UINavBar* navBar;  // 导航栏对象
  NavBarButtonPosition buttonPosition;  // 按钮位置
  NavBarAnimationType animType;  // 动画类型
  
  // 闪烁相关参数
  int blinkCount;         // 闪烁次数
  int currentBlink;       // 当前已闪烁次数
  unsigned long onDuration;   // 高亮持续时间
  unsigned long offDuration;  // 熄灭持续时间
  bool isOn;              // 当前状态（高亮/熄灭）
  unsigned long stateStartTime;  // 当前状态开始时间
};

#endif

