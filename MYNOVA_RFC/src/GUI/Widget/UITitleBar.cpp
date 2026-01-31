/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "UITitleBar.h"
#include <U8g2lib.h>

static unsigned char ant_bits[] = {
   0x00, 0x00, 0xfe, 0x03, 0x24, 0x01, 0xa8, 0x00, 0x70, 0x00, 0x20, 0x00,
   0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x00, 0x00 };

UITitleBar::UITitleBar() {
  // 初始化按钮的标签
  height = 10;
  width = 128;
}

void UITitleBar::render(U8G2* u8g2,int offsetX, int offsetY) {
    // 检查是否可见
    if (!bVisible) return;
    
    u8g2->setFont(u8g2_font_waffle_t_all);
    int drawX = margin_left + x + padding_left;
    int drawY = margin_top + y + padding_top;
    //天线图标
    u8g2->drawXBM(drawX, drawY, 10, 10, ant_bits);
    drawX += 6;
    //信号图标
    switch (signalStrength)
    {
    case 0:
        u8g2->drawGlyph(drawX, drawY + 10, 57944);//1格信号
        break;
    case 1:
        u8g2->drawGlyph(drawX, drawY + 10, 57945);//2格信号
        break;
    case 2:
        u8g2->drawGlyph(drawX, drawY + 10, 57946);//3格信号
        break;
    case 3:
        u8g2->drawGlyph(drawX, drawY + 10, 57947);//4格信号
        break;
    case 4:
        u8g2->drawGlyph(drawX, drawY + 10, 57948);//5格信号
        break;
    }
    if(sendAnime)
    {
        animeCount--;
        signalStrength++;
        if(signalStrength > 4)
            signalStrength = 0;
        if(animeCount <= 0)
            sendAnime = false;
    }
    if(reciveAnime)
    {
        animeCount--;
        signalStrength--;
        if(signalStrength < 0)
            signalStrength = 4;
        if(animeCount <= 0)
            reciveAnime = false;
    }
    drawX += 10;
    if(signalMode == 0)
    { //发送模式图标
        u8g2->drawGlyph(drawX, drawY + 9, 57685);//发送
    }
    else
    {//接收模式图标
        u8g2->drawGlyph(drawX, drawY + 9, 57686);//接收
    }


    //电池电量
    u8g2->setFont(u8g2_font_battery19_tn);
    u8g2->setFontDirection(3);
    drawX = width - margin_right - padding_right;
    drawY = 8;
    switch (batteryLevel)
    {
      case 0:
          u8g2->drawGlyph(drawX, drawY, 48);//0格
          break;
      case 1:
          u8g2->drawGlyph(drawX, drawY, 49);//1格
          break;
      case 2:
          u8g2->drawGlyph(drawX, drawY, 50);//2格
          break;
      case 3:
          u8g2->drawGlyph(drawX, drawY, 51);//3格
          break;
      case 4:
          u8g2->drawGlyph(drawX, drawY, 52);//4格
          break;
      case 5:
          u8g2->drawGlyph(drawX, drawY, 53);//5满电
          break;
    }
     //恢复字体方向
    u8g2->setFontDirection(0);

    drawX -= 30;
    u8g2->setFont(u8g2_font_waffle_t_all);
    //蜂鸣器
    if(buzzerState == 0)
    {
        u8g2->drawGlyph(drawX, drawY, 57858);//静音
    }else
    {
        u8g2->drawGlyph(drawX, drawY, 57859);//开启
    }
   
    drawX -= 12;
    //WIFI模式图标
    if(bShowWifi)
    {
        u8g2->drawGlyph(drawX, drawY, 57882);//WIFI
    }

    drawX -= 12;
    //AP模式图标
    if(bShowAP)
    {
        u8g2->drawGlyph(drawX, drawY, 57389);//AP
    }
}

void UITitleBar::showSendAnime()
{
    animeCount = 25;
    signalStrength = 0;
    sendAnime = true;
}

void UITitleBar::showReciveAnime()
{
    animeCount = 25;
    signalStrength = 4;
    reciveAnime = true;
}

void UITitleBar::setBuzzerState(int state)
{
    buzzerState = state;
}

void UITitleBar::showAP(bool bShow)
{
    bShowAP = bShow;
}

void UITitleBar::showWifi(bool bShow)
{
    bShowWifi = bShow;
}