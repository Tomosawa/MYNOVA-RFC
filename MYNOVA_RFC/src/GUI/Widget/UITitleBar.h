/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef UITitleBar_h
#define UITitleBar_h

#include "UIWidget.h"
#include <Arduino.h>
#include <U8g2lib.h>

class UITitleBar : public UIWidget {
public:
  UITitleBar();
  void render(U8G2* u8g2,int offsetX = 0, int offsetY = 0) override;
  void showSendAnime();
  void showReciveAnime();
  void setBuzzerState(int state);
  void showAP(bool bShow);
  void showWifi(bool bShow);
public:
  int signalStrength;//0-4：一共五格
  int signalMode;//信号模式：0发送；1接收
  int buzzerState;//蜂鸣器：0静音；1发声
  int batteryLevel;//0-5:一共6档5格显示

private:
  String label;
  int animeCount;
  bool sendAnime = false;
  bool reciveAnime = false;
  bool bShowAP = false;
  bool bShowWifi = false;
};

#endif
