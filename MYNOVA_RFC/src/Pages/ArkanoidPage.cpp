/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// ArkanoidPage.cpp
#include "ArkanoidPage.h"
#include "../GUI/UIEngine.h"
#include "../ButtonDetector.h"
#include "clib/u8g2.h"
#include <Arduino.h>

extern UIEngine uiEngine;

ArkanoidPage::ArkanoidPage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) {
    initGame();
}

ArkanoidPage::~ArkanoidPage() {
    // 退出游戏页面时，恢复普通按键响应模式
    ButtonDetector::setFastResponseMode(false);
    ButtonDetector::setLongPressEnabled(false);
}

void ArkanoidPage::showPage() {
    // 进入游戏页面时，启用快速按键响应模式和长按模式
    ButtonDetector::setFastResponseMode(true);
    ButtonDetector::setLongPressEnabled(true);
}

void ArkanoidPage::initGame() {
    gameState = START;
    score = 0;
    lives = 3;
    level = 1;
    combo = 0;
    frameCount = 0;
    hitEffectTimer = 0;
    
    // 初始化挡板
    paddleX = (float)(ARK_BOARD_WIDTH - ARK_PADDLE_WIDTH) / 2.0f;
    paddleWidth = ARK_PADDLE_WIDTH;
    paddleSpeed = 3;
    
    // 初始化球
    ballSpeed = 1.5f;
    ballLaunched = false;
    for (int i = 0; i < MAX_BALLS; i++) {
        balls[i].active = false;
    }
    balls[0].active = true; // 第一个球激活
    resetBall();
    
    // 初始化子弹
    hasGun = false;
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = false;
    }
    
    // 初始化道具
    powerUp.active = false;
    powerUpTimer = 0;
    
    // 初始化道具状态
    activeExpand = false;
    activeShrink = false;
    activeSlow = false;
    activeFast = false;
    activeSpeedUp = false;
    activeGun = false;
    expandTimer = 0;
    shrinkTimer = 0;
    slowTimer = 0;
    fastTimer = 0;
    speedUpTimer = 0;
    gunTimer = 0;
    
    // 初始化关卡
    initLevel();
}

void ArkanoidPage::initLevel() {
    generateLevel(level);
    resetBall();
    combo = 0;
}

void ArkanoidPage::resetBall() {
    ballLaunched = false;
    // 重置所有球
    for (int i = 0; i < MAX_BALLS; i++) {
        balls[i].active = false;
    }
    // 激活第一个球
    balls[0].active = true;
    balls[0].x = paddleX + paddleWidth / 2.0f - ARK_BALL_SIZE / 2.0f;
    balls[0].y = ARK_PADDLE_Y - ARK_BALL_SIZE - 1;
    balls[0].vx = 0;
    balls[0].vy = 0;
}

void ArkanoidPage::generateLevel(int levelNum) {
    bricksRemaining = 0;
    
    // 根据关卡生成不同的砖块布局
    switch (levelNum % 5) {
        case 1: // 第一关 - 简单的彩虹图案
            for (int row = 0; row < ARK_BRICK_ROWS; row++) {
                for (int col = 0; col < ARK_BRICK_COLS; col++) {
                    bricks[row][col] = BRICK_NORMAL;
                    bricksRemaining++;
                }
            }
            break;
            
        case 2: // 第二关 - 金字塔
            for (int row = 0; row < ARK_BRICK_ROWS; row++) {
                for (int col = 0; col < ARK_BRICK_COLS; col++) {
                    if (col >= row && col < ARK_BRICK_COLS - row) {
                        if (row < 2) {
                            bricks[row][col] = BRICK_HARD;
                        } else {
                            bricks[row][col] = BRICK_NORMAL;
                        }
                        bricksRemaining++;
                    } else {
                        bricks[row][col] = BRICK_NONE;
                    }
                }
            }
            break;
            
        case 3: // 第三关 - 棋盘格
            for (int row = 0; row < ARK_BRICK_ROWS; row++) {
                for (int col = 0; col < ARK_BRICK_COLS; col++) {
                    if ((row + col) % 2 == 0) {
                        bricks[row][col] = BRICK_HARD;
                        bricksRemaining++;
                    } else {
                        bricks[row][col] = BRICK_NORMAL;
                        bricksRemaining++;
                    }
                }
            }
            break;
            
        case 4: // 第四关 - 堡垒
            for (int row = 0; row < ARK_BRICK_ROWS; row++) {
                for (int col = 0; col < ARK_BRICK_COLS; col++) {
                    if (col < 3 || col >= ARK_BRICK_COLS - 3) {
                        bricks[row][col] = BRICK_SOLID;
                        bricksRemaining++;
                    } else if (row == 0 || row == ARK_BRICK_ROWS - 1) {
                        bricks[row][col] = BRICK_HARD;
                        bricksRemaining++;
                    } else {
                        bricks[row][col] = BRICK_NORMAL;
                        bricksRemaining++;
                    }
                }
            }
            break;
            
        case 0: // 第五关及以后 - 复杂混合布局
            for (int row = 0; row < ARK_BRICK_ROWS; row++) {
                for (int col = 0; col < ARK_BRICK_COLS; col++) {
                    if (col == ARK_BRICK_COLS / 2 || col == ARK_BRICK_COLS / 2 - 1) {
                        bricks[row][col] = BRICK_INDESTRUCTIBLE;
                    } else {
                        int type = (row + col) % 3;
                        if (type == 0) {
                            bricks[row][col] = BRICK_NORMAL;
                        } else if (type == 1) {
                            bricks[row][col] = BRICK_HARD;
                        } else {
                            bricks[row][col] = BRICK_SOLID;
                        }
                        bricksRemaining++;
                    }
                }
            }
            break;
    }
}

void ArkanoidPage::render(U8G2* u8g2) {
    u8g2->clearBuffer();
    
    switch (gameState) {
        case START:
            drawStartScreen(u8g2);
            break;
            
        case PLAYING:
            drawBricks(u8g2);
            drawPaddle(u8g2);
            drawBalls(u8g2);
            drawBullets(u8g2);
            drawPowerUp(u8g2);
            drawHitEffect(u8g2);
            drawHUD(u8g2);
            break;
            
        case PAUSED:
            drawBricks(u8g2);
            drawPaddle(u8g2);
            drawBalls(u8g2);
            drawHUD(u8g2);
            drawPausedScreen(u8g2);
            break;
            
        case LEVEL_CLEAR:
            drawLevelClearScreen(u8g2);
            break;
            
        case GAME_OVER:
            drawGameOverScreen(u8g2);
            break;
    }
    
    u8g2->sendBuffer();
}

void ArkanoidPage::update() {
    frameCount++;
    
    switch (gameState) {
        case PLAYING:
            updateGame();
            break;
        default:
            break;
    }
}

void ArkanoidPage::updateGame() {
    // 更新球的位置
    if (ballLaunched) {
        updateBalls();
        
        // 检查是否所有球都掉落
        bool anyBallActive = false;
        for (int i = 0; i < MAX_BALLS; i++) {
            if (balls[i].active) {
                anyBallActive = true;
                break;
            }
        }
        
        if (!anyBallActive) {
            lives--;
            combo = 0; // 重置连击
            if (lives <= 0) {
                gameState = GAME_OVER;
            } else {
                resetBall();
            }
        }
    } else {
        // 球未发射时跟随挡板
        balls[0].x = paddleX + paddleWidth / 2.0f - ARK_BALL_SIZE / 2.0f;
    }
    
    // 更新子弹
    updateBullets();
    
    // 更新道具
    updatePowerUp();
    updatePowerUpTimers();
    
    // 更新击中特效
    if (hitEffectTimer > 0) {
        hitEffectTimer--;
    }
    
    // 检查关卡是否完成
    if (bricksRemaining <= 0) {
        gameState = LEVEL_CLEAR;
    }
}

void ArkanoidPage::updateBalls() {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (!balls[i].active) continue;
        
        balls[i].x += balls[i].vx * ballSpeed;
        balls[i].y += balls[i].vy * ballSpeed;
        
        checkWallCollisionForBall(i);
        checkBrickCollisionForBall(i);
        checkPaddleCollisionForBall(i);
        
        // 检查球是否掉落
        if (balls[i].y > SCREEN_HEIGHT) {
            balls[i].active = false;
        }
    }
}

void ArkanoidPage::updateBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        
        bullets[i].y -= 2; // 子弹向上移动
        
        // 子弹超出屏幕
        if (bullets[i].y < 0) {
            bullets[i].active = false;
            continue;
        }
        
        // 检查子弹与砖块碰撞
        for (int row = 0; row < ARK_BRICK_ROWS; row++) {
            for (int col = 0; col < ARK_BRICK_COLS; col++) {
                if (bricks[row][col] == BRICK_NONE) continue;
                
                int brickX = col * (ARK_BRICK_WIDTH + ARK_BRICK_GAP) + 4; // 居中偏移
                int brickY = ARK_BRICK_TOP + row * (ARK_BRICK_HEIGHT + ARK_BRICK_GAP);
                
                if (bullets[i].x + 1 > brickX && bullets[i].x < brickX + ARK_BRICK_WIDTH &&
                    bullets[i].y + 3 > brickY && bullets[i].y < brickY + ARK_BRICK_HEIGHT) {
                    
                    if (bricks[row][col] != BRICK_INDESTRUCTIBLE) {
                        destroyBrick(row, col);
                    }
                    bullets[i].active = false;
                    break;
                }
            }
            if (!bullets[i].active) break;
        }
    }
}

void ArkanoidPage::updatePowerUpTimers() {
    // 更新各个道具计时器
    if (expandTimer > 0) {
        expandTimer--;
        if (expandTimer == 0) {
            activeExpand = false;
            paddleWidth = ARK_PADDLE_WIDTH;
        }
    }
    
    if (shrinkTimer > 0) {
        shrinkTimer--;
        if (shrinkTimer == 0) {
            activeShrink = false;
            paddleWidth = ARK_PADDLE_WIDTH;
        }
    }
    
    if (slowTimer > 0) {
        slowTimer--;
        if (slowTimer == 0) {
            activeSlow = false;
            ballSpeed = 1.5f;
        }
    }
    
    if (fastTimer > 0) {
        fastTimer--;
        if (fastTimer == 0) {
            activeFast = false;
            ballSpeed = 1.5f;
        }
    }
    
    if (speedUpTimer > 0) {
        speedUpTimer--;
        if (speedUpTimer == 0) {
            activeSpeedUp = false;
            paddleSpeed = 3;
        }
    }
    
    if (gunTimer > 0) {
        gunTimer--;
        if (gunTimer == 0) {
            activeGun = false;
            hasGun = false;
        }
    }
}

void ArkanoidPage::updatePowerUp() {
    if (powerUp.active) {
        powerUp.y += 1; // 道具下落速度
        
        // 检查道具是否被挡板接住（道具框8x8）
        if (powerUp.y + 8 >= ARK_PADDLE_Y && 
            powerUp.y <= ARK_PADDLE_Y + ARK_PADDLE_HEIGHT &&
            powerUp.x + 8 >= paddleX && 
            powerUp.x <= paddleX + paddleWidth) {
            applyPowerUp();
            powerUp.active = false;
        }
        
        // 道具掉出屏幕
        if (powerUp.y > SCREEN_HEIGHT) {
            powerUp.active = false;
        }
    }
}

void ArkanoidPage::checkWallCollisionForBall(int ballIdx) {
    if (!balls[ballIdx].active) return;
    
    // 左右墙壁碰撞
    if (balls[ballIdx].x <= 0 || balls[ballIdx].x + ARK_BALL_SIZE >= ARK_BOARD_WIDTH) {
        balls[ballIdx].vx = -balls[ballIdx].vx;
        balls[ballIdx].x = balls[ballIdx].x <= 0 ? 0 : ARK_BOARD_WIDTH - ARK_BALL_SIZE;
    }
    
    // 顶部墙壁碰撞
    if (balls[ballIdx].y <= 0) {
        balls[ballIdx].vy = -balls[ballIdx].vy;
        balls[ballIdx].y = 0;
    }
}

void ArkanoidPage::checkBrickCollisionForBall(int ballIdx) {
    if (!balls[ballIdx].active) return;
    
    for (int row = 0; row < ARK_BRICK_ROWS; row++) {
        for (int col = 0; col < ARK_BRICK_COLS; col++) {
            if (bricks[row][col] == BRICK_NONE) continue;
            
            int brickX = col * (ARK_BRICK_WIDTH + ARK_BRICK_GAP) + 4; // 居中偏移
            int brickY = ARK_BRICK_TOP + row * (ARK_BRICK_HEIGHT + ARK_BRICK_GAP);
            
            if (balls[ballIdx].x + ARK_BALL_SIZE > brickX && balls[ballIdx].x < brickX + ARK_BRICK_WIDTH &&
                balls[ballIdx].y + ARK_BALL_SIZE > brickY && balls[ballIdx].y < brickY + ARK_BRICK_HEIGHT) {
                
                // 判断碰撞方向
                float overlapLeft = (balls[ballIdx].x + ARK_BALL_SIZE) - brickX;
                float overlapRight = (brickX + ARK_BRICK_WIDTH) - balls[ballIdx].x;
                float overlapTop = (balls[ballIdx].y + ARK_BALL_SIZE) - brickY;
                float overlapBottom = (brickY + ARK_BRICK_HEIGHT) - balls[ballIdx].y;
                
                float minOverlap = min(min(overlapLeft, overlapRight), min(overlapTop, overlapBottom));
                
                if (minOverlap == overlapLeft || minOverlap == overlapRight) {
                    balls[ballIdx].vx = -balls[ballIdx].vx;
                } else {
                    balls[ballIdx].vy = -balls[ballIdx].vy;
                }
                
                // 不可破坏砖块只反弹
                if (bricks[row][col] == BRICK_INDESTRUCTIBLE) {
                    hitEffectX = brickX + ARK_BRICK_WIDTH / 2;
                    hitEffectY = brickY + ARK_BRICK_HEIGHT / 2;
                    hitEffectTimer = 10;
                } else {
                    destroyBrick(row, col);
                }
                return; // 一次只处理一个砖块碰撞
            }
        }
    }
}

void ArkanoidPage::checkPaddleCollisionForBall(int ballIdx) {
    if (!balls[ballIdx].active) return;
    
    if (balls[ballIdx].y + ARK_BALL_SIZE >= ARK_PADDLE_Y && 
        balls[ballIdx].y < ARK_PADDLE_Y + ARK_PADDLE_HEIGHT &&
        balls[ballIdx].x + ARK_BALL_SIZE >= paddleX && 
        balls[ballIdx].x <= paddleX + paddleWidth) {
        
        // 反弹
        balls[ballIdx].vy = -abs(balls[ballIdx].vy);
        
        // 根据球击中挡板的位置调整水平速度
        float hitPos = (balls[ballIdx].x + (float)ARK_BALL_SIZE / 2.0f - paddleX) / (float)paddleWidth;
        hitPos = hitPos * 2 - 1; // 归一化到 -1 到 1
        balls[ballIdx].vx = hitPos * 1.5f;
        
        // 确保垂直速度不为零
        if (abs(balls[ballIdx].vy) < 0.5f) {
            balls[ballIdx].vy = -1.0f;
        }
        
        balls[ballIdx].y = ARK_PADDLE_Y - ARK_BALL_SIZE;
    }
}

void ArkanoidPage::destroyBrick(int row, int col) {
    bricks[row][col]--;
    
    if (bricks[row][col] <= BRICK_NONE) {
        bricksRemaining--;
        combo++;
        
        // 计算分数（连击加成）
        int points = 10 * combo;
        score += points;
        
        // 击中特效
        int brickX = col * (ARK_BRICK_WIDTH + ARK_BRICK_GAP) + 4; // 居中偏移
        int brickY = ARK_BRICK_TOP + row * (ARK_BRICK_HEIGHT + ARK_BRICK_GAP);
        hitEffectX = brickX + ARK_BRICK_WIDTH / 2.0f;
        hitEffectY = brickY + ARK_BRICK_HEIGHT / 2.0f;
        hitEffectTimer = 10;
        
        // 随机生成道具（15%概率）
        if (random(100) < 15 && !powerUp.active) {
            spawnPowerUp(brickX + ARK_BRICK_WIDTH / 2, brickY);
        }
    }
}

void ArkanoidPage::spawnPowerUp(int x, int y) {
    powerUp.x = x - 4; // 道具框8x8，居中
    powerUp.y = y;
    powerUp.active = true;
    
    // 随机道具类型（增加了新道具）
    int rand = random(100);
    if (rand < 18) {
        powerUp.type = POWERUP_EXPAND; // L
    } else if (rand < 30) {
        powerUp.type = POWERUP_SHRINK; // S
    } else if (rand < 45) {
        powerUp.type = POWERUP_SLOW; // W
    } else if (rand < 55) {
        powerUp.type = POWERUP_FAST; // F
    } else if (rand < 65) {
        powerUp.type = POWERUP_LIFE; // ♥
    } else if (rand < 78) {
        powerUp.type = POWERUP_SPEED_UP; // M
    } else if (rand < 90) {
        powerUp.type = POWERUP_GUN; // G
    } else {
        powerUp.type = POWERUP_MULTIBALL; // B
    }
}

void ArkanoidPage::applyPowerUp() {
    switch (powerUp.type) {
        case POWERUP_EXPAND:
            paddleWidth = min(40, ARK_PADDLE_WIDTH + 12);
            activeExpand = true;
            expandTimer = 600; // 10秒
            activeShrink = false;
            shrinkTimer = 0;
            break;
            
        case POWERUP_SHRINK:
            paddleWidth = max(10, ARK_PADDLE_WIDTH - 6);
            activeShrink = true;
            shrinkTimer = 600;
            activeExpand = false;
            expandTimer = 0;
            break;
            
        case POWERUP_SLOW:
            ballSpeed = 1.0f;
            activeSlow = true;
            slowTimer = 600;
            activeFast = false;
            fastTimer = 0;
            break;
            
        case POWERUP_FAST:
            ballSpeed = 2.2f;
            activeFast = true;
            fastTimer = 600;
            activeSlow = false;
            slowTimer = 0;
            break;
            
        case POWERUP_LIFE:
            lives++;
            break;
            
        case POWERUP_SPEED_UP:
            paddleSpeed = 5;
            activeSpeedUp = true;
            speedUpTimer = 600;
            break;
            
        case POWERUP_GUN:
            hasGun = true;
            activeGun = true;
            gunTimer = 300; // 子弹威力大，时间减半（5秒）
            break;
            
        case POWERUP_MULTIBALL:
            // 激活额外的球
            for (int i = 1; i < MAX_BALLS; i++) {
                if (!balls[i].active) {
                    balls[i].active = true;
                    balls[i].x = balls[0].x;
                    balls[i].y = balls[0].y;
                    // 不同方向发射
                    balls[i].vx = (i == 1) ? -0.8f : 0.8f;
                    balls[i].vy = -1.0f;
                }
            }
            break;
            
        default:
            break;
    }
}

void ArkanoidPage::nextLevel() {
    level++;
    combo = 0;
    
    // 稍微增加难度
    if (ballSpeed < 2.0f) {
        ballSpeed += 0.1f;
    }
    
    initLevel();
    gameState = PLAYING;
}

void ArkanoidPage::movePaddleLeft() {
    paddleX -= paddleSpeed;
    if (paddleX < 0) {
        paddleX = 0;
    }
}

void ArkanoidPage::movePaddleRight() {
    paddleX += paddleSpeed;
    if (paddleX > (float)ARK_BOARD_WIDTH - paddleWidth) {
        paddleX = (float)ARK_BOARD_WIDTH - paddleWidth;
    }
}

void ArkanoidPage::launchBall() {
    if (!ballLaunched) {
        ballLaunched = true;
        balls[0].vx = random(-100, 100) / 100.0f;
        balls[0].vy = -1.0f;
    }
}

void ArkanoidPage::fireBullet() {
    if (!hasGun) return;
    
    // 从两个炮管位置同时发射两颗子弹
    int bulletsFired = 0;
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].active = true;
            // 第一颗子弹从左炮管，第二颗从右炮管
            if (bulletsFired == 0) {
                bullets[i].x = paddleX + 3; // 左炮管位置
            } else {
                bullets[i].x = paddleX + paddleWidth - 3; // 右炮管位置
            }
            bullets[i].y = ARK_PADDLE_Y - 3;
            bulletsFired++;
            
            if (bulletsFired >= 2) break; // 最多发射2颗
        }
    }
}

// ========== 绘制函数 ==========

void ArkanoidPage::drawBricks(U8G2* u8g2) {
    for (int row = 0; row < ARK_BRICK_ROWS; row++) {
        for (int col = 0; col < ARK_BRICK_COLS; col++) {
            if (bricks[row][col] == BRICK_NONE) continue;
            
            int x = col * (ARK_BRICK_WIDTH + ARK_BRICK_GAP) + 4; // 居中偏移（128 - 120 = 8，左右各4像素）
            int y = ARK_BRICK_TOP + row * (ARK_BRICK_HEIGHT + ARK_BRICK_GAP);
            
            // 根据砖块类型绘制不同样式
            switch (bricks[row][col]) {
                case BRICK_NORMAL:
                    u8g2->drawBox(x, y, ARK_BRICK_WIDTH, ARK_BRICK_HEIGHT);
                    break;
                    
                case BRICK_HARD:
                    u8g2->drawFrame(x, y, ARK_BRICK_WIDTH, ARK_BRICK_HEIGHT);
                    u8g2->drawBox(x + 2, y + 1, ARK_BRICK_WIDTH - 4, ARK_BRICK_HEIGHT - 2);
                    break;
                    
                case BRICK_SOLID:
                    u8g2->drawFrame(x, y, ARK_BRICK_WIDTH, ARK_BRICK_HEIGHT);
                    u8g2->drawFrame(x + 1, y + 1, ARK_BRICK_WIDTH - 2, ARK_BRICK_HEIGHT - 2);
                    u8g2->drawBox(x + 3, y + 2, ARK_BRICK_WIDTH - 6, ARK_BRICK_HEIGHT - 4);
                    break;
                    
                case BRICK_INDESTRUCTIBLE:
                    // 绘制X图案
                    u8g2->drawBox(x, y, ARK_BRICK_WIDTH, ARK_BRICK_HEIGHT);
                    u8g2->drawLine(x, y, x + ARK_BRICK_WIDTH - 1, y + ARK_BRICK_HEIGHT - 1);
                    u8g2->drawLine(x + ARK_BRICK_WIDTH - 1, y, x, y + ARK_BRICK_HEIGHT - 1);
                    break;
            }
        }
    }
}

void ArkanoidPage::drawPaddle(U8G2* u8g2) {
    // 绘制挡板，带有装饰边框
    u8g2->drawBox((int)paddleX, ARK_PADDLE_Y, paddleWidth, ARK_PADDLE_HEIGHT);
    
    // 如果有武器道具，绘制炮管
    if (hasGun) {
        u8g2->drawLine((int)paddleX + 3, ARK_PADDLE_Y - 1, (int)paddleX + 3, ARK_PADDLE_Y - 3);
        u8g2->drawLine((int)paddleX + paddleWidth - 3, ARK_PADDLE_Y - 1, (int)paddleX + paddleWidth - 3, ARK_PADDLE_Y - 3);
    }
}

void ArkanoidPage::drawBalls(U8G2* u8g2) {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (!balls[i].active) continue;
        
        // 绘制球
        u8g2->drawBox((int)balls[i].x, (int)balls[i].y, ARK_BALL_SIZE, ARK_BALL_SIZE);
        
        // 绘制拖尾效果
        if (ballLaunched && frameCount % 2 == 0) {
            int tailX = (int)(balls[i].x - balls[i].vx);
            int tailY = (int)(balls[i].y - balls[i].vy);
            if (tailX >= 0 && tailX < ARK_BOARD_WIDTH && tailY >= 0 && tailY < SCREEN_HEIGHT) {
                u8g2->drawPixel(tailX + 1, tailY + 1);
            }
        }
    }
}

void ArkanoidPage::drawBullets(U8G2* u8g2) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        
        // 绘制子弹（垂直线）
        u8g2->drawLine((int)bullets[i].x, (int)bullets[i].y, (int)bullets[i].x, (int)bullets[i].y + 3);
    }
}

void ArkanoidPage::drawPowerUp(U8G2* u8g2) {
    if (!powerUp.active) return;
    
    // 绘制道具框（8x8像素，留出字母显示空间）
    u8g2->drawFrame(powerUp.x, powerUp.y, 8, 8);
    
    // 根据道具类型绘制字母标识
    u8g2->setFont(u8g2_font_4x6_tf);
    const char* letter = "";
    
    switch (powerUp.type) {
        case POWERUP_EXPAND:
            letter = "L";
            break;
            
        case POWERUP_SHRINK:
            letter = "S";
            break;
            
        case POWERUP_SLOW:
            letter = "W";
            break;
            
        case POWERUP_FAST:
            letter = "F";
            break;
            
        case POWERUP_LIFE:
            // 绘制心形（保持爱心图标，居中）
            u8g2->drawPixel(powerUp.x + 3, powerUp.y + 2);
            u8g2->drawPixel(powerUp.x + 5, powerUp.y + 2);
            u8g2->drawLine(powerUp.x + 2, powerUp.y + 3, powerUp.x + 6, powerUp.y + 3);
            u8g2->drawLine(powerUp.x + 2, powerUp.y + 4, powerUp.x + 6, powerUp.y + 4);
            u8g2->drawLine(powerUp.x + 3, powerUp.y + 5, powerUp.x + 5, powerUp.y + 5);
            u8g2->drawPixel(powerUp.x + 4, powerUp.y + 6);
            return; // 不绘制字母
            
        case POWERUP_SPEED_UP:
            letter = "M";
            break;
            
        case POWERUP_GUN:
            letter = "G";
            break;
            
        case POWERUP_MULTIBALL:
            letter = "B";
            break;
            
        default:
            letter = "?";
            break;
    }
    
    // 绘制字母（居中，留出边框空间）
    u8g2->drawStr(powerUp.x + 2, powerUp.y + 6, letter);
}

void ArkanoidPage::drawHitEffect(U8G2* u8g2) {
    if (hitEffectTimer <= 0) return;
    
    // 绘制扩散的圆形特效
    int radius = (10 - hitEffectTimer) / 2;
    u8g2->drawCircle(hitEffectX, hitEffectY, radius);
}

void ArkanoidPage::drawHUD(U8G2* u8g2) {
    u8g2->setFont(u8g2_font_5x8_tf);
    
    // 绘制分数
    String scoreText = "SC:" + String(score);
    u8g2->drawUTF8(0, SCREEN_HEIGHT - 1, scoreText.c_str());
    
    // 绘制生命值
    int lifeX = 35;
    for (int i = 0; i < lives; i++) {
        // 绘制心形
        u8g2->drawPixel(lifeX + i * 6 + 1, SCREEN_HEIGHT - 7);
        u8g2->drawPixel(lifeX + i * 6 + 3, SCREEN_HEIGHT - 7);
        u8g2->drawLine(lifeX + i * 6, SCREEN_HEIGHT - 6, lifeX + i * 6 + 4, SCREEN_HEIGHT - 6);
        u8g2->drawLine(lifeX + i * 6, SCREEN_HEIGHT - 5, lifeX + i * 6 + 4, SCREEN_HEIGHT - 5);
        u8g2->drawLine(lifeX + i * 6 + 1, SCREEN_HEIGHT - 4, lifeX + i * 6 + 3, SCREEN_HEIGHT - 4);
        u8g2->drawPixel(lifeX + i * 6 + 2, SCREEN_HEIGHT - 3);
    }
    
    // 绘制关卡
    String levelText = "LV:" + String(level);
    u8g2->drawUTF8(75, SCREEN_HEIGHT - 1, levelText.c_str());
    
    // 绘制连击
    if (combo > 1) {
        String comboText = "x" + String(combo);
        u8g2->drawUTF8(100, SCREEN_HEIGHT - 1, comboText.c_str());
    }
    
    // 绘制道具状态（底部右侧，与其他HUD在同一行）
    u8g2->setFont(u8g2_font_4x6_tf);
    int statusX = 122;
    int statusY = SCREEN_HEIGHT - 2;
    
    if (activeGun) {
        u8g2->drawStr(statusX, statusY, "G");
        statusX -= 5;
    }
    if (activeSpeedUp) {
        u8g2->drawStr(statusX, statusY, "M");
        statusX -= 5;
    }
    if (activeFast) {
        u8g2->drawStr(statusX, statusY, "F");
        statusX -= 5;
    }
    if (activeSlow) {
        u8g2->drawStr(statusX, statusY, "W");
        statusX -= 5;
    }
    if (activeShrink) {
        u8g2->drawStr(statusX, statusY, "S");
        statusX -= 5;
    }
    if (activeExpand) {
        u8g2->drawStr(statusX, statusY, "L");
        statusX -= 5;
    }
}

void ArkanoidPage::drawStartScreen(U8G2* u8g2) {
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    
    // 绘制标题 - 基线位置18像素
    String title = "打砖块";
    int titleWidth = u8g2->getUTF8Width(title.c_str());
    u8g2->drawUTF8((SCREEN_WIDTH - titleWidth) / 2, 18, title.c_str());
    
    // 绘制装饰砖块 - 标题下方6像素开始
    for (int i = 0; i < 8; i++) {
        int x = 20 + i * 12;
        u8g2->drawBox(x, 26, 10, 4);
    }
    
    // 绘制小挡板和球 - 砖块下方6像素
    u8g2->drawBox((int)SCREEN_WIDTH / 2 - 10, 40, 20, 3);
    u8g2->drawBox((int)SCREEN_WIDTH / 2 - 1, 36, 3, 3);
    
    // 绘制开始提示 - 挡板下方8像素，基线位置56
    String startText = "按确认键开始";
    int textWidth = u8g2->getUTF8Width(startText.c_str());
    u8g2->drawUTF8((SCREEN_WIDTH - textWidth) / 2, 56, startText.c_str());
}

void ArkanoidPage::drawGameOverScreen(U8G2* u8g2) {
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

void ArkanoidPage::drawPausedScreen(U8G2* u8g2) {
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    
    // "暂停"文字居中 - 背景框从20开始，高度14，文字基线32
    String pausedText = "暂停";
    int textWidth = u8g2->getUTF8Width(pausedText.c_str());
    u8g2->drawBox((SCREEN_WIDTH - textWidth) / 2 - 2, 20, textWidth + 4, 14);
    u8g2->setDrawColor(0);
    u8g2->drawUTF8((SCREEN_WIDTH - textWidth) / 2, 32, pausedText.c_str());
    u8g2->setDrawColor(1);
}

void ArkanoidPage::drawLevelClearScreen(U8G2* u8g2) {
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    
    // "关卡完成"文字 - 基线位置20
    String clearText = "关卡完成!";
    int textWidth = u8g2->getUTF8Width(clearText.c_str());
    u8g2->drawUTF8((SCREEN_WIDTH - textWidth) / 2, 20, clearText.c_str());
    
    // "第X关"文字 - 与上一行间隔14像素，基线位置34
    String levelText = "第 " + String(level) + " 关";
    textWidth = u8g2->getUTF8Width(levelText.c_str());
    u8g2->drawUTF8((SCREEN_WIDTH - textWidth) / 2, 34, levelText.c_str());
    
    // 绘制星星装饰 - 在文字下方6像素
    if (frameCount % 20 < 10) {
        u8g2->drawTriangle(30, 44, 28, 48, 32, 48);
        u8g2->drawTriangle(98, 44, 96, 48, 100, 48);
    }
    
    // "进入下关"提示 - 与装饰间隔8像素，基线位置58
    String nextText = "按确认进入下关";
    textWidth = u8g2->getUTF8Width(nextText.c_str());
    u8g2->drawUTF8((SCREEN_WIDTH - textWidth) / 2, 58, nextText.c_str());
}

// ========== 按钮事件处理 ==========

void ArkanoidPage::onButtonBack(void* context) {
    if (gameState == PLAYING) {
        gameState = PAUSED;
    } else {
        uiEngine.navigateBack();
    }
}

void ArkanoidPage::onButtonMenu(void* context) {
    if (gameState == PLAYING) {
        movePaddleLeft();
    }
}

void ArkanoidPage::onButtonEnter(void* context) {
    switch (gameState) {
        case START:
            gameState = PLAYING;
            break;
            
        case PLAYING:
            if (hasGun) {
                fireBullet();
            } else {
                launchBall();
            }
            break;
            
        case PAUSED:
            gameState = PLAYING;
            break;
            
        case LEVEL_CLEAR:
            nextLevel();
            break;
            
        case GAME_OVER:
            initGame();
            gameState = PLAYING;
            break;
    }
}

void ArkanoidPage::onButton1(void* context) {
    if (gameState == PLAYING) {
        movePaddleLeft();
    }
}

void ArkanoidPage::onButton2(void* context) {
    if (gameState == PLAYING) {
        if (hasGun) {
            fireBullet();
        } else {
            launchBall();
        }
    }
}

void ArkanoidPage::onButton3(void* context) {
    if (gameState == PLAYING) {
        movePaddleRight();
    }
}

void ArkanoidPage::onButton4(void* context) {
    if (gameState == PLAYING) {
        movePaddleLeft();
    }
}

void ArkanoidPage::onButton5(void* context) {
    if (gameState == PLAYING) {
        if (hasGun) {
            fireBullet();
        } else {
            launchBall();
        }
    }
}

void ArkanoidPage::onButton6(void* context) {
    if (gameState == PLAYING) {
        movePaddleRight();
    }
}

void ArkanoidPage::onButton7(void* context) {
    if (gameState == PLAYING) {
        movePaddleLeft();
    }
}

void ArkanoidPage::onButton8(void* context) {
    if (gameState == PLAYING) {
        movePaddleRight();
    }
}

void ArkanoidPage::onButton9(void* context) {
    if (gameState == PLAYING) {
        movePaddleRight();
    }
}
