/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef UIMenu_h
#define UIMenu_h

#include "UIWidget.h"
#include <Arduino.h>
#include <ArrayList.h>
#include <U8g2lib.h>
#include <vector>
#include "UINavBar.h"

class UIMenu : public UIWidget {
public:
  UIMenu(int x, int y, int width, int height, int menuLines);
  ~UIMenu() override;
  void render(U8G2* u8g2,int offsetX = 0, int offsetY = 0) override;
  int selectIndex();//获取当前选择的
  void addMenuItem(String item);
  void removeMenuItem(int iIndex);
  void moveUp();//向上移动光标
  void moveDown();//向下移动光标
  UINavBar* getNavBar();
public:
    bool bShowBtnTips = true;//是否底部显示按钮提示文字
    bool bShowTitle = true;//是否显示标题
    bool bShowBorder = false;//是否显示边框
    bool bShowScrollBar = true;//是否显示滚动条
    const uint8_t* titleFont = u8g2_font_wqy12_t_gb2312;//标题字体
    const uint8_t* menuFont = u8g2_font_wqy12_t_gb2312;
    //const uint8_t* buttonFont = u8g2_font_wqy12_t_gb2312;
    int menuLines = 5;//菜单总的行数(包含标题和按钮)
    int menuItemLines = 3;//菜单显示的行数(选择区域)
     //MENU页面相关
    int menuPos = 0;//菜单光标的位置(当前屏幕可见位置)
    int menuSel = 0;//菜单选择的第几个（菜单总数的第几个）
    int menuSelY = 0;//菜单选择的Y坐标
    int menuStart = 0;//菜单显示的开始
    int marginLeft = 3;//菜单距离左侧边缘距离
    int marginRight = 3;
    int menuLineHeight;
    //提示按钮文字
    // String leftBtnTip;
    // String middleBtnTip;
    // String rightBtnTip;
    String titleText;
private:
  ArrayList<String> items;
  UINavBar* navBar;
};

#endif
