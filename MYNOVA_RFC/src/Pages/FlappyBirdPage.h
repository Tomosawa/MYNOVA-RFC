/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// FlappyBirdPage.h
#ifndef FlappyBirdPage_h
#define FlappyBirdPage_h

#include "../GUI/UIPage.h"
#include "../GUIRender.h"
#include <U8g2lib.h>

class FlappyBirdPage : public UIPage {
public:
    FlappyBirdPage();
    ~FlappyBirdPage();  // 析构函数，用于恢复普通按键模式
    
    // 重写页面显示函数，启用快速响应模式
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
    // 游戏逻辑函数
    void flyDown();
    void flyUp();
    void cancelJump();
    
private:
    // 游戏状态
    enum GameState {
        START,        // 开始界面
        PLAYING,      // 游戏中
        GAME_OVER     // 游戏结束
    };
    
    GameState gameState;
    
    // 游戏参数
    const int BIRD_WIDTH = 16;
    const int BIRD_HEIGHT = 12;
    const int PIPE_WIDTH = 8;
    const int PIPE_GAP = 25;
    const int BIRD_SPEED = 1;
    const int GRAVITY = 1;
    
    // 管道结构体
    struct Pipe {
        int x;
        int gapY;
        bool passed;
    };
    
    // 游戏对象位置
    int birdX, birdY;
    int deltaIde;  // 方向和速度控制变量
    int moveSpeed; // 移动速度
    Pipe pipes[2];  // 两个管道实例
    int score;
    
    // 动画相关
    const int ANIM_FRAME = 30;  // 减少动画帧周期，使小鸟动画更快
    const int DELAY_FRAME = 3;  
    int frameCount;
    int delayFrame;
    
    // 跳跃相关
    int jumpCount;
    const int maxJumpCount = 20;
    bool clicked; // 跟踪按钮状态
    
    // 游戏初始化
    void initGame();
    
    // 游戏逻辑更新
    void updateGame();
    
    // 绘制游戏元素
    void drawBird(U8G2* u8g2);
    void drawPipes(U8G2* u8g2);
    void drawScore(U8G2* u8g2);
    void drawStartScreen(U8G2* u8g2);
    void drawGameOverScreen(U8G2* u8g2);
    
    // 碰撞检测
    bool checkCollision();
};

#endif