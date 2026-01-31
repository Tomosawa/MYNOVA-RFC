/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "UIMenu.h"
#include <U8g2lib.h>
#include "src/GUI/Animation/MenuCursorAnimation.h"
#include "src/GUI/Animation/AnimationEngine.h"
UIMenu::UIMenu(int x, int y, int width, int height,int menuLines)
{
    // 初始化菜单项列表
    this->x = x;
    this->y = y;
    this->height = height;
    this->width = width;
    this->menuLines = menuLines;
    
    menuPos = 0;//菜单光标的位置
    menuSel = 0;//菜单选择的第几个
    menuStart = 0;//菜单显示的开始
    menuSelY = 0;
    marginLeft = 3;//菜单距离左侧边缘距离
    marginRight = 3;
    menuLineHeight = round(height * 1.0 / menuLines);

    // 初始化导航栏
    int navBarHeight = menuLineHeight;
    navBar = new UINavBar(x, y + height - navBarHeight, width, navBarHeight);
    navBar->setMargin(marginLeft, marginRight);
}

UIMenu::~UIMenu()
{
    delete navBar;
}

int UIMenu::selectIndex()
{
    return menuSel;
}

void UIMenu::addMenuItem(String item)
{
    items.add(item);
}

void UIMenu::removeMenuItem(int iIndex)
{
    items.remove(iIndex);
}

void UIMenu::moveUp()
{
    if(menuPos > 0)
    {
        menuPos--;
    }
    else if(menuStart > 0)
    {
        menuStart--;
    }
    else
    {
        menuStart = items.size() - menuItemLines;
        menuPos = menuItemLines - 1;
    }
     // 添加往上移动动画
    MenuCursorAnimation* anim = new MenuCursorAnimation(this, menuSelY, menuPos * menuLineHeight, 100);
    animationEngine.addAnimation(anim);
}

void UIMenu::moveDown()
{
    if(menuPos + 1 < menuItemLines)
    {     
        menuPos++;
    }
    else if(menuStart + menuItemLines < items.size())
    {
        menuStart++;
    }
    else
    {
        menuStart = 0;
        menuPos = 0;
    }

    // 添加往下移动动画
    MenuCursorAnimation* anim = new MenuCursorAnimation(this, menuSelY, menuPos * menuLineHeight, 100);
    animationEngine.addAnimation(anim);
}

// 获取导航栏实例
UINavBar* UIMenu::getNavBar() {
    return navBar;
}

void UIMenu::render(U8G2* u8g2,int offsetX, int offsetY) {
    // 检查是否可见
    if (!bVisible) return;
    
    //清除背景，绘制为黑色
    // 保存当前颜色
    uint8_t oldColor = u8g2->getDrawColor();
    // 设置绘制颜色为 0（黑色，即关闭像素）
    u8g2->setDrawColor(0);
    // 绘制填充矩形
    u8g2->drawBox(offsetX + x, offsetY + y, width, height);
    // 恢复原来的颜色（通常是 1，白色/点亮）
    u8g2->setDrawColor(oldColor);

    //绘制边框
    if(bShowBorder)
    {
        u8g2->drawFrame(offsetX + x, offsetY + y, width, height);
        if(bShowTitle)
            u8g2->drawLine(offsetX + x, offsetY + y + menuLineHeight, x + width, y + menuLineHeight);
        if(bShowBtnTips)
            u8g2->drawLine(offsetX + x, offsetY + y + height - menuLineHeight, x + width, y + height - menuLineHeight);
    }
    //绘制标题
    if(bShowTitle)
    {
        u8g2->setFont(titleFont);
        int8_t fontHeight = u8g2->getMaxCharHeight();
        uint16_t strWidth = u8g2->getUTF8Width(titleText.c_str());
        u8g2->drawUTF8(offsetX + x + (width / 2 - strWidth / 2) , offsetY + y + menuLineHeight / 2 + fontHeight / 2, titleText.c_str());
    }
    
    int itemStartY = y;
    if(bShowTitle)
        itemStartY += menuLineHeight;
    //绘制选中光标
 
    u8g2->setFontMode(1);  /* activate transparent font mode */
    u8g2->setDrawColor(1); /* color 1 for the box */
    u8g2->drawBox(offsetX + x, offsetY + itemStartY + 1 + menuSelY, width, menuLineHeight - 1);
    u8g2->setDrawColor(2);
    //绘制菜单文字
    u8g2->setFont(menuFont);
    //计算文字垂直居中位置
    int strLineOffset = 0;
    int8_t maxCharHeight = u8g2->getMaxCharHeight();
    strLineOffset = menuLineHeight / 2 + maxCharHeight / 2 - 1;
    menuItemLines = menuLines;
    if(bShowTitle)
        menuItemLines--;
    if(bShowBtnTips)
        menuItemLines--;
    for(int i = 0; i < menuItemLines; i++)
    {
         u8g2->drawUTF8(offsetX + x + marginLeft, offsetY + y + itemStartY + menuLineHeight * i + strLineOffset, items.get(menuStart + i).c_str());
    }
    
    u8g2->setFontMode(0);
    u8g2->setDrawColor(1);
    //绘制按钮提示
    if(bShowBtnTips)
    {
       navBar->render(u8g2, offsetX, offsetY);
    }

    //选择的菜单的索引
    menuSel = menuStart + menuPos;
}
