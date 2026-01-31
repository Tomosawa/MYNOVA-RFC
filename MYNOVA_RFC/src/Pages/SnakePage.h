/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// SnakePage.h
#ifndef SnakePage_h
#define SnakePage_h

#include "../GUI/UIPage.h"
#include "../GUIRender.h"
#include <U8g2lib.h>
#include <vector>

// 游戏区域配置
#define SNAKE_BOARD_WIDTH   30   // 游戏区域宽度（格子数）
#define SNAKE_BOARD_HEIGHT  14   // 游戏区域高度（格子数）- 减少1行，为底部文字留空间
#define SNAKE_CELL_SIZE     4    // 每个格子的像素大小
#define SNAKE_BOARD_X       4    // 游戏区域起始X坐标
#define SNAKE_BOARD_Y       1    // 游戏区域起始Y坐标

// 最大蛇身长度
#define SNAKE_MAX_LENGTH    100

class SnakePage : public UIPage {
public:
    SnakePage();
    ~SnakePage();
    
    // 重写页面显示函数
    void showPage() override;
    
    // 重写渲染和更新函数
    void render(U8G2* u8g2) override;
    void update() override;
    
    // 重写按钮事件处理
    void onButtonBack(void* context = nullptr) override;
    void onButtonEnter(void* context = nullptr) override;
    void onButtonMenu(void* context = nullptr) override;
    void onButton2(void* context = nullptr) override;
    void onButton4(void* context = nullptr) override;
    void onButton6(void* context = nullptr) override;
    void onButton8(void* context = nullptr) override;
    
private:
    // 游戏状态
    enum GameState {
        START,
        PLAYING,
        PAUSED,
        GAME_OVER
    };
    
    // 方向枚举
    enum Direction {
        DIR_UP = 0,
        DIR_DOWN,
        DIR_LEFT,
        DIR_RIGHT
    };
    
    // 坐标结构
    struct Point {
        int x;
        int y;
    };
    
    // 食物类型
    enum FoodType {
        FOOD_NORMAL = 0,    // 普通食物 +10分
        FOOD_BONUS,         // 奖励食物 +30分 (会消失)
        FOOD_GOLDEN         // 金色食物 +50分 (稀有)
    };
    
    GameState gameState;
    
    // 蛇身数据
    Point snake[SNAKE_MAX_LENGTH];
    int snakeLength;
    Direction direction;
    Direction nextDirection;  // 下一帧的方向（防止快速转向导致自撞）
    
    // 食物数据
    Point food;
    FoodType foodType;
    int bonusFoodTimer;      // 奖励食物倒计时
    bool hasBonusFood;
    Point bonusFood;
    
    // 游戏数据
    int score;
    int level;
    int moveCounter;
    int moveInterval;        // 移动间隔（帧数）
    int foodEaten;           // 吃掉的食物数量
    
    // 动画相关
    int frameCount;
    int headAnimFrame;       // 蛇头动画帧
    bool mouthOpen;          // 嘴巴是否张开
    
    // 特效
    int eatEffectTimer;      // 吃食物特效计时
    Point eatEffectPos;      // 特效位置
    
    // 游戏初始化
    void initGame();
    
    // 游戏逻辑
    void updateGame();
    void moveSnake();
    void growSnake();
    bool checkCollision();
    bool checkSelfCollision();
    bool checkWallCollision();
    void spawnFood();
    void spawnBonusFood();
    void eatFood();
    
    // 方向控制
    void setDirection(Direction dir);
    
    // 绘制函数
    void drawBoard(U8G2* u8g2);
    void drawSnake(U8G2* u8g2);
    void drawSnakeHead(U8G2* u8g2, int x, int y);
    void drawSnakeBody(U8G2* u8g2, int x, int y, int index);
    void drawSnakeTail(U8G2* u8g2, int x, int y);
    void drawFood(U8G2* u8g2);
    void drawScore(U8G2* u8g2);
    void drawBorder(U8G2* u8g2);
    void drawStartScreen(U8G2* u8g2);
    void drawGameOverScreen(U8G2* u8g2);
    void drawPausedScreen(U8G2* u8g2);
    void drawEatEffect(U8G2* u8g2);
    
    // 辅助函数
    bool isPositionOnSnake(int x, int y);
    int getPixelX(int gridX);
    int getPixelY(int gridY);
};

#endif
