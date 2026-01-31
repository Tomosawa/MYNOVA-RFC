/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#include "UITextArea.h"

UITextArea::UITextArea() {
    scrollIndex = 0;
    visibleLines = 4; // 默认值，渲染时会根据高度重新计算
    lineHeight = 14;  // 增加默认行高，避免拥挤
    textFont = u8g2_font_wqy12_t_gb2312;
    needRecalculate = false;
    bVisible = true;
}

UITextArea::~UITextArea() {
    lines.clear();
}

void UITextArea::setText(String txt) {
    text = txt;
    scrollIndex = 0;
    needRecalculate = true;
}

void UITextArea::scrollUp() {
    if (scrollIndex > 0) {
        scrollIndex--;
    }
}

void UITextArea::scrollDown() {
    // 渲染时会更新 visibleLines，这里先做个安全检查
    if (lines.empty()) return;
    
    // 如果内容比可见区域少，就不滚动
    if ((int)lines.size() <= visibleLines) return;

    if (scrollIndex < (int)lines.size() - visibleLines) {
        scrollIndex++;
    }
}

// 辅助函数：判断是否是UTF-8多字节字符的后续字节
bool isUtf8Continuation(char c) {
    return (c & 0xC0) == 0x80;
}

void UITextArea::splitText(U8G2* u8g2) {
    lines.clear();
    
    if (text.length() == 0) return;
    
    u8g2->setFont(textFont);
    
    // 计算可用宽度：总宽度 - 边框(2) - 滚动条空间(6) - 左右padding(4)
    int contentWidth = width - 12; 
    if (contentWidth <= 0) contentWidth = 10; // 防止无效宽度

    String currentLine = "";
    int len = text.length();
    int i = 0;

    while (i < len) {
        // 提取下一个字符（处理UTF-8）
        String charStr = "";
        char c = text[i];
        uint8_t uc = (uint8_t)c;
        charStr += c;
        i++;

        // 如果是多字节字符的首字节
        int charLen = 1;
        if ((uc & 0xE0) == 0xC0) charLen = 2;
        else if ((uc & 0xF0) == 0xE0) charLen = 3;
        else if ((uc & 0xF8) == 0xF0) charLen = 4;
        
        // 读取剩余字节
        for (int k = 1; k < charLen && i < len; k++) {
            charStr += text[i++];
        }

        // 处理换行符
        if (c == '\n') {
            lines.push_back(currentLine);
            currentLine = "";
            continue;
        }

        // 尝试添加到当前行
        String testLine = currentLine + charStr;
        // 使用 getUTF8Width 替代 getStrWidth 以支持 UTF-8 字符宽度计算
        if (u8g2->getUTF8Width(testLine.c_str()) > contentWidth) {
            // 超出宽度，当前行结束
            // 只有当当前行不为空时才换行，避免死循环（理论上单个字符不会超过contentWidth，除非太窄）
            if (currentLine.length() > 0) {
                lines.push_back(currentLine);
                currentLine = charStr;
            } else {
                // 如果当前行是空的，但加这一个字符就超出了（说明这字符超级宽，或者contentWidth太小）
                // 强制放入一行
                lines.push_back(charStr);
                currentLine = "";
            }
        } else {
            currentLine = testLine;
        }
    }
    
    // 添加最后一行
    if (currentLine.length() > 0) {
        lines.push_back(currentLine);
    }
    
    needRecalculate = false;
}

void UITextArea::render(U8G2* u8g2, int offsetX, int offsetY) {
    if (!bVisible) return;
    
    int absX = x + offsetX;
    int absY = y + offsetY;
    
    // 动态计算可见行数
    // 高度 - 上下边框(2) - 上下padding(4)
    int availableHeight = height - 6;
    if (availableHeight < lineHeight) availableHeight = lineHeight;
    visibleLines = availableHeight / lineHeight;

    // 如果需要重新计算行（比如首次渲染或文本改变）
    // 或者宽度改变了（虽然这里没法直接检测宽度改变，但splitText依赖宽度）
    // 为了安全，每次render前确保splitText使用最新的宽度逻辑
    // 但splitText比较耗时，还是依赖 needRecalculate
    // 假设 width 不会频繁变
    if (needRecalculate) {
        splitText(u8g2);
    }
    
    // 绘制边框
    u8g2->drawFrame(absX, absY, width, height);
    
    // 设置字体
    u8g2->setFont(textFont);
    u8g2->setFontPosTop(); 
    
    // 内容区域
    // x: +2 (边框) + 2 (padding)
    // y: +2 (边框) + 2 (padding)
    int contentX = absX + 4;
    int contentY = absY + 3; //稍微下移一点，避免顶部切削
    
    // 设置裁剪区域，防止溢出边框
    // 范围：边框内部
    u8g2->setClipWindow(absX + 1, absY + 1, absX + width - 1, absY + height - 1);

    // 绘制文本行
    int count = 0;
    // 修正 scrollIndex 范围
    if (lines.size() > 0 && scrollIndex > (int)lines.size() - visibleLines) {
        scrollIndex = (int)lines.size() - visibleLines;
    }
    if (scrollIndex < 0) scrollIndex = 0;

    for (int i = scrollIndex; i < (int)lines.size(); i++) {
        if (count >= visibleLines) break;
        
        u8g2->drawUTF8(contentX, contentY + count * lineHeight, lines[i].c_str());
        count++;
    }
    
    // 恢复裁剪
    u8g2->setMaxClipWindow();
    u8g2->setFontPosBaseline(); // 恢复默认
    
    // 绘制滚动条
    int scrollBarX = absX + width - 6;
    int scrollBarY = absY + 2;
    int scrollBarHeight = height - 4;
    int scrollBarWidth = 4;
    
    // 绘制滚动槽
    u8g2->drawFrame(scrollBarX, scrollBarY, scrollBarWidth, scrollBarHeight);
    
    // 绘制滑块
    if (lines.size() > visibleLines) {
        // 滑块高度比例
        int sliderHeight = scrollBarHeight * visibleLines / lines.size();
        if (sliderHeight < 4) sliderHeight = 4; // 最小高度
        
        int maxScroll = lines.size() - visibleLines;
        // 计算滑块Y坐标
        // 避免除零
        int sliderY = scrollBarY;
        if (maxScroll > 0) {
             sliderY += (scrollIndex * (scrollBarHeight - sliderHeight) / maxScroll);
        }
        
        u8g2->drawBox(scrollBarX + 1, sliderY, scrollBarWidth - 2, sliderHeight);
    } else {
        // 内容不足一页，滑块充满
        u8g2->drawBox(scrollBarX + 1, scrollBarY, scrollBarWidth - 2, scrollBarHeight);
    }
}
