/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "UIEngine.h"
#include "Animation/AnimationEngine.h"
#include "Animation/PageTransition.h"
#include "Animation/MenuCursorAnimation.h"
#include "Animation/SelectionAnimation.h"
#include "Animation/PageTransition.h"
#include "../Pages/HomePage.h"

UIEngine::UIEngine() {
  this->currentPage = nullptr;
  this->nextPage = nullptr;
}

UIEngine::~UIEngine() {
  // 清理堆栈中的所有页面
  while (!this->pages.empty()) {
    UIPage* page = this->pages.top();
    this->pages.pop();
    delete page;
  }
  
  // 清理待删除队列
  for (auto page : pagesToDelete) {
    delete page;
  }
  pagesToDelete.clear();
  
  // 清理当前页面
  if (this->currentPage != nullptr) {
    delete this->currentPage;
    this->currentPage = nullptr;
  }
  
  // 清理下一个页面
  if (this->nextPage != nullptr) {
    delete this->nextPage;
    this->nextPage = nullptr;
  }
}

// 标记页面为待删除
void UIEngine::markForDeletion(UIPage* page) {
  if (page != nullptr) {
    Serial.println("markForDeletion: markForDeletion");
    Serial.flush();
    pagesToDelete.push_back(page);
  }
}

// 处理待删除的页面（在渲染帧结束后调用）
void UIEngine::processPendingDeletions() {
  if (pagesToDelete.empty()) {
    return;
  }
  
  Serial.print("processPendingDeletions: start deleting ");
  Serial.print(pagesToDelete.size());
  Serial.println(" pages");
  Serial.flush();
  
  // 先清除所有动画，防止动画引用待删除的对象
  animationEngine.clearAnimations();
  
  for (auto page : pagesToDelete) {
    yield(); // 喂看门狗
    delete page;
  }
  
  pagesToDelete.clear();
  Serial.println("processPendingDeletions: deleted");
  Serial.flush();
}

void UIEngine::render(U8G2* u8g2) {
  
  if (this->currentPage != nullptr) {
    // 渲染当前页面
    this->currentPage->render(u8g2);
  }
  if (this->nextPage != nullptr) {
    // 渲染下一个页面内容
    this->nextPage->render(u8g2);
  }
  
  // 渲染完成后，安全删除待删除的页面
  processPendingDeletions();
}

void UIEngine::update() {
  // 更新动画引擎
  animationEngine.update();
  
  // 更新当前页面
  if (this->currentPage != nullptr) {
    this->currentPage->update();
  }
}

void UIEngine::navigateTo(UIPage* page, AnimationType aniType) {
  if (page == nullptr || page == this->currentPage) {
    return;
  }
  
  // 保存当前页面到堆栈
  if (this->currentPage != nullptr) {
    this->pages.push(this->currentPage);
  }
  
  // 设置下一个页面
  this->nextPage = page;

  // 创建页面过渡动画
  if (aniType != ANIME_NONE && this->currentPage != nullptr) {
    PageTransition *pageTransition = new PageTransition(this->currentPage, this->nextPage, aniType, 300);
    animationEngine.addAnimation(pageTransition);
  } else {
    // 无动画，直接切换
    this->currentPage = page;
    this->nextPage = nullptr;
    // 调用新页面的显示方法
    if (this->currentPage != nullptr) {
      this->currentPage->showPage();
    }
  }
}

UIPage* UIEngine::navigateBack(AnimationType aniType) {
  if (this->pages.empty()) {
    return this->currentPage;
  }
  
  // 获取上一个页面
  UIPage* prevPage = this->pages.top();
  this->pages.pop();
  
  Serial.println("navigateBack: called");
  
  // 设置下一个页面
  this->nextPage = prevPage;
  
  // 创建页面过渡动画
  if (aniType != ANIME_NONE && this->currentPage != nullptr) {
    Serial.println("navigateBack: with animation");
    // 第5个参数 true 表示动画完成后删除旧页面，第6个参数传入 this 用于延迟删除
    PageTransition *pageTransition = new PageTransition(this->currentPage, this->nextPage, aniType, 300, true, this);
    animationEngine.addAnimation(pageTransition);
  } else {
    Serial.println("navigateBack: no animation, delete now");
    // 无动画，保存要删除的旧页面
    UIPage* pageToDelete = this->currentPage;
    // 直接切换
    this->currentPage = this->nextPage;
    this->nextPage = nullptr;
    // 调用新页面的显示方法
    if (this->currentPage != nullptr) {
      this->currentPage->showPage();
    }
    // 标记旧页面为待删除（延迟删除）
    if (pageToDelete != nullptr) {
      Serial.println("navigateBack: mark for deletion");
      Serial.flush();
      markForDeletion(pageToDelete);
    }
  }
  
  return this->currentPage;
}

void UIEngine::navigateBack(UIPage* targetPage, AnimationType aniType) {
  if (targetPage == nullptr || targetPage == this->currentPage) {
    return;
  }
  
  // 查找目标页面
  bool found = false;
  std::stack<UIPage*> tempStack;
  
  while (!this->pages.empty()) {
    UIPage* page = this->pages.top();
    this->pages.pop();
    
    if (page == targetPage) {
      found = true;
      break;
    }
    
    tempStack.push(page);
  }
  
  // 如果没有找到目标页面，恢复堆栈
  if (!found) {
    while (!tempStack.empty()) {
      this->pages.push(tempStack.top());
      tempStack.pop();
    }
    return;
  }
  
  // 删除所有跳过的页面（延迟删除）
  while (!tempStack.empty()) {
    UIPage* pageToDelete = tempStack.top();
    tempStack.pop();
    markForDeletion(pageToDelete);
  }
  
  // 设置下一个页面
  this->nextPage = targetPage;
  
  // 创建页面过渡动画
  if (aniType != ANIME_NONE && this->currentPage != nullptr) {
      // 第5个参数 true 表示动画完成后删除旧页面，第6个参数传入 this 用于延迟删除
      PageTransition *pageTransition = new PageTransition(this->currentPage, this->nextPage, aniType, 300, true, this);
      animationEngine.addAnimation(pageTransition);
  } else {
    // 无动画，保存要删除的旧页面
    UIPage* pageToDelete = this->currentPage;
    // 直接切换
    this->currentPage = this->nextPage;
    this->nextPage = nullptr;
    // 调用新页面的显示方法
    if (this->currentPage != nullptr) {
      this->currentPage->showPage();
    }
    // 标记旧页面为待删除（延迟删除）
    if (pageToDelete != nullptr) {
      markForDeletion(pageToDelete);
    }
  }
}

UIPage* UIEngine::navigateBackSteps(int steps, AnimationType aniType) {
  if (steps <= 0 || this->pages.empty()) {
    return this->currentPage;
  }
  
  // 如果步数超过堆栈大小，返回到最底部的页面
  int stackSize = this->pages.size();
  if (steps > stackSize) {
    steps = stackSize;
  }
  
  // 保存要删除的页面
  std::vector<UIPage*> pagesToDelete;
  UIPage* targetPage = nullptr;
  
  // 从堆栈中取出指定数量的页面
  for (int i = 0; i < steps; i++) {
    if (this->pages.empty()) {
      break;
    }
    UIPage* page = this->pages.top();
    this->pages.pop();
    
    if (i == steps - 1) {
      // 最后一个页面是目标页面
      targetPage = page;
    } else {
      // 其他页面需要删除
      pagesToDelete.push_back(page);
    }
  }
  
  if (targetPage == nullptr) {
    // 如果没有找到目标页面，恢复堆栈
    for (auto it = pagesToDelete.rbegin(); it != pagesToDelete.rend(); ++it) {
      this->pages.push(*it);
    }
    return this->currentPage;
  }
  
  // 标记要删除的页面
  for (auto page : pagesToDelete) {
    markForDeletion(page);
  }
  
  // 设置下一个页面
  this->nextPage = targetPage;
  
  // 创建页面过渡动画
  if (aniType != ANIME_NONE && this->currentPage != nullptr) {
    // 第5个参数 true 表示动画完成后删除旧页面，第6个参数传入 this 用于延迟删除
    PageTransition *pageTransition = new PageTransition(this->currentPage, this->nextPage, aniType, 300, true, this);
    animationEngine.addAnimation(pageTransition);
  } else {
    // 无动画，保存要删除的旧页面
    UIPage* pageToDelete = this->currentPage;
    // 直接切换
    this->currentPage = this->nextPage;
    this->nextPage = nullptr;
    // 调用新页面的显示方法
    if (this->currentPage != nullptr) {
      this->currentPage->showPage();
    }
    // 标记旧页面为待删除（延迟删除）
    if (pageToDelete != nullptr) {
      markForDeletion(pageToDelete);
    }
  }
  
  return this->currentPage;
}

UIPage* UIEngine::getCurrentPage() {
  return this->currentPage;
}

void UIEngine::setCurrentPage(UIPage* page) {
  this->currentPage = page;
  if (this->currentPage != nullptr) {
    this->currentPage->showPage();
  }
}

bool UIEngine::backSubPage(UIPage* page) {
  // 实现子页面返回逻辑
  return false;
}
