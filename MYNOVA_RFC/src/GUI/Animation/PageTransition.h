/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef PageTransition_h
#define PageTransition_h

#include "AnimationEngine.h"
#include "../UIPage.h"
#include "Animation.h"

// 前向声明
class UIEngine;

// 页面过渡动画类
class PageTransition : public Animation {
public:
  PageTransition(UIPage*& fromPage, UIPage*& toPage, AnimationType type, unsigned long duration = 300, bool deleteOldPage = false, UIEngine* engine = nullptr);
  ~PageTransition() override;
  
  void update() override;

private:
  UIPage*& fromPage;      // 起始页面
  UIPage*& toPage;        // 目标页面
  UIPage* oldPageToDelete; // 保存要删除的旧页面指针
  UIEngine* uiEngine;      // UIEngine 引用，用于延迟删除
  AnimationType type;    // 动画类型
  bool shouldDeleteOldPage; // 是否应该删除旧页面
  
  // 渲染滑动动画
  void pageSlide();
  // 渲染淡入淡出动画
  void pageFade();
  // 渲染缩放动画
  void pageZoom();
};

#endif