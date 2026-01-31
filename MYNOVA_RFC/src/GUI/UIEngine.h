/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef UIEngine_h
#define UIEngine_h

#include "UIPage.h"
#include <Arduino.h>
#include <stack>
#include <vector>
#include <U8g2lib.h>
#include "Animation/Animation.h"

class UIEngine {
public:
  UIEngine();
  ~UIEngine();
  void render(U8G2* u8g2);
  void update();
  void navigateTo(UIPage* page, AnimationType aniType = ANIME_NONE);
  UIPage* navigateBack(AnimationType aniType = ANIME_NONE);
  void navigateBack(UIPage* targetPage, AnimationType aniType = ANIME_NONE);
  UIPage* navigateBackSteps(int steps, AnimationType aniType = ANIME_NONE);  // 返回到堆栈中的第N个页面
  UIPage* getCurrentPage();
  void setCurrentPage(UIPage* page);
  
  // 延迟删除：在渲染帧结束后安全删除
  void markForDeletion(UIPage* page);
  void processPendingDeletions();

private:
  UIPage* currentPage;
  UIPage* nextPage;
  std::stack<UIPage*> pages;
  std::vector<UIPage*> pagesToDelete; // 待删除的页面队列
  bool backSubPage(UIPage* page);
};

#endif
