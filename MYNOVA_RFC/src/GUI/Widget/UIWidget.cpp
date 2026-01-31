/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "UIWidget.h"

UIWidget::UIWidget() {
  // 初始化默认位置和大小
  x = 0;
  y = 0;
  width = 0;
  height = 0;
  margin_left = 0;
  margin_top = 0; 
  margin_right = 0;
  margin_bottom = 0;
  padding_left = 0;
  padding_top = 0;
  padding_right = 0; 
  padding_bottom = 0;
  bVisible = true;  // 默认可见
}

UIWidget::~UIWidget() {
  // 析构函数实现
}

void UIWidget::render(U8G2* u8g2,int offsetX, int offsetY) {

}