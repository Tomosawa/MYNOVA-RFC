/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef UITextArea_h
#define UITextArea_h

#include "UIWidget.h"
#include <vector>

class UITextArea : public UIWidget {
public:
  UITextArea();
  virtual ~UITextArea();
  
  void setText(String text);
  void scrollUp();
  void scrollDown();
  void render(U8G2* u8g2, int offsetX = 0, int offsetY = 0) override;

public:
  String text;
  int scrollIndex; // 当前显示的起始行索引
  int visibleLines; // 可见行数
  int lineHeight; // 行高
  const uint8_t* textFont;
  
private:
  std::vector<String> lines;
  void splitText(U8G2* u8g2); // 将文本分割成多行
  bool needRecalculate;
};

#endif
