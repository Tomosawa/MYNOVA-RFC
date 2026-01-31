/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// FlappyBirdPage.cpp
#include "FlappyBirdPage.h"
#include "../GUI/UIEngine.h"
#include "../ButtonDetector.h"
#include "clib/u8g2.h"

extern UIEngine uiEngine;

// 小鸟位图数据
static const uint8_t flappybird_frame_1[] PROGMEM = {
    0x03, 0xF0, 0x0C, 0x48, 0x10, 0x84, 0x78, 0x8A,
    0x84, 0x8A, 0x82, 0x42, 0x82, 0x3E, 0x44, 0x41,
    0x38, 0xBE, 0x20, 0x41, 0x18, 0x3E, 0x07, 0xC0
};

static const uint8_t flappybird_frame_2[] PROGMEM = {
    0x03, 0xF0, 0x0C, 0x48, 0x10, 0x84, 0x20, 0x8A,
    0x40, 0x8A, 0x40, 0x42, 0x7C, 0x3E, 0x82, 0x41,
    0x84, 0xBE, 0x88, 0x41, 0x78, 0x3E, 0x07, 0xC0
};

// 管道位图数据
static const uint8_t bar_top[] PROGMEM = {
    0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 
    0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x42, 0xFF, 0xFF, 0xFF
};

static const uint8_t bar_bottom[] PROGMEM = {
    0xFF, 0xFF, 0xFF, 0x42, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 
    0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E
};

// 管道参数
const int PIPE_HEIGHT = 20;
const int PIPE_BITMAP_WIDTH = 8;

FlappyBirdPage::FlappyBirdPage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) {
    initGame();
}

FlappyBirdPage::~FlappyBirdPage() {
    // 退出游戏页面时，恢复普通按键响应模式
    ButtonDetector::setFastResponseMode(false);
}

void FlappyBirdPage::showPage() {
    // 进入游戏页面时，启用快速按键响应模式
    ButtonDetector::setFastResponseMode(true);
}

void FlappyBirdPage::initGame() {
    gameState = START;
    birdX = SCREEN_WIDTH / 4;
    birdY = SCREEN_HEIGHT / 2;
    deltaIde = 1;  
    moveSpeed = 1;  // 初始移动速度
    
    // 初始化两个管道
    pipes[0].x = SCREEN_WIDTH;
    pipes[0].gapY = random(10, SCREEN_HEIGHT - 10 - PIPE_GAP);
    pipes[0].passed = false;
    
    pipes[1].x = SCREEN_WIDTH + SCREEN_WIDTH / 2;  // 第二个管道在屏幕右侧中间位置
    pipes[1].gapY = random(10, SCREEN_HEIGHT - 10 - PIPE_GAP);
    pipes[1].passed = false;
    
    score = 0;
    frameCount = 0;
    delayFrame = 0;
    jumpCount = 0;
    clicked = false;
}

void FlappyBirdPage::render(U8G2* u8g2) {
    // 清空屏幕
    u8g2->clearBuffer();
    
    switch (gameState) {
        case START:
            drawStartScreen(u8g2);
            break;
        case PLAYING:
            drawPipes(u8g2);
            drawBird(u8g2);
            drawScore(u8g2);
            break;
        case GAME_OVER:
            drawPipes(u8g2);
            drawBird(u8g2);
            drawScore(u8g2);
            drawGameOverScreen(u8g2);
            break;
    }
    
    // 更新屏幕
    u8g2->sendBuffer();
}

void FlappyBirdPage::update() {
    switch (gameState) {
        case PLAYING:
            // 更新游戏状态
            updateGame();
            break;
        default:
            break;
    }
}

void FlappyBirdPage::updateGame() {
    // 帧计数器每帧都更新，确保动画流畅
    frameCount++;
    if (frameCount >= ANIM_FRAME) frameCount = 0;
    
    birdY += deltaIde * moveSpeed;
    
    // 更新所有管道位置
    for (int i = 0; i < 2; i++) {
        pipes[i].x -= BIRD_SPEED;
        
        // 检查小鸟是否通过管道，如果通过则增加分数
        if (pipes[i].x + PIPE_WIDTH < birdX && !pipes[i].passed) {
            score++;
            pipes[i].passed = true;
        }
        
        // 管道移出屏幕后重新生成
        if (pipes[i].x < -PIPE_WIDTH) {
            pipes[i].x = SCREEN_WIDTH;
            pipes[i].gapY = random(10, SCREEN_HEIGHT - 10 - PIPE_GAP);
            pipes[i].passed = false; // 重置通过标记
        }
    }
    
    delayFrame++;
    if (delayFrame == DELAY_FRAME) {
        delayFrame = 0;
        if(deltaIde > 0)
            deltaIde += 1;
    }
    
    // 确保鸟不会飞出屏幕底部
    if (birdY > SCREEN_HEIGHT - BIRD_HEIGHT) {
        birdY = SCREEN_HEIGHT - BIRD_HEIGHT;
        gameState = GAME_OVER;
    }
    
    // 确保鸟不会飞出屏幕顶部
    if (birdY < 0) {
        birdY = 0;
    }
    
    // 碰撞检测
    if (checkCollision()) {
        gameState = GAME_OVER;
    }
    
    // 重置点击状态
    if (clicked) {
        cancelJump();
        clicked = false;
    }
}

bool FlappyBirdPage::checkCollision() {
    // 检查是否撞到任何管道
    for (int i = 0; i < 2; i++) {
        if (birdX + BIRD_WIDTH > pipes[i].x && birdX < pipes[i].x + PIPE_WIDTH) {
            if (birdY < pipes[i].gapY || birdY + BIRD_HEIGHT > pipes[i].gapY + PIPE_GAP) {
                return true;
            }
        }
    }
    return false;
}

// 鸟向下飞
void FlappyBirdPage::flyDown() {
    deltaIde = 1;
    moveSpeed = 1;
}

// 鸟向上飞
void FlappyBirdPage::flyUp() {
    deltaIde = -1;
    moveSpeed = 5;  // 增加向上飞的速度，提高按键响应
}

// 取消跳跃
void FlappyBirdPage::cancelJump() {
    jumpCount = 0;
    flyDown();
}

void FlappyBirdPage::drawBird(U8G2* u8g2) {
    // 根据帧计数器绘制不同的小鸟动画帧
    if (frameCount < ANIM_FRAME / 2) {
        u8g2->drawBitmap(birdX, birdY, BIRD_WIDTH / 8, BIRD_HEIGHT, flappybird_frame_1);
    } else {
        u8g2->drawBitmap(birdX, birdY, BIRD_WIDTH / 8, BIRD_HEIGHT, flappybird_frame_2);
    }
}

void FlappyBirdPage::drawPipes(U8G2* u8g2) {
    // 管道中间部分宽度为6像素（0x7E的二进制是01111110，中间6位是1）
    const int PIPE_MIDDLE_WIDTH = 6;
    
    // 绘制所有管道
    for (int i = 0; i < 2; i++) {
        // 计算管道中间部分的起始X坐标（8像素宽的位图，中间6像素从X+1开始）
        int middleX = pipes[i].x + 1;
        
        // 绘制上半部分管道的延伸部分，充满屏幕顶部
        if (pipes[i].gapY - PIPE_HEIGHT > 0) {
            u8g2->drawBox(middleX, 0, PIPE_MIDDLE_WIDTH, pipes[i].gapY - PIPE_HEIGHT);
        }
        
        // 绘制上半部分管道的固定部分（8像素宽的位图）
        u8g2->drawBitmap(pipes[i].x, pipes[i].gapY - PIPE_HEIGHT, 1, PIPE_HEIGHT, bar_top);
        
        // 绘制下半部分管道的固定部分（8像素宽的位图）
        u8g2->drawBitmap(pipes[i].x, pipes[i].gapY + PIPE_GAP, 1, PIPE_HEIGHT, bar_bottom);
        
        // 绘制下半部分管道的延伸部分，充满屏幕底部
        if (pipes[i].gapY + PIPE_GAP + PIPE_HEIGHT < SCREEN_HEIGHT) {
            u8g2->drawBox(middleX, pipes[i].gapY + PIPE_GAP + PIPE_HEIGHT, PIPE_MIDDLE_WIDTH, SCREEN_HEIGHT - (pipes[i].gapY + PIPE_GAP + PIPE_HEIGHT));
        }
    }
}

void FlappyBirdPage::drawScore(U8G2* u8g2) {
    // 绘制分数在左上角
    u8g2->setFont(u8g2_font_5x8_tf);
    String scoreText = "SCORE:" + String(score);
    u8g2->drawUTF8(0, 8, scoreText.c_str());
}

void FlappyBirdPage::drawStartScreen(U8G2* u8g2) {
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    
    // 绘制标题 - 基线在屏幕中上部
    String gameTitle = "Flappy Bird";
    int textWidth = u8g2->getUTF8Width(gameTitle.c_str());
    u8g2->drawUTF8((SCREEN_WIDTH - textWidth) / 2, 20, gameTitle.c_str());
    
    // 在开始界面绘制小鸟 - 在标题下方
    u8g2->drawBitmap(SCREEN_WIDTH / 2 - BIRD_WIDTH / 2, 26, 
                   BIRD_WIDTH / 8, BIRD_HEIGHT, flappybird_frame_1);
    
    // 绘制开始提示 - 在小鸟下方
    String startText = "按确认键开始";
    textWidth = u8g2->getUTF8Width(startText.c_str());
    u8g2->drawUTF8((SCREEN_WIDTH - textWidth) / 2, 52, startText.c_str());
}

void FlappyBirdPage::drawGameOverScreen(U8G2* u8g2) {
    // 半透明遮罩效果
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

void FlappyBirdPage::onButtonBack(void* context) {
    // 返回游戏清单页面
    uiEngine.navigateBack();
}

void FlappyBirdPage::onButtonEnter(void* context) {
    switch (gameState) {
        case START:
            // 开始游戏
            gameState = PLAYING;
            break;
        case PLAYING:
            // 鸟向上飞
            flyUp();
            clicked = true;
            break;
        case GAME_OVER:
            // 重新开始游戏
            initGame();
            break;
    }
}

void FlappyBirdPage::onButtonMenu(void* context) {
    // 菜单键也用于鸟向上飞
    if (gameState == PLAYING) {
        // 鸟向上飞
        flyUp();
        clicked = true;
    }
}

void FlappyBirdPage::onButton1(void* context) {
    if (gameState == PLAYING) {
        // 鸟向上飞
        flyUp();
        clicked = true;
    }
}

void FlappyBirdPage::onButton2(void* context) {
    if (gameState == PLAYING) {
        // 鸟向上飞
        flyUp();
        clicked = true;
    }
}

void FlappyBirdPage::onButton3(void* context) {
    if (gameState == PLAYING) {
        // 鸟向上飞
        flyUp();
        clicked = true;
    }
}

void FlappyBirdPage::onButton4(void* context) {
    if (gameState == PLAYING) {
        // 鸟向上飞
        flyUp();
        clicked = true;
    }
}

void FlappyBirdPage::onButton5(void* context) {
    if (gameState == PLAYING) {
        // 鸟向上飞
        flyUp();
        clicked = true;
    }
}

void FlappyBirdPage::onButton6(void* context) {
    if (gameState == PLAYING) {
        // 鸟向上飞
        flyUp();
        clicked = true;
    }
}

void FlappyBirdPage::onButton7(void* context) {
    if (gameState == PLAYING) {
        // 鸟向上飞
        flyUp();
        clicked = true;
    }
}

void FlappyBirdPage::onButton8(void* context) {
    if (gameState == PLAYING) {
        // 鸟向上飞
        flyUp();
        clicked = true;
    }
}

void FlappyBirdPage::onButton9(void* context) {
    if (gameState == PLAYING) {
        // 鸟向上飞
        flyUp();
        clicked = true;
    }
}