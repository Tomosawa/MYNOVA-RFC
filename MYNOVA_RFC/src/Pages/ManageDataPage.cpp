/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "ManageDataPage.h"
#include "EditDataPage.h"
#include "../GUI/UIEngine.h"
#include "../GUI/Animation/MessageBoxAnimation.h"
#include "../DataStore.h"
#include "../GUIRender.h"

extern UIEngine uiEngine;
extern DataStore dataStore;

ManageDataPage::ManageDataPage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT),
    currentState(STATE_SELECT_DATA),
    selectedDataIndex(0),
    hasData(false) {
    initLayout();
}

ManageDataPage::~ManageDataPage() {
}

void ManageDataPage::initLayout() {
    // 初始化数据列表菜单
    dataListMenu = new UIMenu(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 5);
    dataListMenu->titleText = "选择数据位置";
    dataListMenu->bShowBtnTips = true;
    dataListMenu->bShowBorder = false;
    dataListMenu->bShowTitle = false;
    dataListMenu->bVisible = true;
    
    dataListMenu->getNavBar()->setLeftButtonText("返回");
    dataListMenu->getNavBar()->setRightButtonText("选择");
    
    // 加载所有数据位置
    for (int i = 1; i <= 100; i++) {
        String itemName = String(i) + ". ";
        RadioData data = dataStore.ReadData(i);
        if (data.name.length() > 0) {
            itemName += data.name;
        } else {
            itemName += "----------";
        }
        dataListMenu->addMenuItem(itemName);
    }
    addWidget(dataListMenu);
    
    // 初始化操作菜单
    actionMenu = new UIMenu(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 5);
    actionMenu->titleText = "选择操作";
    actionMenu->bShowBtnTips = true;
    actionMenu->bShowBorder = false;
    actionMenu->bShowTitle = false;
    actionMenu->bVisible = false;
    
    actionMenu->getNavBar()->setLeftButtonText("返回");
    actionMenu->getNavBar()->setRightButtonText("确认");
    
    actionMenu->addMenuItem("1. 修改数据");
    actionMenu->addMenuItem("2. 删除数据");
    addWidget(actionMenu);
    
    // 初始化删除确认框
    deleteConfirmBox = new UIMessageBox(-1, -1, 100, 50);
    deleteConfirmBox->setDefaultPosition(SCREEN_WIDTH, SCREEN_HEIGHT, 15);
    deleteConfirmBox->setMessage("确认删除该数据？");
    deleteConfirmBox->setTitle("删除确认");
    deleteConfirmBox->bShowButtons = false;
    addWidget(deleteConfirmBox);
    
    // 删除确认的导航栏
    deleteNavBar = new UINavBar(0, SCREEN_HEIGHT - 15, SCREEN_WIDTH, 15);
    deleteNavBar->setLeftButtonText("取消");
    deleteNavBar->setRightButtonText("删除");
    deleteNavBar->bVisible = false;
    addWidget(deleteNavBar);
}

void ManageDataPage::showActionMenu(int dataIndex) {
    selectedDataIndex = dataIndex;
    
    // 检查该位置是否有数据
    RadioData data = dataStore.ReadData(dataIndex);
    hasData = (data.name.length() > 0);
    
    if (hasData) {
        // 有数据，显示操作菜单
        dataListMenu->bVisible = false;
        actionMenu->bVisible = true;
        currentState = STATE_SELECT_ACTION;
    } else {
        // 没有数据，直接进入新建
        createNewData();
    }
}

void ManageDataPage::showDeleteConfirm() {
    actionMenu->bVisible = false;
    deleteConfirmBox->show(MSGBOX_ANIME_ZOOM_CENTER, 300);
    deleteNavBar->bVisible = true;
    currentState = STATE_DELETE_CONFIRM;
}

void ManageDataPage::deleteData() {
    // 创建空数据来覆盖
    RadioData emptyData;
    emptyData.name = "";
    emptyData.rcData.data = 0;
    emptyData.rcData.bitLength = 0;
    emptyData.rcData.protocal = 0;
    emptyData.rcData.pulseLength = 0;
    emptyData.rcData.freqType = FREQ_315;
    
    dataStore.SaveData(selectedDataIndex, emptyData);
    
    // 刷新数据列表
    refreshDataList();
    
    // 隐藏确认框
    deleteConfirmBox->hide();
    deleteNavBar->bVisible = false;
    
    // 返回数据列表
    dataListMenu->bVisible = true;
    currentState = STATE_SELECT_DATA;
}

void ManageDataPage::editData() {
    // 跳转到编辑页面
    EditDataPage* editPage = new EditDataPage(selectedDataIndex, false);
    uiEngine.navigateTo(editPage);
}

void ManageDataPage::createNewData() {
    // 跳转到新建页面
    EditDataPage* editPage = new EditDataPage(selectedDataIndex, true);
    uiEngine.navigateTo(editPage);
}

void ManageDataPage::refreshDataList() {
    // 清空并重新加载数据列表
    // 删除所有现有菜单项
    for (int i = 99; i >= 0; i--) {
        dataListMenu->removeMenuItem(i);
    }
    
    // 重新加载所有数据
    for (int i = 1; i <= 100; i++) {
        String itemName = String(i) + ". ";
        RadioData data = dataStore.ReadData(i);
        if (data.name.length() > 0) {
            itemName += data.name;
        } else {
            itemName += "----------";
        }
        dataListMenu->addMenuItem(itemName);
    }
}

void ManageDataPage::render(U8G2* u8g2) {
    UIPage::render(u8g2);
    update();
}

void ManageDataPage::update() {
    deleteConfirmBox->update();
}

void ManageDataPage::showPage() {
    UIPage::showPage();
    // 页面显示时刷新数据列表
    refreshDataList();
}

void ManageDataPage::onButtonBack(void* context) {
    switch (currentState) {
        case STATE_SELECT_DATA:
            dataListMenu->getNavBar()->showLeftBlink(1, 80, 80, [this]() {
                uiEngine.navigateBack();
            });
            break;
        case STATE_SELECT_ACTION:
            actionMenu->getNavBar()->showLeftBlink(1, 80, 80, [this]() {
                actionMenu->bVisible = false;
                dataListMenu->bVisible = true;
                currentState = STATE_SELECT_DATA;
            });
            break;
        case STATE_DELETE_CONFIRM:
            deleteNavBar->showLeftBlink(1, 80, 80, [this]() {
                deleteConfirmBox->hide();
                deleteNavBar->bVisible = false;
                actionMenu->bVisible = true;
                currentState = STATE_SELECT_ACTION;
            });
            break;
    }
}

void ManageDataPage::onButtonEnter(void* context) {
    switch (currentState) {
        case STATE_SELECT_DATA: {
            int selectedIndex = dataListMenu->menuSel + 1;
            dataListMenu->getNavBar()->showRightBlink(1, 80, 80, [this, selectedIndex]() {
                showActionMenu(selectedIndex);
            });
            break;
        }
        case STATE_SELECT_ACTION: {
            int actionIndex = actionMenu->menuSel;
            actionMenu->getNavBar()->showRightBlink(1, 80, 80, [this, actionIndex]() {
                if (actionIndex == 0) {
                    // 修改数据
                    editData();
                } else if (actionIndex == 1) {
                    // 删除数据
                    showDeleteConfirm();
                }
            });
            break;
        }
        case STATE_DELETE_CONFIRM:
            deleteNavBar->showRightBlink(1, 80, 80, [this]() {
                deleteData();
            });
            break;
    }
}

void ManageDataPage::onButtonMenu(void* context) {
    // Menu键可用于在数据列表中向下移动
    if (currentState == STATE_SELECT_DATA) {
        dataListMenu->moveDown();
    }
}

void ManageDataPage::onButton1(void* context) {
    if (currentState == STATE_SELECT_DATA) {
        dataListMenu->moveUp();
    } else if (currentState == STATE_SELECT_ACTION) {
        actionMenu->moveUp();
    }
}

void ManageDataPage::onButton2(void* context) {
    if (currentState == STATE_SELECT_DATA) {
        dataListMenu->moveUp();
    } else if (currentState == STATE_SELECT_ACTION) {
        actionMenu->moveUp();
    }
}

void ManageDataPage::onButton3(void* context) {
}

void ManageDataPage::onButton4(void* context) {
}

void ManageDataPage::onButton5(void* context) {
    onButtonEnter(context);
}

void ManageDataPage::onButton6(void* context) {
}

void ManageDataPage::onButton7(void* context) {
}

void ManageDataPage::onButton8(void* context) {
    if (currentState == STATE_SELECT_DATA) {
        dataListMenu->moveDown();
    } else if (currentState == STATE_SELECT_ACTION) {
        actionMenu->moveDown();
    }
}

void ManageDataPage::onButton9(void* context) {
}

