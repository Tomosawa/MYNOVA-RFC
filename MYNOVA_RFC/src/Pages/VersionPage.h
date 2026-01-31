/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// VersionPage.h
#ifndef VersionPage_h
#define VersionPage_h

#include "../GUI/UIPage.h"
#include "../GUI/Widget/UILabel.h"
#include "../GUI/Widget/UINavBar.h"
#include "../GUIRender.h"

class VersionPage : public UIPage {
public:
    VersionPage();
    
    // 重写按钮事件处理
    void onButtonBack(void* context = nullptr) override;
    
private:
    void initLayout(); // 初始化页面布局
    String convertDateFormat(const char* buildDate); // 转换日期格式
    
    UILabel *titleLabel;
    UILabel *versionLabel;
    UILabel *dateLabel;
    UILabel *serialLabel;
    UINavBar *navBar;
};

#endif

