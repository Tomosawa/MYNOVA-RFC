/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef Animation_h
#define Animation_h

#include <Arduino.h>
#include <U8g2lib.h>
#include <functional>

// 动画类型枚举
enum AnimationType {
    ANIME_NONE,
    ANIME_SLIDE_IN_LEFT,       // 从右向左滑动进入
    ANIME_SLIDE_IN_RIGHT,      // 从左向右滑动进入
    ANIME_SLIDE_IN_UP,         // 从下向上滑动进入
    ANIME_SLIDE_IN_DOWN,       // 从上向下滑动进入
    ANIME_SLIDE_OUT_LEFT,      // 从右向左滑动退出
    ANIME_SLIDE_OUT_RIGHT,     // 从右向左滑动退出
    ANIME_SLIDE_OUT_UP,        // 从右向左滑动退出
    ANIME_SLIDE_OUT_DOWN,      // 从右向左滑动退出
    ANIME_ZOOM_IN,             // 放大进入 
    ANIME_ZOOM_OUT             // 缩小退出
  };
// 动画状态枚举
enum AnimationState {
  ANIME_STATE_IDLE,      // 空闲状态
  ANIME_STATE_RUNNING,   // 运行中
  ANIME_STATE_PAUSED,    // 暂停
  ANIME_STATE_FINISHED   // 已完成
};

class Animation {
public:
  Animation(unsigned long duration = 300);
  virtual ~Animation();
  
  // 更新动画状态
  virtual void update();
  
  // 开始动画
  virtual void start();
  
  // 暂停动画
  virtual void pause();
  
  // 恢复动画
  virtual void resume();
  
  // 停止动画
  virtual void stop();
  
  // 重置动画
  virtual void reset();
  
  // 获取动画状态
  AnimationState getState() const;
  
  // 获取动画进度 (0.0 - 1.0)
  float getProgress() const;
  
  // 设置完成回调
  void setOnComplete(std::function<void()> callback);
  
protected:
  AnimationState state;       // 当前状态
  unsigned long startTime;    // 开始时间
  unsigned long pauseTime;    // 暂停时间
  unsigned long duration;     // 持续时间
  float progress;             // 进度 (0.0 - 1.0)
  std::function<void()> onComplete; // 完成回调
};

#endif