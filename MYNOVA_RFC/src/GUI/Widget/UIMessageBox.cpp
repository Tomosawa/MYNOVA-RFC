/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "UIMessageBox.h"
#include "../Animation/MessageBoxAnimation.h"
#include "../Animation/MessageBoxButtonAnimation.h"
#include "../Animation/AnimationEngine.h"
#include <U8g2lib.h>

UIMessageBox::UIMessageBox(int x, int y, int width, int height) {
  // 初始化消息框
  this->width = width;
  this->height = height;
  this->x = x; 
  this->y = y;
  
  // 保存原始尺寸
  this->originalX = x;
  this->originalY = y;
  this->originalWidth = width;
  this->originalHeight = height;

  // 设置默认边距和内边距
  margin_left = 2;
  margin_top = 2;
  margin_right = 2;
  margin_bottom = 2;
  padding_left = 4;
  padding_top = 4;
  padding_right = 4;
  padding_bottom = 4;

  // 初始化状态
  bVisible = false;
  bShowTitle = true;
  bShowButtons = false;  // 默认不显示按钮（由UINavBar显示）

  // 初始化默认内容
  message = "";
  title = "提示";
  leftButton = "确定";
  rightButton = "取消";
  
  // 初始化动画
  currentAnimation = nullptr;
  
  // 初始化自动关闭
  autoCloseEnabled = false;
  autoCloseDelay = 0;
  showStartTime = 0;
  
  // 初始化按钮高亮状态
  bLeftButtonHighLight = false;
  bRightButtonHighLight = false;
  
  // 初始化屏幕尺寸（默认值）
  screenWidth = 128;
  screenHeight = 64;
}

UIMessageBox::~UIMessageBox() {
  // 清理动画指针（不访问，AnimationEngine会管理生命周期）
  currentAnimation = nullptr;
}

void UIMessageBox::update() {
  // 更新自动关闭计时器
  if (bVisible && autoCloseEnabled) {
    unsigned long currentTime = millis();
    if (currentTime - showStartTime >= autoCloseDelay) {
      hide(false);  // 自动关闭，不使用动画
      autoCloseEnabled = false;
    }
  }
}

void UIMessageBox::render(U8G2* u8g2, int offsetX, int offsetY) {
  // 检查是否可见
  if (!bVisible) return;

  int actualX = offsetX + originalX;
  int actualY = offsetY + originalY;
  int renderWidth = originalWidth;
  int renderHeight = originalHeight;
  float alpha = 1.0f;
  
  // 如果有动画正在运行，应用动画效果
  if (currentAnimation != nullptr && currentAnimation->getState() == ANIME_STATE_RUNNING) {
    float scaleX = currentAnimation->getScaleX();
    float scaleY = currentAnimation->getScaleY();
    int animOffsetX = currentAnimation->getOffsetX();
    int animOffsetY = currentAnimation->getOffsetY();
    alpha = currentAnimation->getAlpha();
    
    // 计算缩放后的尺寸
    renderWidth = (int)(originalWidth * scaleX);
    renderHeight = (int)(originalHeight * scaleY);
    
    // 计算居中位置（缩放时从中心缩放）
    actualX = actualX + (originalWidth - renderWidth) / 2 + animOffsetX;
    actualY = actualY + (originalHeight - renderHeight) / 2 + animOffsetY;
    
    // 如果尺寸太小，不渲染
    if (renderWidth < 2 || renderHeight < 2) {
      return;
    }
  }
  
  // 渲染内容
  renderContent(u8g2, actualX, actualY, renderWidth, renderHeight, alpha);
}

void UIMessageBox::renderContent(U8G2* u8g2, int actualX, int actualY, int renderWidth, int renderHeight, float alpha) {

  // 绘制背景
  uint8_t oldColor = u8g2->getDrawColor();
  u8g2->setDrawColor(0);
  u8g2->drawBox(actualX, actualY, renderWidth, renderHeight);
  u8g2->setDrawColor(1);

  // 绘制边框
  u8g2->drawFrame(actualX, actualY, renderWidth, renderHeight);

  // 计算内容区域（根据渲染尺寸调整）
  float scale = (float)renderWidth / (float)originalWidth;
  int scaledPaddingLeft = (int)(padding_left * scale);
  int scaledPaddingRight = (int)(padding_right * scale);
  int scaledPaddingTop = (int)(padding_top * scale);
  int scaledPaddingBottom = (int)(padding_bottom * scale);
  
  int contentX = actualX + scaledPaddingLeft;
  int contentWidth = renderWidth - scaledPaddingLeft - scaledPaddingRight;

  int titleHeight = 0;
  int buttonHeight = 0;
  int messageY = actualY + scaledPaddingTop;

  // 绘制标题
  if (bShowTitle && title.length() > 0 && scale > 0.3f) { // 缩放太小时不显示
    u8g2->setFont(titleFont);
    titleHeight = (int)((u8g2->getMaxCharHeight() + 2) * scale);

    // 绘制标题分隔线
    u8g2->drawLine(actualX, actualY + titleHeight, actualX + renderWidth, actualY + titleHeight);

    // 居中显示标题
    uint16_t titleWidth = u8g2->getUTF8Width(title.c_str());
    u8g2->drawUTF8(contentX + (contentWidth - titleWidth) / 2, 
                   actualY + (int)(u8g2->getMaxCharHeight() * scale), 
                   title.c_str());

    messageY += titleHeight + (int)(2 * scale);
  }

  // 绘制按钮
  if (bShowButtons && scale > 0.3f) { // 缩放太小时不显示
    u8g2->setFont(buttonFont);
    int fontHeight = u8g2->getMaxCharHeight();
    buttonHeight = (int)((fontHeight + 4) * scale);

    // 绘制按钮分隔线
    u8g2->drawLine(actualX, actualY + renderHeight - buttonHeight, 
                   actualX + renderWidth, actualY + renderHeight - buttonHeight);

    // 绘制中间分隔线
    u8g2->drawLine(actualX + renderWidth / 2, actualY + renderHeight - buttonHeight,
                   actualX + renderWidth / 2, actualY + renderHeight);

    // 计算按钮文字垂直居中位置
    int buttonTextY = actualY + renderHeight - buttonHeight + (buttonHeight + fontHeight) / 2 - 2;
    
    // 左按钮 - 占据左半部分区域
    uint16_t leftBtnWidth = u8g2->getUTF8Width(leftButton.c_str());
    int leftButtonX = actualX + (renderWidth / 2 - leftBtnWidth) / 2;
    
    if (bLeftButtonHighLight) {
      // 高亮模式：填充整个左半部分按钮区域
      u8g2->setDrawColor(1);
      u8g2->drawRBox(actualX + 1, actualY + renderHeight - buttonHeight + 1, 
                     renderWidth / 2 - 1, buttonHeight - 2, 2);
      // 反色绘制文字
      u8g2->setFontMode(1);
      u8g2->setDrawColor(2);
      u8g2->drawUTF8(leftButtonX, buttonTextY, leftButton.c_str());
      u8g2->setFontMode(0);
      u8g2->setDrawColor(1);
    } else {
      // 正常模式
      u8g2->drawUTF8(leftButtonX, buttonTextY, leftButton.c_str());
    }

    // 右按钮 - 占据右半部分区域
    uint16_t rightBtnWidth = u8g2->getUTF8Width(rightButton.c_str());
    int rightButtonX = actualX + renderWidth / 2 + (renderWidth / 2 - rightBtnWidth) / 2;
    
    if (bRightButtonHighLight) {
      // 高亮模式：填充整个右半部分按钮区域
      u8g2->setDrawColor(1);
      u8g2->drawRBox(actualX + renderWidth / 2 + 1, actualY + renderHeight - buttonHeight + 1, 
                     renderWidth / 2 - 2, buttonHeight - 2, 2);
      // 反色绘制文字
      u8g2->setFontMode(1);
      u8g2->setDrawColor(2);
      u8g2->drawUTF8(rightButtonX, buttonTextY, rightButton.c_str());
      u8g2->setFontMode(0);
      u8g2->setDrawColor(1);
    } else {
      // 正常模式
      u8g2->drawUTF8(rightButtonX, buttonTextY, rightButton.c_str());
    }
  }

  // 绘制消息内容
  if (scale > 0.3f) { // 缩放太小时不显示文字
    u8g2->setFont(messageFont);
    int lineHeight = (int)((u8g2->getMaxCharHeight() + 2) * scale);
    int messageHeight = renderHeight - scaledPaddingTop - scaledPaddingBottom - titleHeight - buttonHeight;
    int maxLines = messageHeight / lineHeight;
    
    if (lineHeight > 0 && maxLines > 0) {
      // 简化的文本换行处理 - 超出宽度就换行
      int currentLine = 0;
      int startPos = 0;
      int msgLength = message.length();

      while (currentLine < maxLines && startPos < msgLength) {
        // 计算当前行能容纳的最大字符数
        int endPos = startPos;
        
        // 逐字符测试，找到能放入当前行的最后一个字符
        while (endPos < msgLength) {
          String testLine = message.substring(startPos, endPos + 1);
          uint16_t textWidth = u8g2->getUTF8Width(testLine.c_str());
          
          // 如果超出宽度，使用上一个位置作为结束
          if (textWidth > contentWidth) {
            break;
          }
          endPos++;
        }

        // 确保至少显示一个字符（即使超宽）
        if (endPos <= startPos) {
          endPos = startPos + 1;
        }

        // 绘制当前行
        String line = message.substring(startPos, endPos);
        u8g2->drawUTF8(contentX, messageY + lineHeight * (currentLine + 1), line.c_str());

        // 移动到下一行
        startPos = endPos;
        currentLine++;
      }
    }
  }

  // 恢复原始颜色
  u8g2->setDrawColor(oldColor);
}

void UIMessageBox::setMessage(String msg) {
  message = msg;
}

void UIMessageBox::setTitle(String title) {
  this->title = title;
}

void UIMessageBox::setButtons(String leftBtn, String rightBtn) {
  leftButton = leftBtn;
  rightButton = rightBtn;
  bShowButtons = true;
}

void UIMessageBox::show(MessageBoxAnimationType animType, unsigned long animDuration) {
  bVisible = true;
  showStartTime = millis();
  
  // 清理旧动画指针（不调用stop，避免访问已释放的内存）
  // AnimationEngine 会自动管理动画的生命周期
  currentAnimation = nullptr;
  
  // 创建新动画
  if (animType != MSGBOX_ANIME_NONE) {
    currentAnimation = new MessageBoxAnimation(this, animType, animDuration);
    
    // 设置动画完成回调，在动画完成时清空指针（避免野指针）
    currentAnimation->setOnComplete([this]() {
      currentAnimation = nullptr;
    });
    
    animationEngine.addAnimation(currentAnimation);
  }
}

void UIMessageBox::show() {
  bVisible = true;
  showStartTime = millis();
}

void UIMessageBox::hide(bool animated) {
  if (animated) {
    // 清理旧动画指针
    currentAnimation = nullptr;
    
    // 创建退出动画（默认使用缩小消失效果）
    currentAnimation = new MessageBoxAnimation(this, MSGBOX_ANIME_ZOOM_OUT, 200);
    
    // 设置动画完成回调，在动画完成后真正隐藏
    currentAnimation->setOnComplete([this]() {
      bVisible = false;
      autoCloseEnabled = false;
      currentAnimation = nullptr;
    });
    
    animationEngine.addAnimation(currentAnimation);
  } else {
    // 直接隐藏，无动画
    bVisible = false;
    autoCloseEnabled = false;
    
    // 清理动画指针（不访问，避免野指针）
    currentAnimation = nullptr;
  }
}

void UIMessageBox::hide(MessageBoxAnimationType animType, unsigned long animDuration) {
  // 清理旧动画指针
  currentAnimation = nullptr;
  
  // 创建退出动画
  currentAnimation = new MessageBoxAnimation(this, animType, animDuration);
  
  // 设置动画完成回调，在动画完成后真正隐藏
  currentAnimation->setOnComplete([this]() {
    bVisible = false;
    autoCloseEnabled = false;
    currentAnimation = nullptr;
  });
  
  animationEngine.addAnimation(currentAnimation);
}

bool UIMessageBox::isVisible() {
  return bVisible;
}

bool UIMessageBox::isAnimating() {
  return currentAnimation != nullptr && 
         currentAnimation->getState() == ANIME_STATE_RUNNING;
}

void UIMessageBox::setAutoClose(unsigned long delayMs) {
  autoCloseEnabled = true;
  autoCloseDelay = delayMs;
  showStartTime = millis();
}

void UIMessageBox::cancelAutoClose() {
  autoCloseEnabled = false;
}

void UIMessageBox::setPosition(int x, int y) {
  this->originalX = x;
  this->originalY = y;
  this->x = x;
  this->y = y;
  
  // 应用边界检查
  clampToScreen();
}

void UIMessageBox::setDefaultPosition(int screenWidth, int screenHeight, int navBarHeight) {
  // 保存屏幕尺寸
  this->screenWidth = screenWidth;
  this->screenHeight = screenHeight - navBarHeight; // 减去NavBar高度
  
  // 默认居中，但避开底部的NavBar区域
  int availableHeight = screenHeight - navBarHeight;
  
  if (originalX == -1) {
    originalX = (screenWidth - originalWidth) / 2;
    x = originalX;
  }
  
  if (originalY == -1) {
    originalY = (availableHeight - originalHeight) / 2;
    y = originalY;
  }
  
  // 应用边界检查
  clampToScreen();
}

void UIMessageBox::clampToScreen() {
  // 确保消息框不会超出屏幕边界
  
  // 检查左边界
  if (originalX < 0) {
    originalX = 0;
    x = originalX;
  }
  
  // 检查右边界
  if (originalX + originalWidth > screenWidth) {
    originalX = screenWidth - originalWidth;
    x = originalX;
    if (originalX < 0) { // 如果消息框比屏幕宽，至少保证左边对齐
      originalX = 0;
      x = 0;
    }
  }
  
  // 检查上边界
  if (originalY < 0) {
    originalY = 0;
    y = originalY;
  }
  
  // 检查下边界
  if (originalY + originalHeight > screenHeight) {
    originalY = screenHeight - originalHeight;
    y = originalY;
    if (originalY < 0) { // 如果消息框比屏幕高，至少保证顶部对齐
      originalY = 0;
      y = 0;
    }
  }
}

void UIMessageBox::showLeftButtonHighLight(std::function<void()> onComplete) {
  bLeftButtonHighLight = true;
  
  // 创建高亮动画并添加到动画引擎
  MessageBoxButtonAnimation* highlightAnim = new MessageBoxButtonAnimation(this, MSGBOX_BUTTON_LEFT, 300);
  if (onComplete) {
    highlightAnim->setOnComplete(onComplete);
  }
  animationEngine.addAnimation(highlightAnim);
}

void UIMessageBox::showRightButtonHighLight(std::function<void()> onComplete) {
  bRightButtonHighLight = true;
  
  // 创建高亮动画并添加到动画引擎
  MessageBoxButtonAnimation* highlightAnim = new MessageBoxButtonAnimation(this, MSGBOX_BUTTON_RIGHT, 300);
  if (onComplete) {
    highlightAnim->setOnComplete(onComplete);
  }
  animationEngine.addAnimation(highlightAnim);
}

