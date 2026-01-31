/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// TetrisPage.cpp
#include "TetrisPage.h"
#include "../GUI/UIEngine.h"
#include "../ButtonDetector.h"
#include "clib/u8g2.h"

extern UIEngine uiEngine;

// 方块位图数据 - 用于装饰性显示
// I方块图案 (4x4)
static const uint8_t piece_I_icon[] PROGMEM = {
    0xF0, 0xF0, 0xF0, 0xF0
};

// O方块图案 (4x4)  
static const uint8_t piece_O_icon[] PROGMEM = {
    0x60, 0x60, 0x60, 0x60
};

// T方块图案 (4x4)
static const uint8_t piece_T_icon[] PROGMEM = {
    0xE0, 0x40, 0x00, 0x00
};

// 方块形状定义 - 使用16位表示4x4矩阵
// 每4位表示一行，从高位到低位
const uint16_t TetrisPage::PIECES[TETRIS_PIECE_COUNT][4] = {
    // I - 长条
    {0x0F00, 0x2222, 0x00F0, 0x4444},
    // O - 方块
    {0x6600, 0x6600, 0x6600, 0x6600},
    // T - T形
    {0x0E40, 0x4C40, 0x4E00, 0x4640},
    // S - S形
    {0x06C0, 0x8C40, 0x6C00, 0x4620},
    // Z - Z形
    {0x0C60, 0x4C80, 0xC600, 0x2640},
    // J - J形
    {0x0E80, 0xC440, 0x2E00, 0x44C0},
    // L - L形
    {0x0E20, 0x44C0, 0x8E00, 0xC440}
};

// 方块样式图案 - 每种方块有不同的填充样式
static const uint8_t cellPatterns[TETRIS_PIECE_COUNT][2] = {
    {0xFF, 0xFF},  // I - 实心
    {0xFF, 0xFF},  // O - 实心
    {0xAA, 0x55},  // T - 棋盘格
    {0xCC, 0xCC},  // S - 竖条纹
    {0xF0, 0xF0},  // Z - 横条纹
    {0xEE, 0xBB},  // J - 斜纹
    {0xDD, 0x77}   // L - 反斜纹
};

TetrisPage::TetrisPage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) {
    initGame();
}

TetrisPage::~TetrisPage() {
    // 退出游戏页面时，恢复普通按键响应模式
    ButtonDetector::setFastResponseMode(false);
    ButtonDetector::setLongPressEnabled(false);
}

void TetrisPage::showPage() {
    // 进入游戏页面时，启用快速按键响应模式和长按模式
    ButtonDetector::setFastResponseMode(true);
    ButtonDetector::setLongPressEnabled(true);
}

void TetrisPage::initGame() {
    gameState = START;
    score = 0;
    level = 1;
    linesCleared = 0;
    dropCounter = 0;
    dropInterval = 30;  // 初始下落速度
    frameCount = 0;
    flashCounter = 0;
    showGhostPiece = false;  // 默认不显示幽灵方块
    
    // 清空游戏板
    for (int y = 0; y < TETRIS_BOARD_HEIGHT; y++) {
        flashLine[y] = false;
        for (int x = 0; x < TETRIS_BOARD_WIDTH; x++) {
            board[y][x] = 0;
        }
    }
    
    // 生成第一个方块
    currentPiece = random(TETRIS_PIECE_COUNT);
    nextPiece = random(TETRIS_PIECE_COUNT);
    currentRotation = 0;
    currentX = TETRIS_BOARD_WIDTH / 2 - 2;
    currentY = 0;
}

void TetrisPage::render(U8G2* u8g2) {
    u8g2->clearBuffer();
    
    switch (gameState) {
        case START:
            drawStartScreen(u8g2);
            break;
        case PLAYING:
            drawBorder(u8g2);
            drawBoard(u8g2);
            if (showGhostPiece) {  // 只有开启时才显示幽灵方块
                drawGhostPiece(u8g2);
            }
            drawCurrentPiece(u8g2);
            drawNextPiece(u8g2);
            drawScore(u8g2);
            break;
        case PAUSED:
            drawBorder(u8g2);
            drawBoard(u8g2);
            drawCurrentPiece(u8g2);
            drawNextPiece(u8g2);
            drawScore(u8g2);
            drawPausedScreen(u8g2);
            break;
        case GAME_OVER:
            drawBorder(u8g2);
            drawBoard(u8g2);
            drawNextPiece(u8g2);
            drawScore(u8g2);
            drawGameOverScreen(u8g2);
            break;
    }
    
    u8g2->sendBuffer();
}

void TetrisPage::update() {
    frameCount++;
    
    if (gameState == PLAYING) {
        // 处理消行闪烁动画
        if (flashCounter > 0) {
            flashCounter--;
            if (flashCounter == 0) {
                clearLines();
            }
            return;
        }
        
        updateGame();
    }
}

void TetrisPage::updateGame() {
    dropCounter++;
    
    if (dropCounter >= dropInterval) {
        dropCounter = 0;
        
        // 尝试下移
        if (canMove(currentX, currentY + 1, currentRotation)) {
            currentY++;
        } else {
            // 不能下移，固定方块
            placePiece();
            checkLines();
            spawnNewPiece();
            
            // 检查游戏结束
            if (!canMove(currentX, currentY, currentRotation)) {
                gameState = GAME_OVER;
            }
        }
    }
}

bool TetrisPage::getPieceCell(int piece, int rotation, int x, int y) {
    if (x < 0 || x >= 4 || y < 0 || y >= 4) return false;
    uint16_t shape = PIECES[piece][rotation];
    int bit = (3 - y) * 4 + (3 - x);
    return (shape >> bit) & 1;
}

bool TetrisPage::canMove(int newX, int newY, int newRotation) {
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            if (getPieceCell(currentPiece, newRotation, px, py)) {
                int boardX = newX + px;
                int boardY = newY + py;
                
                // 检查边界
                if (boardX < 0 || boardX >= TETRIS_BOARD_WIDTH) return false;
                if (boardY >= TETRIS_BOARD_HEIGHT) return false;
                
                // 检查碰撞（只检查已在游戏区域内的部分）
                if (boardY >= 0 && board[boardY][boardX] != 0) return false;
            }
        }
    }
    return true;
}

void TetrisPage::placePiece() {
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            if (getPieceCell(currentPiece, currentRotation, px, py)) {
                int boardX = currentX + px;
                int boardY = currentY + py;
                if (boardY >= 0 && boardY < TETRIS_BOARD_HEIGHT && 
                    boardX >= 0 && boardX < TETRIS_BOARD_WIDTH) {
                    board[boardY][boardX] = currentPiece + 1;  // 存储方块类型(1-7)
                }
            }
        }
    }
}

void TetrisPage::checkLines() {
    bool hasFullLine = false;
    
    for (int y = 0; y < TETRIS_BOARD_HEIGHT; y++) {
        bool fullLine = true;
        for (int x = 0; x < TETRIS_BOARD_WIDTH; x++) {
            if (board[y][x] == 0) {
                fullLine = false;
                break;
            }
        }
        
        if (fullLine) {
            flashLine[y] = true;
            hasFullLine = true;
        }
    }
    
    if (hasFullLine) {
        flashCounter = 10;  // 闪烁10帧
    }
}

void TetrisPage::clearLines() {
    int clearedCount = 0;
    
    // 第一步：统计要消除的行数
    for (int y = 0; y < TETRIS_BOARD_HEIGHT; y++) {
        if (flashLine[y]) {
            clearedCount++;
        }
    }
    
    if (clearedCount == 0) return;
    
    // 第二步：从下往上处理，将未消除的行移动到新位置
    int writeY = TETRIS_BOARD_HEIGHT - 1; // 写入位置（从底部开始）
    
    for (int readY = TETRIS_BOARD_HEIGHT - 1; readY >= 0; readY--) {
        if (!flashLine[readY]) {
            // 这行不需要消除，复制到写入位置
            if (writeY != readY) {
                for (int x = 0; x < TETRIS_BOARD_WIDTH; x++) {
                    board[writeY][x] = board[readY][x];
                }
            }
            writeY--;
        } else {
            // 清除闪烁标记
            flashLine[readY] = false;
        }
    }
    
    // 第三步：清空顶部的空行
    for (int y = 0; y <= writeY; y++) {
        for (int x = 0; x < TETRIS_BOARD_WIDTH; x++) {
            board[y][x] = 0;
        }
    }
    
    // 计分
    if (clearedCount > 0) {
        // 经典计分: 1行=100, 2行=300, 3行=500, 4行=800
        int points[] = {0, 100, 300, 500, 800};
        score += points[clearedCount] * level;
        linesCleared += clearedCount;
        
        // 每10行升一级
        int newLevel = linesCleared / 10 + 1;
        if (newLevel > level) {
            level = newLevel;
            // 加快下落速度
            dropInterval = max(5, 30 - (level - 1) * 3);
        }
    }
}

void TetrisPage::spawnNewPiece() {
    currentPiece = nextPiece;
    nextPiece = random(TETRIS_PIECE_COUNT);
    currentRotation = 0;
    currentX = TETRIS_BOARD_WIDTH / 2 - 2;
    currentY = 0;
    dropCounter = 0;
}

void TetrisPage::rotatePiece() {
    int newRotation = (currentRotation + 1) % 4;
    
    // 尝试原地旋转
    if (canMove(currentX, currentY, newRotation)) {
        currentRotation = newRotation;
        return;
    }
    
    // 墙踢 - 尝试左移
    if (canMove(currentX - 1, currentY, newRotation)) {
        currentX--;
        currentRotation = newRotation;
        return;
    }
    
    // 墙踢 - 尝试右移
    if (canMove(currentX + 1, currentY, newRotation)) {
        currentX++;
        currentRotation = newRotation;
        return;
    }
    
    // 墙踢 - 尝试左移2格（I方块）
    if (canMove(currentX - 2, currentY, newRotation)) {
        currentX -= 2;
        currentRotation = newRotation;
        return;
    }
    
    // 墙踢 - 尝试右移2格（I方块）
    if (canMove(currentX + 2, currentY, newRotation)) {
        currentX += 2;
        currentRotation = newRotation;
        return;
    }
}

void TetrisPage::moveLeft() {
    if (canMove(currentX - 1, currentY, currentRotation)) {
        currentX--;
    }
}

void TetrisPage::moveRight() {
    if (canMove(currentX + 1, currentY, currentRotation)) {
        currentX++;
    }
}

void TetrisPage::moveDown() {
    if (canMove(currentX, currentY + 1, currentRotation)) {
        currentY++;
        score += 1;  // 软降加分
    }
}

void TetrisPage::hardDrop() {
    int dropDistance = 0;
    while (canMove(currentX, currentY + 1, currentRotation)) {
        currentY++;
        dropDistance++;
    }
    score += dropDistance * 2;  // 硬降加分
    
    placePiece();
    checkLines();
    spawnNewPiece();
    
    if (!canMove(currentX, currentY, currentRotation)) {
        gameState = GAME_OVER;
    }
}

void TetrisPage::drawCell(U8G2* u8g2, int x, int y, int type, bool isGhost) {
    int px = TETRIS_BOARD_X + x * TETRIS_CELL_SIZE;
    int py = TETRIS_BOARD_Y + y * TETRIS_CELL_SIZE;
    
    if (isGhost) {
        // 幽灵方块 - 只画边框
        u8g2->drawFrame(px, py, TETRIS_CELL_SIZE, TETRIS_CELL_SIZE);
    } else {
        // 实体方块 - 带样式填充
        u8g2->drawBox(px, py, TETRIS_CELL_SIZE, TETRIS_CELL_SIZE);
        // 添加高光效果（左上角像素留白）
        if (TETRIS_CELL_SIZE >= 3) {
            u8g2->setDrawColor(0);
            u8g2->drawPixel(px + TETRIS_CELL_SIZE - 1, py + TETRIS_CELL_SIZE - 1);
            u8g2->setDrawColor(1);
        }
    }
}

void TetrisPage::drawBoard(U8G2* u8g2) {
    for (int y = 0; y < TETRIS_BOARD_HEIGHT; y++) {
        // 消行闪烁效果
        if (flashLine[y] && (frameCount % 4 < 2)) {
            continue;  // 闪烁时跳过绘制
        }
        
        for (int x = 0; x < TETRIS_BOARD_WIDTH; x++) {
            if (board[y][x] != 0) {
                drawCell(u8g2, x, y, board[y][x], false);
            }
        }
    }
}

void TetrisPage::drawCurrentPiece(U8G2* u8g2) {
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            if (getPieceCell(currentPiece, currentRotation, px, py)) {
                int boardX = currentX + px;
                int boardY = currentY + py;
                if (boardY >= 0) {
                    drawCell(u8g2, boardX, boardY, currentPiece + 1, false);
                }
            }
        }
    }
}

void TetrisPage::drawGhostPiece(U8G2* u8g2) {
    // 计算幽灵方块位置
    int ghostY = currentY;
    while (canMove(currentX, ghostY + 1, currentRotation)) {
        ghostY++;
    }
    
    if (ghostY == currentY) return;  // 已经在底部
    
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            if (getPieceCell(currentPiece, currentRotation, px, py)) {
                int boardX = currentX + px;
                int boardY = ghostY + py;
                if (boardY >= 0) {
                    drawCell(u8g2, boardX, boardY, currentPiece + 1, true);
                }
            }
        }
    }
}

void TetrisPage::drawNextPiece(U8G2* u8g2) {
    // 在右侧显示下一个方块
    int previewX = TETRIS_BOARD_X + TETRIS_BOARD_WIDTH * TETRIS_CELL_SIZE + 8;
    int previewY = 12;
    
    // 绘制"NEXT"标签
    u8g2->setFont(u8g2_font_5x7_tf);
    u8g2->drawStr(previewX, 8, "NEXT");
    
    // 绘制预览框
    u8g2->drawFrame(previewX - 2, previewY - 2, 18, 18);
    
    // 绘制下一个方块
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            if (getPieceCell(nextPiece, 0, px, py)) {
                int drawX = previewX + px * 4;
                int drawY = previewY + py * 4;
                u8g2->drawBox(drawX, drawY, 3, 3);
            }
        }
    }
}

void TetrisPage::drawScore(U8G2* u8g2) {
    int infoX = TETRIS_BOARD_X + TETRIS_BOARD_WIDTH * TETRIS_CELL_SIZE + 8;
    
    u8g2->setFont(u8g2_font_5x7_tf);
    
    // 分数
    u8g2->drawStr(infoX, 38, "SCORE");
    char scoreStr[8];
    snprintf(scoreStr, sizeof(scoreStr), "%d", score);
    u8g2->drawStr(infoX, 46, scoreStr);
    
    // 等级
    u8g2->drawStr(infoX, 56, "LV:");
    char levelStr[4];
    snprintf(levelStr, sizeof(levelStr), "%d", level);
    u8g2->drawStr(infoX + 15, 56, levelStr);
}

void TetrisPage::drawBorder(U8G2* u8g2) {
    // 绘制游戏区域边框
    int borderX = TETRIS_BOARD_X - 1;
    int borderY = TETRIS_BOARD_Y - 1;
    int borderW = TETRIS_BOARD_WIDTH * TETRIS_CELL_SIZE + 2;
    int borderH = TETRIS_BOARD_HEIGHT * TETRIS_CELL_SIZE + 2;
    
    u8g2->drawFrame(borderX, borderY, borderW, borderH);
    
    // 绘制装饰性角落
    u8g2->drawPixel(borderX - 1, borderY - 1);
    u8g2->drawPixel(borderX + borderW, borderY - 1);
    u8g2->drawPixel(borderX - 1, borderY + borderH);
    u8g2->drawPixel(borderX + borderW, borderY + borderH);
}

void TetrisPage::drawStartScreen(U8G2* u8g2) {
    // 绘制标题
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    const char* title = "俄罗斯方块";
    int titleWidth = u8g2->getUTF8Width(title);
    u8g2->drawUTF8((SCREEN_WIDTH - titleWidth) / 2, 20, title);
    
    // 绘制装饰性方块图案
    int decorX = SCREEN_WIDTH / 2 - 12;
    int decorY = 28;
    
    // 绘制一个T形方块作为装饰
    u8g2->drawBox(decorX, decorY, 6, 6);
    u8g2->drawBox(decorX + 6, decorY, 6, 6);
    u8g2->drawBox(decorX + 12, decorY, 6, 6);
    u8g2->drawBox(decorX + 6, decorY + 6, 6, 6);
    
    // 绘制操作说明
    u8g2->setFont(u8g2_font_5x7_tf);
    const char* operationText = "4:L 6:R 5:ROT 8:DROP";
    int operationWidth = strlen(operationText) * 5;
    u8g2->drawStr((SCREEN_WIDTH - operationWidth) / 2, 45, operationText);
    
    // 绘制开始提示
    if (frameCount % 60 < 40) {
        u8g2->setFont(u8g2_font_wqy12_t_gb2312);
        const char* startText = "按确认键开始";
        int startWidth = u8g2->getUTF8Width(startText);
        u8g2->drawUTF8((SCREEN_WIDTH - startWidth) / 2, SCREEN_HEIGHT - 2, startText);
    }
}

void TetrisPage::drawGameOverScreen(U8G2* u8g2) {
    // 半透明遮罩效果（棋盘格）
    for (int y = 12; y < 52; y += 2) {
        for (int x = 20 + (y % 4) / 2; x < 108; x += 2) {
            u8g2->setDrawColor(0);
            u8g2->drawPixel(x, y);
        }
    }
    u8g2->setDrawColor(1);
    
    // 绘制游戏结束框
    u8g2->drawFrame(18, 10, 92, 44);
    u8g2->setDrawColor(0);
    u8g2->drawBox(19, 11, 90, 42);
    u8g2->setDrawColor(1);
    u8g2->drawFrame(20, 12, 88, 40);
    
    // 绘制"游戏结束"标题 - 中文字体高度12像素
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    const char* gameOverText = "GAME OVER";
    int textWidth = u8g2->getUTF8Width(gameOverText);
    u8g2->drawUTF8((SCREEN_WIDTH - textWidth) / 2, 26, gameOverText);
    
    // 显示最终分数 - 英文字体高度7像素，与上一行间隔10像素
    u8g2->setFont(u8g2_font_5x7_tf);
    char finalScore[20];
    snprintf(finalScore, sizeof(finalScore), "SCORE: %d", score);
    int scoreWidth = strlen(finalScore) * 5;
    u8g2->drawStr((SCREEN_WIDTH - scoreWidth) / 2, 38, finalScore);
    
    // 重新开始提示 - 与上一行间隔10像素
    if (frameCount % 60 < 40) {
        const char* restartText = "ENTER TO RESTART";
        int restartWidth = strlen(restartText) * 5;
        u8g2->drawStr((SCREEN_WIDTH - restartWidth) / 2, 48, restartText);
    }
}

void TetrisPage::drawPausedScreen(U8G2* u8g2) {
    // 绘制暂停框
    u8g2->setDrawColor(0);
    u8g2->drawBox(30, 22, 68, 20);
    u8g2->setDrawColor(1);
    u8g2->drawFrame(30, 22, 68, 20);
    
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    const char* pauseText = "暂停中";
    int textWidth = u8g2->getUTF8Width(pauseText);
    u8g2->drawUTF8((SCREEN_WIDTH - textWidth) / 2, 36, pauseText);
}

// 按钮处理
void TetrisPage::onButtonBack(void* context) {
    if (gameState == PLAYING) {
        gameState = PAUSED;
    } else if (gameState == PAUSED) {
        uiEngine.navigateBack();
    } else {
        uiEngine.navigateBack();
    }
}

void TetrisPage::onButtonEnter(void* context) {
    switch (gameState) {
        case START:
            initGame();
            gameState = PLAYING;
            break;
        case PLAYING:
            hardDrop();
            break;
        case PAUSED:
            gameState = PLAYING;
            break;
        case GAME_OVER:
            initGame();
            gameState = PLAYING;
            break;
    }
}

void TetrisPage::onButtonMenu(void* context) {
    if (gameState == PLAYING) {
        gameState = PAUSED;
    } else if (gameState == PAUSED) {
        gameState = PLAYING;
    }
}

// 数字键控制
void TetrisPage::onButton1(void* context) {
    // 1键 - 切换幽灵方块显示
    if (gameState == PLAYING) {
        showGhostPiece = !showGhostPiece;  // 开关切换
    }
}

void TetrisPage::onButton2(void* context) {
    // 2键 - 旋转
    if (gameState == PLAYING) {
        rotatePiece();
    }
}

void TetrisPage::onButton3(void* context) {
    // 3键 - 旋转
    if (gameState == PLAYING) {
        rotatePiece();
    }
}

void TetrisPage::onButton4(void* context) {
    // 4键 - 左移
    if (gameState == PLAYING) {
        moveLeft();
    }
}

void TetrisPage::onButton5(void* context) {
    // 5键 - 旋转
    if (gameState == PLAYING) {
        rotatePiece();
    }
}

void TetrisPage::onButton6(void* context) {
    // 6键 - 右移
    if (gameState == PLAYING) {
        moveRight();
    }
}

void TetrisPage::onButton7(void* context) {
    // 7键 - 软降
    if (gameState == PLAYING) {
        moveDown();
    }
}

void TetrisPage::onButton8(void* context) {
    // 8键 - 软降
    if (gameState == PLAYING) {
        moveDown();
    }
}

void TetrisPage::onButton9(void* context) {
    // 9键 - 硬降
    if (gameState == PLAYING) {
        hardDrop();
    }
}
