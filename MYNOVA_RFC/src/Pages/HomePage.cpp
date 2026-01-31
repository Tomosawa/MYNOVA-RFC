/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// HomePage.cpp
#include "HomePage.h"
#include "../GUI/UIEngine.h"
#include "../Pages/MenuPage.h"
#include "../RadioHelper.h"
#include "../DataStore.h"
#include "../SystemSetting.h"

extern UIEngine uiEngine;
extern RadioHelper radioHelper;
extern DataStore dataStore;
extern SystemSetting systemSetting;

HomePage::HomePage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) {
    initLayout();
    updateStatus();
}

void HomePage::showPage() {
    Serial.println("HomePage: showPage() 被调用");
    updateStatus();
    Serial.println("HomePage: showPage() 执行完成");
}

void HomePage::initLayout() {
    //构建首页HOME
    //初始化标题栏
    titleBar.x = 0;
    titleBar.y = 0;
    titleBar.width = SCREEN_WIDTH;
    titleBar.height = 10;
    titleBar.showSendAnime();
    
    addWidget(&titleBar);
    
    //初始化9个按钮
    quickButtons[0].x = 0;
    quickButtons[0].y = 10;
    quickButtons[0].width = 42;
    quickButtons[0].height = 17;
    quickButtons[0].keyNum = "1";
    quickButtons[0].label = "----------";
    addWidget(&quickButtons[0]);


    quickButtons[1].x = 43;
    quickButtons[1].y = 10;
    quickButtons[1].width = 42;
    quickButtons[1].height = 17;
    quickButtons[1].keyNum = "2";
    quickButtons[1].label = "----------";
    addWidget(&quickButtons[1]);

    quickButtons[2].x = 86;
    quickButtons[2].y = 10;
    quickButtons[2].width = 42;
    quickButtons[2].height = 17;
    quickButtons[2].keyNum = "3";
    quickButtons[2].label = "----------";
    addWidget(&quickButtons[2]);

    quickButtons[3].x = 0;
    quickButtons[3].y = 28;
    quickButtons[3].width = 42;
    quickButtons[3].height = 17;
    quickButtons[3].keyNum = "4";
    quickButtons[3].label = "----------";
    addWidget(&quickButtons[3]);

    quickButtons[4].x = 43;
    quickButtons[4].y = 28;
    quickButtons[4].width = 42;
    quickButtons[4].height = 17;
    quickButtons[4].keyNum = "5";
    quickButtons[4].label = "----------";
    addWidget(&quickButtons[4]);

    quickButtons[5].x = 86;
    quickButtons[5].y = 28;
    quickButtons[5].width = 42;
    quickButtons[5].height = 17;
    quickButtons[5].keyNum = "6";
    quickButtons[5].label = "----------";
    addWidget(&quickButtons[5]);

    quickButtons[6].x = 0;
    quickButtons[6].y = 46;
    quickButtons[6].width = 42;
    quickButtons[6].height = 17;
    quickButtons[6].keyNum = "7";
    quickButtons[6].label = "----------";
    addWidget(&quickButtons[6]);

    quickButtons[7].x = 43;
    quickButtons[7].y = 46;
    quickButtons[7].width = 42;
    quickButtons[7].height = 17;
    quickButtons[7].keyNum = "8";
    quickButtons[7].label = "----------";
    addWidget(&quickButtons[7]);

    quickButtons[8].x = 86;
    quickButtons[8].y = 46;
    quickButtons[8].width = 42;
    quickButtons[8].height = 17;
    quickButtons[8].keyNum = "9";
    quickButtons[8].label = "----------";
    addWidget(&quickButtons[8]);
}

void HomePage::updateStatus() {
    // 从 Flash 加载快捷键数据到缓存
    loadQuickKeyData();

    // 更新蜂鸣器状态
    titleBar.setBuzzerState(systemSetting.getBuzzerEnable() ? 1 : 0);
    
    // 更新AP状态
    SystemConfig config = systemSetting.getConfig();
 
    if (config.WifiEnabled)
        titleBar.showWifi(true);
    else
        titleBar.showWifi(false);

    if (config.APEnabled)
        titleBar.showAP(true);
    else
        titleBar.showAP(false);

   
}

// 从 Flash 加载快捷键数据到缓存
void HomePage::loadQuickKeyData() {
    quickKey = dataStore.LoadQuickKey();
    
    // 快捷键索引数组，方便循环处理
    int keyIndices[9] = {
        quickKey.key1, quickKey.key2, quickKey.key3,
        quickKey.key4, quickKey.key5, quickKey.key6,
        quickKey.key7, quickKey.key8, quickKey.key9
    };
    
    // 加载数据并更新按钮标签
    for (int i = 0; i < 9; i++) {
        if (keyIndices[i] > 0) {
            cachedRadioData[i] = dataStore.ReadData(keyIndices[i]);
            if (cachedRadioData[i].name.length() > 0) {
                quickButtons[i].label = cachedRadioData[i].name;
            } else {
                quickButtons[i].label = "----------";
                cachedRadioData[i].name = "";  // 清空无效数据
            }
        } else {
            quickButtons[i].label = "----------";
            cachedRadioData[i].name = "";  // 标记为空
        }
    }
    
    Serial.println("HomePage: 快捷键数据已从Flash加载到缓存");
}

// 发送缓存的数据（buttonIndex: 0-8）
bool HomePage::sendCachedData(int buttonIndex) {
    // 防抖：检查距离上次发送是否已经过了足够的时间
    unsigned long currentTime = millis();
    if (currentTime - lastSendTime < SEND_DEBOUNCE_MS) {
        Serial.println("HomePage::sendCachedData() 防抖拦截");
        return false;
    }
    
    // 检查索引有效性
    if (buttonIndex < 0 || buttonIndex >= 9) {
        return false;
    }
    
    // 直接使用缓存的数据
    if (cachedRadioData[buttonIndex].name.length() > 0) {
        lastSendTime = currentTime;  // 更新上次发送时间
        radioHelper.SendData(cachedRadioData[buttonIndex].rcData);
        // 显示发送动画
        titleBar.showSendAnime();
        return true;
    }
    return false;
}

void HomePage::onButtonMenu(void* context) {
    uiEngine.navigateTo(new MenuPage(), ANIME_SLIDE_IN_UP);
}

void HomePage::onButton1(void* context) {
    if(sendCachedData(0))
        quickButtons[0].showBlink(3, 100, 100);
}

void HomePage::onButton2(void* context) {
    if(sendCachedData(1))
        quickButtons[1].showBlink(3, 100, 100);
}

void HomePage::onButton3(void* context) {
    if(sendCachedData(2))
        quickButtons[2].showBlink(3, 100, 100);
}

void HomePage::onButton4(void* context) {
    if(sendCachedData(3))
        quickButtons[3].showBlink(3, 100, 100);
}

void HomePage::onButton5(void* context) {
    if(sendCachedData(4))
        quickButtons[4].showBlink(3, 100, 100);
}

void HomePage::onButton6(void* context) {
    if(sendCachedData(5))
        quickButtons[5].showBlink(3, 100, 100);
}

void HomePage::onButton7(void* context) {
    if(sendCachedData(6))
        quickButtons[6].showBlink(3, 100, 100);
}

void HomePage::onButton8(void* context) {
    if(sendCachedData(7))
        quickButtons[7].showBlink(3, 100, 100);
}

void HomePage::onButton9(void* context) {
    if(sendCachedData(8))
        quickButtons[8].showBlink(3, 100, 100);
}

void HomePage::setBattery(float voltage)
{
    if(voltage >= 4.0)
        titleBar.batteryLevel = 5;//5满电
    else if(voltage >= 3.8 && voltage < 4.0)
        titleBar.batteryLevel = 4;//4格
    else if(voltage >= 3.6 && voltage < 3.8)
        titleBar.batteryLevel = 3;//3格
    else if(voltage >= 3.4 && voltage < 3.6)
        titleBar.batteryLevel = 2;//2格
    else if(voltage >= 3.2 && voltage < 3.4)
        titleBar.batteryLevel = 1;//1格
    else if(voltage < 3.2)
        titleBar.batteryLevel = 0;//0格
}