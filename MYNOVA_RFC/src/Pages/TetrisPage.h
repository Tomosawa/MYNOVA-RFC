/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// TetrisPage.h
#ifndef TetrisPage_h
#define TetrisPage_h

#include "../GUI/UIPage.h"
#include "../GUIRender.h"
#include <U8g2lib.h>

// 游戏区域配置
#define TETRIS_BOARD_WIDTH  10   // 游戏区域宽度（格子数）
#define TETRIS_BOARD_HEIGHT 20   // 游戏区域高度（格子数）
#define TETRIS_CELL_SIZE    3    // 每个格子的像素大小
#define TETRIS_BOARD_X      34   // 游戏区域起始X坐标（居中：(128-10*3)/2 = 49，但留空间给左侧信息，所以用34）
#define TETRIS_BOARD_Y      2    // 游戏区域起始Y坐标

// 方块类型数量
#define TETRIS_PIECE_COUNT  7

class TetrisPage : public UIPage {
public:
    TetrisPage();
    ~TetrisPage();
    
    // 重写页面显示函数
    void showPage() override;
    
    // 重写渲染和更新函数
    void render(U8G2* u8g2) override;
    void update() override;
    
    // 重写按钮事件处理
    void onButtonBack(void* context = nullptr) override;
    void onButtonEnter(void* context = nullptr) override;
    void onButtonMenu(void* context = nullptr) override;
    void onButton1(void* context = nullptr) override;
    void onButton2(void* context = nullptr) override;
    void onButton3(void* context = nullptr) override;
    void onButton4(void* context = nullptr) override;
    void onButton5(void* context = nullptr) override;
    void onButton6(void* context = nullptr) override;
    void onButton7(void* context = nullptr) override;
    void onButton8(void* context = nullptr) override;
    void onButton9(void* context = nullptr) override;
    
private:
    // 游戏状态
    enum GameState {
        START,
        PLAYING,
        PAUSED,
        GAME_OVER
    };
    
    GameState gameState;
    
    // 方块形状定义 (4x4矩阵，每种方块4个旋转状态)
    // I, O, T, S, Z, J, L
    static const uint16_t PIECES[TETRIS_PIECE_COUNT][4];
    
    // 游戏板 (0=空, 1-7=不同方块类型)
    uint8_t board[TETRIS_BOARD_HEIGHT][TETRIS_BOARD_WIDTH];
    
    // 当前方块
    int currentPiece;      // 当前方块类型 (0-6)
    int currentRotation;   // 当前旋转状态 (0-3)
    int currentX;          // 当前X位置
    int currentY;          // 当前Y位置
    
    // 下一个方块
    int nextPiece;
    
    // 游戏数据
    int score;
    int level;
    int linesCleared;
    int dropCounter;
    int dropInterval;      // 下落间隔（帧数）
    
    // 动画相关
    int frameCount;
    bool flashLine[TETRIS_BOARD_HEIGHT];  // 消除行闪烁效果
    int flashCounter;
    
    // 显示设置
    bool showGhostPiece;  // 是否显示幽灵方块（预览位置）
    
    // 游戏初始化
    void initGame();
    
    // 游戏逻辑
    void updateGame();
    bool canMove(int newX, int newY, int newRotation);
    void placePiece();
    void checkLines();
    void clearLines();
    void spawnNewPiece();
    void rotatePiece();
    void moveLeft();
    void moveRight();
    void moveDown();
    void hardDrop();
    
    // 获取方块在4x4矩阵中的位置
    bool getPieceCell(int piece, int rotation, int x, int y);
    
    // 绘制函数
    void drawBoard(U8G2* u8g2);
    void drawCurrentPiece(U8G2* u8g2);
    void drawGhostPiece(U8G2* u8g2);
    void drawNextPiece(U8G2* u8g2);
    void drawScore(U8G2* u8g2);
    void drawBorder(U8G2* u8g2);
    void drawStartScreen(U8G2* u8g2);
    void drawGameOverScreen(U8G2* u8g2);
    void drawPausedScreen(U8G2* u8g2);
    
    // 绘制单个方块格子（带样式）
    void drawCell(U8G2* u8g2, int x, int y, int type, bool isGhost = false);
};

#endif
