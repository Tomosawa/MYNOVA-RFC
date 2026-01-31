/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef MANAGEDATAPAGE_H
#define MANAGEDATAPAGE_H

#include "../GUI/UIPage.h"
#include "../GUI/Widget/UIMenu.h"
#include "../GUI/Widget/UIMessageBox.h"
#include "../GUI/Widget/UINavBar.h"

/**
 * ManageDataPage - 数据管理页面
 * 功能：选择数据位置 -> 选择操作(修改/删除/新建) -> 执行操作
 */
class ManageDataPage : public UIPage {
public:
    ManageDataPage();
    ~ManageDataPage();
    
    void render(U8G2* u8g2) override;
    void update();
    void showPage() override;  // 页面显示时刷新数据
    
    // 按钮事件处理
    void onButtonBack(void* context) override;
    void onButtonEnter(void* context) override;
    void onButtonMenu(void* context) override;
    void onButton1(void* context) override;
    void onButton2(void* context) override;
    void onButton3(void* context) override;
    void onButton4(void* context) override;
    void onButton5(void* context) override;
    void onButton6(void* context) override;
    void onButton7(void* context) override;
    void onButton8(void* context) override;
    void onButton9(void* context) override;
    
private:
    enum ManageState {
        STATE_SELECT_DATA,      // 选择数据位置
        STATE_SELECT_ACTION,    // 选择操作（修改/删除）
        STATE_DELETE_CONFIRM    // 删除确认
    };
    
    void initLayout();
    void showActionMenu(int dataIndex);
    void showDeleteConfirm();
    void deleteData();
    void editData();
    void createNewData();
    void refreshDataList();
    
    ManageState currentState;
    int selectedDataIndex;  // 当前选择的数据索引(1-100)
    bool hasData;           // 当前选择的位置是否有数据
    
    // UI组件
    UIMenu* dataListMenu;       // 数据列表菜单
    UIMenu* actionMenu;         // 操作菜单
    UIMessageBox* deleteConfirmBox;  // 删除确认框
    UINavBar* deleteNavBar;     // 删除确认的导航栏
};

#endif

