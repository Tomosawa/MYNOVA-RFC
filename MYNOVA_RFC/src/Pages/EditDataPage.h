/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef EDITDATAPAGE_H
#define EDITDATAPAGE_H

#include "../GUI/UIPage.h"
#include "../GUI/Widget/UILabel.h"
#include "../GUI/Widget/UINavBar.h"
#include "../GUI/Widget/UIInput.h"
#include "../GUI/Widget/UISelectValue.h"
#include "../GUI/Widget/UIEditableNumber.h"
#include "../DataStore.h"
#include <functional>

/**
 * EditDataPage - 编辑/新建数据页面
 * 用于编辑RadioData的所有字段
 */
class EditDataPage : public UIPage {
public:
    // dataIndex: 数据位置索引(1-100)，isNew: 是否是新建
    EditDataPage(int dataIndex, bool isNew);
    ~EditDataPage();
    
    void render(U8G2* u8g2) override;
    void update() override;
    
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
    enum EditState {
        EDIT_FIELDS,        // 编辑所有字段（一个屏幕显示）
        EDIT_NAME           // 编辑名称
    };
    
    enum FieldIndex {
        FIELD_FREQ = 0,     // 频率
        FIELD_PROTOCOL,     // 协议
        FIELD_BITLENGTH,    // 位长
        FIELD_PULSELENGTH,  // 脉冲宽度
        FIELD_DATA,         // 数据
        FIELD_COUNT         // 字段总数
    };
    
    void initLayout();
    void switchToNameEdit();
    void saveAndReturn();
    void updateFieldDisplay();
    void moveFieldUp();
    void moveFieldDown();
    String generateDefaultName();
    
    int dataIndex;
    bool isNewData;
    RadioData currentData;
    EditState currentState;
    int currentFieldIndex;  // 当前选中的字段索引
    
    // UI组件 - 字段编辑状态
    UILabel* statusLabel;           // 顶部状态标签
    UILabel* freqLabel;             // 频率标签（包含标题和值）
    UILabel* protocolLabel;         // 协议标签（包含标题和值）
    UILabel* bitLengthLabel;        // 位长标签（包含标题和值）
    UILabel* pulseLengthLabel;      // 脉宽标签（包含标题）
    UILabel* dataLabel;             // 数据标签（包含标题）
    
    UISelectValue* freqSelect;      // 频率选择器
    UISelectValue* protocolSelect;  // 协议选择器
    UISelectValue* bitLengthSelect; // 位长选择器
    UIEditableNumber* pulseLengthEdit;  // 脉宽编辑器
    UIEditableNumber* dataEdit;         // 数据编辑器
    
    UINavBar* navBar;
    
    // UI组件 - 名称编辑状态
    UIInput* nameInput;
    UINavBar* nameNavBar;
    
    // 选项数组
    static const int PROTOCOL_COUNT = 6;
    static const int BITLENGTH_COUNT = 10;
    const int bitLengthOptions[BITLENGTH_COUNT] = {4, 8, 12, 16, 20, 24, 28, 32, 36, 40};
    
    int currentFreqIndex;       // 0=315MHz, 1=433MHz
    int currentProtocolIndex;
    int currentBitLengthIndex;
    
    // 辅助函数：根据位长更新数据字段的范围
    void updateDataRangeByBitLength();
};

#endif

