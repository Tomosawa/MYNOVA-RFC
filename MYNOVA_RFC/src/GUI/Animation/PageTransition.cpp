/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "PageTransition.h"
#include "../../GUIRender.h"
#include "../UIEngine.h"

PageTransition::PageTransition(UIPage*& fromPage, UIPage*& toPage, AnimationType type, unsigned long duration, bool deleteOldPage, UIEngine* engine)
  : Animation(duration), fromPage(fromPage), toPage(toPage), type(type), shouldDeleteOldPage(deleteOldPage), oldPageToDelete(nullptr), uiEngine(engine) {
  
  // 如果需要删除旧页面,保存当前的 fromPage 指针
  if (shouldDeleteOldPage) {
    oldPageToDelete = fromPage;
  }
  
  // 初始化页面位置
   switch (this->type) {
    case ANIME_SLIDE_IN_LEFT:
      toPage->pageX = SCREEN_WIDTH;
      break;
    case ANIME_SLIDE_IN_RIGHT:
      toPage->pageX = -SCREEN_WIDTH;
      break;
    case ANIME_SLIDE_IN_UP:
      toPage->pageY = SCREEN_HEIGHT;
      break;
    case ANIME_SLIDE_IN_DOWN:
      toPage->pageY = -SCREEN_HEIGHT;
      break;
    case ANIME_SLIDE_OUT_LEFT:
      break;
    case ANIME_SLIDE_OUT_RIGHT:
      break;
    case ANIME_SLIDE_OUT_UP:
      break;
    case ANIME_SLIDE_OUT_DOWN:
      break;
    default:
      break;
  }
}

PageTransition::~PageTransition() {
  // 析构函数
}

void PageTransition::update() {
  Animation::update();
  switch (this->type) {
    case ANIME_SLIDE_IN_LEFT:
    case ANIME_SLIDE_IN_RIGHT:
    case ANIME_SLIDE_IN_UP:
    case ANIME_SLIDE_IN_DOWN:
      this->pageSlide();
      break;
    case ANIME_ZOOM_IN:
    case ANIME_ZOOM_OUT:
      this->pageZoom();
      break;
    default:
        progress = 1.0f;
        stop();
      break;
  }
  // 检查动画是否结束，如果结束则更新外部指针
  if (this->progress >= 1.0f) {
    fromPage = toPage;
    toPage = nullptr;
    
    // 调用新页面的showPage方法
    if (fromPage != nullptr) {
      fromPage->showPage();
    }
    
    // 在切换完成后延迟删除旧页面
    if (shouldDeleteOldPage && oldPageToDelete != nullptr && uiEngine != nullptr) {
      Serial.println("PageTransition: markForDeletion");
      uiEngine->markForDeletion(oldPageToDelete);
      oldPageToDelete = nullptr;
    } else if (shouldDeleteOldPage && oldPageToDelete != nullptr) {
      // 如果没有 UIEngine 引用，直接删除（兼容旧代码）
      Serial.println("PageTransition: direct delete old page (no UIEngine reference)");
      Serial.flush();
      yield();
      delete oldPageToDelete;
      oldPageToDelete = nullptr;
      yield();
    }
  }
}

// 替换 pushMatrix 和 popMatrix 方法
void PageTransition::pageSlide() {
  int width = toPage->pageWidth;
  int height = toPage->pageHeight;
  
  // 计算偏移量
  switch (this->type) {
    case ANIME_SLIDE_IN_LEFT:  // 从右向左滑动
      toPage->pageX = (int)((1.0f - this->progress) * width);
      break;
      
    case ANIME_SLIDE_IN_RIGHT:  // 从左向右滑动
      toPage->pageX = (int)(this->progress * width);
      break;
      
    case ANIME_SLIDE_IN_UP:  // 从下向上滑动
      toPage->pageY = (int)((1.0f - this->progress) * height);
      break;
      
    case ANIME_SLIDE_IN_DOWN:  // 从上向下滑动
      toPage->pageY = (int)(this->progress * height);
      break;
      
    default:
      break;
  }
}

void PageTransition::pageFade() {
  
}

void PageTransition::pageZoom() {
  
}