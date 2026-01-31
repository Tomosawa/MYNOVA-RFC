/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef UIMessageBox_h
#define UIMessageBox_h

#include "UIWidget.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <functional>
#include "../Animation/MessageBoxAnimation.h"

class UIMessageBox : public UIWidget {
public:
  UIMessageBox(int x = -1, int y = -1, int width = 100, int height = 60);
  ~UIMessageBox();
  
  void render(U8G2* u8g2, int offsetX = 0, int offsetY = 0) override;
  void update(); // 更新动画和计时器
  
  void setMessage(String msg);
  void setTitle(String title);
  void setButtons(String leftBtn, String rightBtn);
  
  // 显示消息框（带动画）
  void show(MessageBoxAnimationType animType, unsigned long animDuration = 300);
  void show(); // 无动画显示
  
  // 隐藏消息框（带可选动画）
  void hide(bool animated = false);
  void hide(MessageBoxAnimationType animType, unsigned long animDuration = 200);
  
  bool isVisible();
  bool isAnimating();
  
  // 设置自动关闭
  void setAutoClose(unsigned long delayMs);
  void cancelAutoClose();
  
  // 设置显示位置（-1表示自动居中，考虑NavBar）
  void setPosition(int x, int y);
  
  // 设置默认位置（考虑屏幕和NavBar尺寸）
  void setDefaultPosition(int screenWidth, int screenHeight, int navBarHeight = 15);
  
  // 按钮高亮动画（类似UINavBar）
  void showLeftButtonHighLight(std::function<void()> onComplete = nullptr);
  void showRightButtonHighLight(std::function<void()> onComplete = nullptr);

  bool bShowTitle;
  bool bShowButtons;
  
  // 动画相关（由MessageBoxAnimation访问）
  MessageBoxAnimation* currentAnimation;
  
  // 按钮高亮状态（public以便动画类访问）
  bool bLeftButtonHighLight;
  bool bRightButtonHighLight;
  
private:
  String message;
  String title;
  String leftButton;
  String rightButton;

  const uint8_t* titleFont = u8g2_font_wqy12_t_gb2312;
  const uint8_t* messageFont = u8g2_font_wqy12_t_gb2312;
  const uint8_t* buttonFont = u8g2_font_wqy12_t_gb2312;
  
  // 自动关闭计时器
  bool autoCloseEnabled;
  unsigned long autoCloseDelay;
  unsigned long showStartTime;
  
  // 原始尺寸（用于动画缩放）
  int originalX;
  int originalY;
  int originalWidth;
  int originalHeight;
  
  // 屏幕尺寸（用于边界检查）
  int screenWidth;
  int screenHeight;
  
  // 渲染辅助函数
  void renderContent(U8G2* u8g2, int actualX, int actualY, int renderWidth, int renderHeight, float alpha);
  
  // 边界检查
  void clampToScreen();
};

#endif

