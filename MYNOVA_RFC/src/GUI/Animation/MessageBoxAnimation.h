/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef MessageBoxAnimation_h
#define MessageBoxAnimation_h

#include "Animation.h"

// MessageBox动画类型
enum MessageBoxAnimationType {
  MSGBOX_ANIME_NONE,              // 无动画
  MSGBOX_ANIME_ZOOM_CENTER,       // 从中心点缩放展开
  MSGBOX_ANIME_ZOOM_BOTTOM,       // 从底部缩放展开（类似macOS dock）
  MSGBOX_ANIME_FADE_IN,           // 淡入
  MSGBOX_ANIME_DROP_DOWN,         // 从上方下落
  MSGBOX_ANIME_SLIDE_UP,          // 从下方滑入
  MSGBOX_ANIME_BOUNCE_IN,         // 弹跳进入
  // 退出动画
  MSGBOX_ANIME_ZOOM_OUT,          // 缩小消失
  MSGBOX_ANIME_FADE_OUT,          // 淡出
  MSGBOX_ANIME_SLIDE_DOWN         // 向下滑出
};

class UIMessageBox; // 前向声明

class MessageBoxAnimation : public Animation {
public:
  MessageBoxAnimation(UIMessageBox* msgBox, MessageBoxAnimationType type, unsigned long duration = 300);
  virtual ~MessageBoxAnimation();
  
  void update() override;
  void start() override;
  
  // 设置动画类型
  void setAnimationType(MessageBoxAnimationType type);
  
  // 获取当前的缩放和偏移值（用于渲染）
  float getScaleX() const { return scaleX; }
  float getScaleY() const { return scaleY; }
  int getOffsetX() const { return offsetX; }
  int getOffsetY() const { return offsetY; }
  float getAlpha() const { return alpha; }
  
private:
  UIMessageBox* messageBox;
  MessageBoxAnimationType animType;
  
  // 动画参数
  float scaleX;
  float scaleY;
  int offsetX;
  int offsetY;
  float alpha;
  
  // 缓动函数
  float easeOutCubic(float t);
  float easeOutBounce(float t);
  float easeOutBack(float t);
  
  // 更新动画参数
  void updateZoomCenter(float progress);
  void updateZoomBottom(float progress);
  void updateFadeIn(float progress);
  void updateDropDown(float progress);
  void updateSlideUp(float progress);
  void updateBounceIn(float progress);
  // 退出动画
  void updateZoomOut(float progress);
  void updateFadeOut(float progress);
  void updateSlideDown(float progress);
};

#endif

