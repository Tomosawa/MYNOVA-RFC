/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// ArkanoidPage.h
#ifndef ArkanoidPage_h
#define ArkanoidPage_h

#include "../GUI/UIPage.h"
#include "../GUIRender.h"
#include <U8g2lib.h>

// 游戏区域配置
#define ARK_BOARD_WIDTH     128  // 游戏区域宽度
#define ARK_BOARD_HEIGHT    56   // 游戏区域高度（留8像素给分数显示）

// 砖块配置
#define ARK_BRICK_COLS      11   // 砖块列数（减少到11列以适应屏幕）
#define ARK_BRICK_ROWS      4    // 砖块行数
#define ARK_BRICK_WIDTH     10   // 砖块宽度
#define ARK_BRICK_HEIGHT    4    // 砖块高度
#define ARK_BRICK_GAP       1    // 砖块间隙
#define ARK_BRICK_TOP       8    // 砖块区域顶部偏移
// 总宽度: 11 * 10 + 10 * 1 = 110 + 10 = 120 (留8像素边距)

// 挡板配置
#define ARK_PADDLE_WIDTH    20   // 挡板宽度
#define ARK_PADDLE_HEIGHT   3    // 挡板高度
#define ARK_PADDLE_Y        52   // 挡板Y位置

// 球配置
#define ARK_BALL_SIZE       3    // 球大小

class ArkanoidPage : public UIPage {
public:
    ArkanoidPage();
    ~ArkanoidPage();
    
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
        LEVEL_CLEAR,
        GAME_OVER
    };
    
    // 砖块类型
    enum BrickType {
        BRICK_NONE = 0,
        BRICK_NORMAL,       // 普通砖块 - 1次击破
        BRICK_HARD,         // 硬砖块 - 2次击破
        BRICK_SOLID,        // 坚固砖块 - 3次击破
        BRICK_INDESTRUCTIBLE // 不可破坏砖块
    };
    
    // 道具类型
    enum PowerUpType {
        POWERUP_NONE = 0,
        POWERUP_EXPAND,      // 挡板变长 L (Long)
        POWERUP_SHRINK,      // 挡板变短 S (Short)
        POWERUP_SLOW,        // 球减速 W (sloW)
        POWERUP_FAST,        // 球加速 F (Fast)
        POWERUP_LIFE,        // 额外生命 ♥ (保持爱心)
        POWERUP_SPEED_UP,    // 挡板加速 M (Move fast)
        POWERUP_GUN,         // 发射子弹 G (Gun)
        POWERUP_MULTIBALL    // 多球 B (Ball)
    };
    
    // 道具结构
    struct PowerUp {
        float x, y;
        PowerUpType type;
        bool active;
    };
    
    // 球结构（支持多球）
    struct Ball {
        float x, y;
        float vx, vy;
        bool active;
    };
    
    // 子弹结构
    struct Bullet {
        float x, y;
        bool active;
    };
    
    GameState gameState;
    
    // 砖块数据
    uint8_t bricks[ARK_BRICK_ROWS][ARK_BRICK_COLS];
    int bricksRemaining;
    
    // 挡板数据
    float paddleX;
    int paddleWidth;
    int paddleSpeed;
    
    // 球数据（支持多球，最多3个）
    static const int MAX_BALLS = 3;
    Ball balls[MAX_BALLS];
    float ballSpeed;
    bool ballLaunched;
    
    // 子弹数据
    static const int MAX_BULLETS = 2;
    Bullet bullets[MAX_BULLETS];
    bool hasGun;
    
    // 道具
    PowerUp powerUp;
    int powerUpTimer;
    
    // 道具状态（用于HUD显示）
    bool activeExpand;
    bool activeShrink;
    bool activeSlow;
    bool activeFast;
    bool activeSpeedUp;
    bool activeGun;
    int expandTimer;
    int shrinkTimer;
    int slowTimer;
    int fastTimer;
    int speedUpTimer;
    int gunTimer;
    
    // 游戏数据
    int score;
    int lives;
    int level;
    int combo;
    
    // 动画相关
    int frameCount;
    int hitEffectTimer;
    int hitEffectX, hitEffectY;
    
    // 游戏初始化
    void initGame();
    void initLevel();
    void resetBall();
    
    // 游戏逻辑
    void updateGame();
    void updateBalls();
    void updateBullets();
    void updatePowerUp();
    void updatePowerUpTimers();
    void checkBrickCollisionForBall(int ballIdx);
    void checkPaddleCollisionForBall(int ballIdx);
    void checkWallCollisionForBall(int ballIdx);
    void destroyBrick(int row, int col);
    void spawnPowerUp(int x, int y);
    void applyPowerUp();
    void nextLevel();
    void fireBullet();
    
    // 挡板控制
    void movePaddleLeft();
    void movePaddleRight();
    void launchBall();
    
    // 绘制函数
    void drawBricks(U8G2* u8g2);
    void drawPaddle(U8G2* u8g2);
    void drawBalls(U8G2* u8g2);
    void drawBullets(U8G2* u8g2);
    void drawPowerUp(U8G2* u8g2);
    void drawHUD(U8G2* u8g2);
    void drawHitEffect(U8G2* u8g2);
    void drawStartScreen(U8G2* u8g2);
    void drawGameOverScreen(U8G2* u8g2);
    void drawPausedScreen(U8G2* u8g2);
    void drawLevelClearScreen(U8G2* u8g2);
    
    // 辅助函数
    void generateLevel(int levelNum);
};

#endif
