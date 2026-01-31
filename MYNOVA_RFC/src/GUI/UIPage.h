/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef UIPage_h
#define UIPage_h

#include <Arduino.h>
#include <vector>
#include <U8g2lib.h>
#include "Widget/UIWidget.h"

// 前向声明
class Animation;
// 定义按钮事件回调函数类型
typedef void (*ButtonCallback)(void*);

class UIPage {
public:
  UIPage(int x, int y, int width, int height);
  virtual ~UIPage();
  
  // 保留原有功能
  void addWidget(UIWidget* widget);
  virtual void render(U8G2* u8g2);
  UIPage *subPage = nullptr;

  // 新增showPage函数
  virtual void showPage();
  
  // 页面更新函数，在主循环中调用
  virtual void update();

   // 设置按钮回调函数
  void setOnButtonBack(ButtonCallback callback) { onButtonBackCallback = callback; }
  void setOnButtonMenu(ButtonCallback callback) { onButtonMenuCallback = callback; }
  void setOnButtonEnter(ButtonCallback callback) { onButtonEnterCallback = callback; }
  void setOnButton1(ButtonCallback callback) { onButton1Callback = callback; }
  void setOnButton2(ButtonCallback callback) { onButton2Callback = callback; }
  void setOnButton3(ButtonCallback callback) { onButton3Callback = callback; }
  void setOnButton4(ButtonCallback callback) { onButton4Callback = callback; }
  void setOnButton5(ButtonCallback callback) { onButton5Callback = callback; }
  void setOnButton6(ButtonCallback callback) { onButton6Callback = callback; }
  void setOnButton7(ButtonCallback callback) { onButton7Callback = callback; }
  void setOnButton8(ButtonCallback callback) { onButton8Callback = callback; }
  void setOnButton9(ButtonCallback callback) { onButton9Callback = callback; }

  // 按钮事件处理函数
  virtual void onButtonBack(void* context = nullptr);
  virtual void onButtonMenu(void* context = nullptr);
  virtual void onButtonEnter(void* context = nullptr);
  virtual void onButton1(void* context = nullptr);
  virtual void onButton2(void* context = nullptr);
  virtual void onButton3(void* context = nullptr);
  virtual void onButton4(void* context = nullptr);
  virtual void onButton5(void* context = nullptr);
  virtual void onButton6(void* context = nullptr);
  virtual void onButton7(void* context = nullptr);
  virtual void onButton8(void* context = nullptr);
  virtual void onButton9(void* context = nullptr);
  // 页面整体坐标偏移
  int pageX, pageY;
  int pageWidth, pageHeight;
private:
  std::vector<UIWidget*> widgets;

  // 按钮回调函数指针
  ButtonCallback onButtonBackCallback = nullptr;
  ButtonCallback onButtonMenuCallback = nullptr;
  ButtonCallback onButtonEnterCallback = nullptr;
  ButtonCallback onButton1Callback = nullptr;
  ButtonCallback onButton2Callback = nullptr;
  ButtonCallback onButton3Callback = nullptr;
  ButtonCallback onButton4Callback = nullptr;
  ButtonCallback onButton5Callback = nullptr;
  ButtonCallback onButton6Callback = nullptr;
  ButtonCallback onButton7Callback = nullptr;
  ButtonCallback onButton8Callback = nullptr;
  ButtonCallback onButton9Callback = nullptr;
};

#endif