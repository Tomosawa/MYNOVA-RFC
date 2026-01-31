/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// SnakePage.cpp
#include "SnakePage.h"
#include "../GUI/UIEngine.h"
#include "../ButtonDetector.h"
#include "clib/u8g2.h"

extern UIEngine uiEngine;

// 蛇头位图 - 向右 (8x8)
static const uint8_t snakeHeadRight[] PROGMEM = {
    0x3C, 0x42, 0x95, 0xA1, 0xA1, 0x95, 0x42, 0x3C
};

// 蛇头位图 - 向右张嘴 (8x8)
static const uint8_t snakeHeadRightOpen[] PROGMEM = {
    0x3C, 0x42, 0x95, 0xA1, 0xA1, 0x95, 0x4E, 0x38
};

// 食物位图 - 苹果 (8x8)
static const uint8_t foodApple[] PROGMEM = {
    0x08, 0x04, 0x3C, 0x7E, 0x7E, 0x7E, 0x3C, 0x00
};

// 奖励食物位图 - 樱桃 (8x8)
static const uint8_t foodCherry[] PROGMEM = {
    0x20, 0x10, 0x4C, 0x32, 0x32, 0x4C, 0x00, 0x00
};

// 金色食物位图 - 星星 (8x8)
static const uint8_t foodStar[] PROGMEM = {
    0x10, 0x10, 0x54, 0x38, 0x38, 0x54, 0x10, 0x10
};

SnakePage::SnakePage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) {
    initGame();
}

SnakePage::~SnakePage() {
    // 退出游戏页面时，恢复普通按键响应模式
    ButtonDetector::setFastResponseMode(false);
}

void SnakePage::showPage() {
    // 进入游戏页面时，启用快速按键响应模式
    ButtonDetector::setFastResponseMode(true);
}

void SnakePage::initGame() {
    gameState = START;
    score = 0;
    level = 1;
    foodEaten = 0;
    moveCounter = 0;
    moveInterval = 8;  // 初始移动速度
    frameCount = 0;
    headAnimFrame = 0;
    mouthOpen = false;
    eatEffectTimer = 0;
    hasBonusFood = false;
    bonusFoodTimer = 0;
    
    // 初始化蛇 - 从中间开始，长度为3
    snakeLength = 3;
    int startX = SNAKE_BOARD_WIDTH / 2;
    int startY = SNAKE_BOARD_HEIGHT / 2;
    
    for (int i = 0; i < snakeLength; i++) {
        snake[i].x = startX - i;
        snake[i].y = startY;
    }
    
    direction = DIR_RIGHT;
    nextDirection = DIR_RIGHT;
    
    // 生成第一个食物
    spawnFood();
}

void SnakePage::render(U8G2* u8g2) {
    u8g2->clearBuffer();
    
    switch (gameState) {
        case START:
            drawStartScreen(u8g2);
            break;
        case PLAYING:
            drawBorder(u8g2);
            drawFood(u8g2);
            drawSnake(u8g2);
            drawEatEffect(u8g2);
            drawScore(u8g2);
            break;
        case PAUSED:
            drawBorder(u8g2);
            drawFood(u8g2);
            drawSnake(u8g2);
            drawScore(u8g2);
            drawPausedScreen(u8g2);
            break;
        case GAME_OVER:
            drawBorder(u8g2);
            drawFood(u8g2);
            drawSnake(u8g2);
            drawScore(u8g2);
            drawGameOverScreen(u8g2);
            break;
    }
    
    u8g2->sendBuffer();
}

void SnakePage::update() {
    frameCount++;
    
    // 蛇头动画
    if (frameCount % 10 == 0) {
        headAnimFrame = (headAnimFrame + 1) % 4;
    }
    
    if (gameState == PLAYING) {
        // 吃食物特效
        if (eatEffectTimer > 0) {
            eatEffectTimer--;
        }
        
        // 奖励食物倒计时
        if (hasBonusFood) {
            bonusFoodTimer--;
            if (bonusFoodTimer <= 0) {
                hasBonusFood = false;
            }
        }
        
        updateGame();
    }
}

void SnakePage::updateGame() {
    moveCounter++;
    
    if (moveCounter >= moveInterval) {
        moveCounter = 0;
        
        // 更新方向
        direction = nextDirection;
        
        // 移动蛇
        moveSnake();
        
        // 检查是否吃到食物
        if (snake[0].x == food.x && snake[0].y == food.y) {
            eatFood();
            spawnFood();
            
            // 随机生成奖励食物
            if (!hasBonusFood && random(100) < 20) {
                spawnBonusFood();
            }
        }
        
        // 检查是否吃到奖励食物
        if (hasBonusFood && snake[0].x == bonusFood.x && snake[0].y == bonusFood.y) {
            score += 30;
            hasBonusFood = false;
            growSnake();
            eatEffectTimer = 15;
            eatEffectPos = bonusFood;
        }
        
        // 检查碰撞
        if (checkCollision()) {
            gameState = GAME_OVER;
        }
    }
}

void SnakePage::moveSnake() {
    // 保存尾巴位置（用于生长）
    Point tail = snake[snakeLength - 1];
    
    // 移动身体
    for (int i = snakeLength - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    
    // 移动头部
    switch (direction) {
        case DIR_UP:
            snake[0].y--;
            break;
        case DIR_DOWN:
            snake[0].y++;
            break;
        case DIR_LEFT:
            snake[0].x--;
            break;
        case DIR_RIGHT:
            snake[0].x++;
            break;
    }
}

void SnakePage::growSnake() {
    if (snakeLength < SNAKE_MAX_LENGTH) {
        // 在尾部添加一节
        snake[snakeLength] = snake[snakeLength - 1];
        snakeLength++;
    }
}

bool SnakePage::checkCollision() {
    return checkWallCollision() || checkSelfCollision();
}

bool SnakePage::checkWallCollision() {
    return snake[0].x < 0 || snake[0].x >= SNAKE_BOARD_WIDTH ||
           snake[0].y < 0 || snake[0].y >= SNAKE_BOARD_HEIGHT;
}

bool SnakePage::checkSelfCollision() {
    for (int i = 1; i < snakeLength; i++) {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            return true;
        }
    }
    return false;
}

void SnakePage::spawnFood() {
    int attempts = 0;
    do {
        food.x = random(SNAKE_BOARD_WIDTH);
        food.y = random(SNAKE_BOARD_HEIGHT);
        attempts++;
    } while (isPositionOnSnake(food.x, food.y) && attempts < 100);
    
    // 决定食物类型
    int chance = random(100);
    if (chance < 5) {
        foodType = FOOD_GOLDEN;  // 5% 金色食物
    } else if (chance < 15) {
        foodType = FOOD_BONUS;   // 10% 奖励食物
    } else {
        foodType = FOOD_NORMAL;  // 85% 普通食物
    }
}

void SnakePage::spawnBonusFood() {
    int attempts = 0;
    do {
        bonusFood.x = random(SNAKE_BOARD_WIDTH);
        bonusFood.y = random(SNAKE_BOARD_HEIGHT);
        attempts++;
    } while ((isPositionOnSnake(bonusFood.x, bonusFood.y) || 
              (bonusFood.x == food.x && bonusFood.y == food.y)) && 
             attempts < 100);
    
    hasBonusFood = true;
    bonusFoodTimer = 150;  // 约5秒后消失
}

void SnakePage::eatFood() {
    // 根据食物类型加分
    switch (foodType) {
        case FOOD_NORMAL:
            score += 10;
            break;
        case FOOD_BONUS:
            score += 30;
            break;
        case FOOD_GOLDEN:
            score += 50;
            break;
    }
    
    foodEaten++;
    growSnake();
    
    // 特效
    eatEffectTimer = 10;
    eatEffectPos = food;
    mouthOpen = true;
    
    // 每吃5个食物升一级
    int newLevel = foodEaten / 5 + 1;
    if (newLevel > level && level < 10) {
        level = newLevel;
        // 加快速度
        moveInterval = max(2, 8 - (level - 1));
    }
}

void SnakePage::setDirection(Direction dir) {
    // 防止180度转向
    if ((direction == DIR_UP && dir == DIR_DOWN) ||
        (direction == DIR_DOWN && dir == DIR_UP) ||
        (direction == DIR_LEFT && dir == DIR_RIGHT) ||
        (direction == DIR_RIGHT && dir == DIR_LEFT)) {
        return;
    }
    nextDirection = dir;
}

bool SnakePage::isPositionOnSnake(int x, int y) {
    for (int i = 0; i < snakeLength; i++) {
        if (snake[i].x == x && snake[i].y == y) {
            return true;
        }
    }
    return false;
}

int SnakePage::getPixelX(int gridX) {
    return SNAKE_BOARD_X + gridX * SNAKE_CELL_SIZE;
}

int SnakePage::getPixelY(int gridY) {
    return SNAKE_BOARD_Y + gridY * SNAKE_CELL_SIZE;
}

void SnakePage::drawSnake(U8G2* u8g2) {
    // 绘制蛇身（从尾到头）
    for (int i = snakeLength - 1; i >= 0; i--) {
        int px = getPixelX(snake[i].x);
        int py = getPixelY(snake[i].y);
        
        if (i == 0) {
            // 蛇头
            drawSnakeHead(u8g2, px, py);
        } else if (i == snakeLength - 1) {
            // 蛇尾
            drawSnakeTail(u8g2, px, py);
        } else {
            // 蛇身
            drawSnakeBody(u8g2, px, py, i);
        }
    }
}

void SnakePage::drawSnakeHead(U8G2* u8g2, int x, int y) {
    // 根据方向绘制蛇头
    // 使用简化的图形绘制，适应4x4格子
    
    // 绘制头部主体
    u8g2->drawBox(x, y, SNAKE_CELL_SIZE, SNAKE_CELL_SIZE);
    
    // 绘制眼睛位置（根据方向）
    u8g2->setDrawColor(0);
    switch (direction) {
        case DIR_RIGHT:
            u8g2->drawPixel(x + SNAKE_CELL_SIZE - 1, y);
            if (mouthOpen && headAnimFrame < 2) {
                u8g2->drawPixel(x + SNAKE_CELL_SIZE - 1, y + SNAKE_CELL_SIZE / 2);
            }
            break;
        case DIR_LEFT:
            u8g2->drawPixel(x, y);
            if (mouthOpen && headAnimFrame < 2) {
                u8g2->drawPixel(x, y + SNAKE_CELL_SIZE / 2);
            }
            break;
        case DIR_UP:
            u8g2->drawPixel(x, y);
            if (mouthOpen && headAnimFrame < 2) {
                u8g2->drawPixel(x + SNAKE_CELL_SIZE / 2, y);
            }
            break;
        case DIR_DOWN:
            u8g2->drawPixel(x, y + SNAKE_CELL_SIZE - 1);
            if (mouthOpen && headAnimFrame < 2) {
                u8g2->drawPixel(x + SNAKE_CELL_SIZE / 2, y + SNAKE_CELL_SIZE - 1);
            }
            break;
    }
    u8g2->setDrawColor(1);
    
    // 重置嘴巴状态
    if (headAnimFrame >= 2) {
        mouthOpen = false;
    }
}

void SnakePage::drawSnakeBody(U8G2* u8g2, int x, int y, int index) {
    // 蛇身 - 带花纹效果
    u8g2->drawBox(x, y, SNAKE_CELL_SIZE, SNAKE_CELL_SIZE);
    
    // 添加花纹（交替图案）
    if (index % 2 == 0) {
        u8g2->setDrawColor(0);
        u8g2->drawPixel(x + 1, y + 1);
        u8g2->setDrawColor(1);
    }
}

void SnakePage::drawSnakeTail(U8G2* u8g2, int x, int y) {
    // 蛇尾 - 较小的方块
    u8g2->drawBox(x + 1, y + 1, SNAKE_CELL_SIZE - 2, SNAKE_CELL_SIZE - 2);
}

void SnakePage::drawFood(U8G2* u8g2) {
    int px = getPixelX(food.x);
    int py = getPixelY(food.y);
    
    // 食物动画（闪烁效果）
    bool visible = true;
    if (foodType == FOOD_GOLDEN) {
        visible = (frameCount % 8) < 6;  // 金色食物快速闪烁
    } else if (foodType == FOOD_BONUS) {
        visible = (frameCount % 12) < 10;  // 奖励食物慢速闪烁
    }
    
    if (visible) {
        // 根据食物类型绘制不同样式
        switch (foodType) {
            case FOOD_NORMAL:
                // 普通食物 - 实心圆点
                u8g2->drawBox(px, py, SNAKE_CELL_SIZE, SNAKE_CELL_SIZE);
                break;
            case FOOD_BONUS:
                // 奖励食物 - 空心方块
                u8g2->drawFrame(px, py, SNAKE_CELL_SIZE, SNAKE_CELL_SIZE);
                u8g2->drawPixel(px + 1, py + 1);
                break;
            case FOOD_GOLDEN:
                // 金色食物 - 十字星
                u8g2->drawPixel(px + 1, py);
                u8g2->drawPixel(px, py + 1);
                u8g2->drawPixel(px + 1, py + 1);
                u8g2->drawPixel(px + 2, py + 1);
                u8g2->drawPixel(px + 1, py + 2);
                break;
        }
    }
    
    // 绘制奖励食物
    if (hasBonusFood) {
        int bpx = getPixelX(bonusFood.x);
        int bpy = getPixelY(bonusFood.y);
        
        // 闪烁效果（快要消失时更快）
        bool bonusVisible = true;
        if (bonusFoodTimer < 50) {
            bonusVisible = (frameCount % 4) < 2;
        } else {
            bonusVisible = (frameCount % 8) < 6;
        }
        
        if (bonusVisible) {
            // 樱桃样式
            u8g2->drawCircle(bpx + 1, bpy + 2, 1);
            u8g2->drawPixel(bpx + 2, bpy);
            u8g2->drawPixel(bpx + 3, bpy + 1);
        }
    }
}

void SnakePage::drawEatEffect(U8G2* u8g2) {
    if (eatEffectTimer > 0) {
        int px = getPixelX(eatEffectPos.x);
        int py = getPixelY(eatEffectPos.y);
        int size = (10 - eatEffectTimer) * 2;
        
        // 扩散效果
        u8g2->drawCircle(px + SNAKE_CELL_SIZE / 2, py + SNAKE_CELL_SIZE / 2, size / 2);
    }
}

void SnakePage::drawScore(U8G2* u8g2) {
    // 在底部留出的空白区域显示分数和等级
    u8g2->setFont(u8g2_font_5x7_tf);
    
    char scoreStr[16];
    snprintf(scoreStr, sizeof(scoreStr), "SCORE:%d", score);
    u8g2->drawStr(3, SCREEN_HEIGHT, scoreStr);
    
    char levelStr[8];
    snprintf(levelStr, sizeof(levelStr), "LV:%d", level);
    int levelWidth = u8g2->getUTF8Width(levelStr);
    u8g2->drawUTF8(SCREEN_WIDTH - levelWidth - 3, SCREEN_HEIGHT, levelStr);
}

void SnakePage::drawBorder(U8G2* u8g2) {
    // 绘制游戏区域边框
    int borderX = SNAKE_BOARD_X - 1;
    int borderY = SNAKE_BOARD_Y - 1;
    int borderW = SNAKE_BOARD_WIDTH * SNAKE_CELL_SIZE + 2;
    int borderH = SNAKE_BOARD_HEIGHT * SNAKE_CELL_SIZE + 2;
    
    u8g2->drawFrame(borderX, borderY, borderW, borderH);
}

void SnakePage::drawStartScreen(U8G2* u8g2) {
    // 绘制标题
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    const char* title = "贪吃蛇";
    int titleWidth = u8g2->getUTF8Width(title);
    u8g2->drawUTF8((SCREEN_WIDTH - titleWidth) / 2, 18, title);
    
    // 绘制装饰性蛇图案
    int decorX = SCREEN_WIDTH / 2 - 15;
    int decorY = 24;
    
    // 蛇身
    for (int i = 0; i < 6; i++) {
        int segX = decorX + i * 5;
        int segY = decorY + (i % 2) * 2;
        u8g2->drawBox(segX, segY, 4, 4);
    }
    
    // 绘制操作说明
    u8g2->setFont(u8g2_font_5x7_tf);
    const char* operationText = "2:UP 8:DOWN 4:L 6:R";
    u8g2->drawStr(16, 42, operationText);
    
    // 绘制开始提示
    if (frameCount % 60 < 40) {
        u8g2->setFont(u8g2_font_wqy12_t_gb2312);
        const char* startText = "按确认键开始";
        int startWidth = u8g2->getUTF8Width(startText);
        u8g2->drawUTF8((SCREEN_WIDTH - startWidth) / 2, 56, startText);
    }
}

void SnakePage::drawGameOverScreen(U8G2* u8g2) {
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

void SnakePage::drawPausedScreen(U8G2* u8g2) {
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
void SnakePage::onButtonBack(void* context) {
    if (gameState == PLAYING) {
        gameState = PAUSED;
    } else if (gameState == PAUSED) {
        uiEngine.navigateBack();
    } else {
        uiEngine.navigateBack();
    }
}

void SnakePage::onButtonEnter(void* context) {
    switch (gameState) {
        case START:
            initGame();
            gameState = PLAYING;
            break;
        case PLAYING:
            // 确认键可以暂停
            gameState = PAUSED;
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

void SnakePage::onButtonMenu(void* context) {
    if (gameState == PLAYING) {
        gameState = PAUSED;
    } else if (gameState == PAUSED) {
        gameState = PLAYING;
    }
}


void SnakePage::onButton2(void* context) {
    // 2键 - 上
    if (gameState == PLAYING) {
        setDirection(DIR_UP);
    }
}


void SnakePage::onButton4(void* context) {
    // 4键 - 左
    if (gameState == PLAYING) {
        setDirection(DIR_LEFT);
    }
}


void SnakePage::onButton6(void* context) {
    // 6键 - 右
    if (gameState == PLAYING) {
        setDirection(DIR_RIGHT);
    }
}



void SnakePage::onButton8(void* context) {
    // 8键 - 下
    if (gameState == PLAYING) {
        setDirection(DIR_DOWN);
    }
}

