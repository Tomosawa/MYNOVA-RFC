/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "UIPage.h"
#include "Animation/Animation.h"

UIPage::UIPage(int x, int y, int width, int height) {
  // 构造函数
  pageX = x;
  pageY = y;
  pageWidth = width;
  pageHeight = height;
}

UIPage::~UIPage() {
  // 先删除子页面
  if (subPage != nullptr) {
    yield(); // 喂看门狗
    delete subPage;
    subPage = nullptr;
  }
  
  // 删除所有widgets
  for (size_t i = 0; i < widgets.size(); i++) {
    auto widget = widgets[i];
    if (widget != nullptr) {
      delete widget;
      widgets[i] = nullptr;
    }
    // 每隔几个对象喂一次看门狗
    if (i % 5 == 0) {
      yield();
    }
  }
  widgets.clear();
}

void UIPage::addWidget(UIWidget* widget) {
  this->widgets.push_back(widget);
}

void UIPage::render(U8G2* u8g2) {
  // 渲染所有Widget
  for (auto widget : widgets) {
    widget->render(u8g2, pageX, pageY);
  }
  
  // 如果有子页面，渲染子页面
  if (subPage != nullptr) {
    subPage->render(u8g2);
  }
}

// 实现showPage函数，默认不执行任何操作
void UIPage::showPage() {
  // 默认实现为空，子类可以重写此方法以执行页面显示时的初始化操作
}

// 实现update函数，默认不执行任何操作
void UIPage::update() {
  // 默认实现为空，子类可以重写此方法以执行页面更新逻辑
}

// 实现按钮事件处理函数
void UIPage::onButtonBack(void* context) {
  if (onButtonBackCallback) {
    onButtonBackCallback(context);
  }
}

void UIPage::onButtonMenu(void* context) {
  if (onButtonMenuCallback) {
    onButtonMenuCallback(context);
  }
}

void UIPage::onButtonEnter(void* context) {
  if (onButtonEnterCallback) {
    onButtonEnterCallback(context);
  }
}

// 为其他按钮实现类似的方法
void UIPage::onButton1(void* context) {
  if (onButton1Callback) {
    onButton1Callback(context);
  }
}

void UIPage::onButton2(void* context) {
  if (onButton2Callback) {
    onButton2Callback(context);
  }
}

void UIPage::onButton3(void* context) {
  if (onButton3Callback) {
    onButton3Callback(context);
  }
}

void UIPage::onButton4(void* context) {
  if (onButton4Callback) {
    onButton4Callback(context);
  }
}

void UIPage::onButton5(void* context) {
  if (onButton5Callback) {
    onButton5Callback(context);
  }
}

void UIPage::onButton6(void* context) {
  if (onButton6Callback) {
    onButton6Callback(context);
  }
}

void UIPage::onButton7(void* context) {
  if (onButton7Callback) {
    onButton7Callback(context);
  }
}

void UIPage::onButton8(void* context) {
  if (onButton8Callback) {
    onButton8Callback(context);
  }
}

void UIPage::onButton9(void* context) {
  if (onButton9Callback) {
    onButton9Callback(context);
  }
}