/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// MessageBoxButtonAnimation.h
#ifndef MessageBoxButtonAnimation_h
#define MessageBoxButtonAnimation_h

#include "Animation.h"

// 前向声明
class UIMessageBox;

// 消息框按钮位置枚举
enum MessageBoxButtonPosition {
  MSGBOX_BUTTON_LEFT,    // 左键
  MSGBOX_BUTTON_RIGHT    // 右键
};

// 消息框按钮高亮动画类
class MessageBoxButtonAnimation : public Animation {
public:
  MessageBoxButtonAnimation(UIMessageBox* msgBox, MessageBoxButtonPosition position, unsigned long duration = 300);
  ~MessageBoxButtonAnimation();
  
  void update() override;
  
private:
  UIMessageBox* msgBox;  // 消息框对象
  MessageBoxButtonPosition buttonPosition;  // 按钮位置
};

#endif

