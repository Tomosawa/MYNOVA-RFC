/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// RacingPage.cpp
#include "RacingPage.h"
#include "../GUI/UIEngine.h"
#include "../ButtonDetector.h"
#include "clib/u8g2.h"
#include <Arduino.h>

extern UIEngine uiEngine;

RacingPage::RacingPage() : UIPage(0, 0, RACING_SCREEN_WIDTH, RACING_SCREEN_HEIGHT) {
    initGame();
}

RacingPage::~RacingPage() {
    ButtonDetector::setFastResponseMode(false);
    ButtonDetector::setLongPressEnabled(false);
}

void RacingPage::showPage() {
    ButtonDetector::setFastResponseMode(true);
    ButtonDetector::setLongPressEnabled(true);
}

void RacingPage::initGame() {
    gameState = START;
    
    // 玩家初始化
    playerX = 0.0f; // 从道路中央开始
    playerSpeed = 0.0f;
    playerTargetSpeed = 0.0f;
    playerLane = 1; // 保留变量但不使用
    crashed = false;
    crashTimer = 0;
    
    // 世界初始化
    cameraZ = 0.0f;
    roadCurve = 0.0f;
    roadCurveTarget = 0.0f;
    
    // 游戏数据初始化
    score = 0;
    distance = 0;
    speed = 0;
    fuel = 100;
    coins = 0;
    countdown = 90; // 1.5秒倒计时（60帧/秒，加快速度）
    
    // 清空障碍物
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i].active = false;
    }
    
    // 清空粒子
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }
    
    frameCount = 0;
    roadAnimOffset = 0;
}

void RacingPage::render(U8G2* u8g2) {
    u8g2->clearBuffer();
    
    switch (gameState) {
        case START:
            drawStartScreen(u8g2);
            break;
            
        case COUNTDOWN:
            drawRoad(u8g2);
            drawObstacles(u8g2);
            drawPlayer(u8g2);
            drawHUD(u8g2);
            drawCountdown(u8g2);
            break;
            
        case PLAYING:
            drawRoad(u8g2);
            drawObstacles(u8g2);
            drawPlayer(u8g2);
            drawParticles(u8g2);
            drawHUD(u8g2);
            drawSpeedometer(u8g2);
            break;
            
        case PAUSED:
            drawRoad(u8g2);
            drawObstacles(u8g2);
            drawPlayer(u8g2);
            drawHUD(u8g2);
            drawPausedScreen(u8g2);
            break;
            
        case GAME_OVER:
            drawRoad(u8g2);
            drawObstacles(u8g2);
            drawPlayer(u8g2);
            drawHUD(u8g2);
            drawGameOverScreen(u8g2);
            break;
    }
    
    u8g2->sendBuffer();
}

void RacingPage::update() {
    frameCount++;
    
    switch (gameState) {
        case COUNTDOWN:
            countdown--;
            if (countdown <= 0) {
                gameState = PLAYING;
            }
            break;
            
        case PLAYING:
            updateGame();
            break;
            
        default:
            break;
    }
}

void RacingPage::updateGame() {
    // 更新玩家
    updatePlayer();
    
    // 更新摄像机
    updateCamera();
    
    // 更新障碍物
    updateObstacles();
    
    // 更新粒子
    updateParticles();
    
    // 生成障碍物
    if (frameCount % 45 == 0) { // 每0.75秒生成
        generateObstacles();
    }
    
    // 检查碰撞
    if (!crashed) {
        checkCollision();
    }
    
    // 更新道路动画
    if (playerSpeed > 0.1f) {
        roadAnimOffset = (roadAnimOffset + (int)(playerSpeed * 10)) % 8;
    }
    
    // 消耗燃料
    if (frameCount % 20 == 0 && fuel > 0) {
        fuel--;
    }
    
    // 燃料耗尽
    if (fuel <= 0) {
        playerTargetSpeed = 0.0f;
        if (playerSpeed < 0.01f) {
            gameState = GAME_OVER;
        }
    }
    
    // 撞车计时
    if (crashed) {
        crashTimer--;
        if (crashTimer <= 0) {
            crashed = false;
            playerSpeed = playerTargetSpeed * 0.3f; // 重启后速度降低
        }
    }
}

void RacingPage::updatePlayer() {
    // 平滑速度过渡
    if (!crashed) {
        playerSpeed += (playerTargetSpeed - playerSpeed) * 0.1f;
    } else {
        playerSpeed *= 0.95f; // 撞车后快速减速
    }
    
    // 自由移动模式，不再有固定车道
    // playerX 直接通过按键控制，范围由道路边界限制
    
    // 检查道路边界碰撞（会自动限制playerX范围）
    checkRoadBoundary();
    
    // 更新显示速度
    speed = (int)(playerSpeed * 200.0f); // 转换为km/h显示
    
    // 更新距离和分数
    if (!crashed) {
        distance += (int)(playerSpeed * 10.0f);
        score += (int)(playerSpeed * 2.0f);
    }
}

void RacingPage::updateCamera() {
    // 摄像机跟随玩家移动
    cameraZ += playerSpeed;
    
    // 生成动态弯道（更温和的弯道）
    if (frameCount % 120 == 0) {
        // 随机生成新的弯道（减小弯曲度）
        int rnd = random(100);
        if (rnd < 30) {
            roadCurveTarget = random(-8, -3) / 100.0f; // 左弯（减小）
        } else if (rnd < 60) {
            roadCurveTarget = random(3, 8) / 100.0f; // 右弯（减小）
        } else {
            roadCurveTarget = 0.0f; // 直道
        }
    }
    
    // 平滑弯道过渡
    roadCurve += (roadCurveTarget - roadCurve) * 0.02f;
}

void RacingPage::updateObstacles() {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;
        
        // 障碍物相对摄像机的位置
        float relativeZ = obstacles[i].z - cameraZ;
        
        // 如果障碍物在摄像机后面，移除
        if (relativeZ < -2.0f) {
            obstacles[i].active = false;
        }
    }
}

void RacingPage::updateParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].life--;
        
        if (particles[i].life <= 0) {
            particles[i].active = false;
        }
    }
}

void RacingPage::generateObstacles() {
    // 查找空闲槽位
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) {
            obstacles[i].active = true;
            obstacles[i].z = cameraZ + 50.0f; // 在前方生成
            
            // 智能车道选择：避免左右并排
            // 检查是否有其他车在附近
            bool leftOccupied = false;
            bool rightOccupied = false;
            
            for (int j = 0; j < MAX_OBSTACLES; j++) {
                if (j != i && obstacles[j].active) {
                    float zDiff = abs(obstacles[j].z - obstacles[i].z);
                    if (zDiff < 15.0f) { // 附近范围
                        if (obstacles[j].lane == 0) leftOccupied = true;
                        if (obstacles[j].lane == 2) rightOccupied = true;
                    }
                }
            }
            
            // 选择车道：优先选择空闲的车道
            if (leftOccupied && !rightOccupied) {
                obstacles[i].lane = 2; // 右车道
            } else if (rightOccupied && !leftOccupied) {
                obstacles[i].lane = 0; // 左车道
            } else {
                // 都空或都占，随机选择
                obstacles[i].lane = (random(2) == 0) ? 0 : 2;
            }
            
            // 车辆位置：靠近边缘，不压中线
            obstacles[i].x = (obstacles[i].lane == 0) ? -0.35f : 0.35f;
            
            // 随机类型
            int rnd = random(100);
            if (rnd < 50) {
                obstacles[i].type = OBS_CAR;
            } else if (rnd < 70) {
                obstacles[i].type = OBS_TRUCK;
            } else if (rnd < 85) {
                obstacles[i].type = OBS_OIL;
            } else {
                obstacles[i].type = OBS_COIN;
            }
            
            break; // 一次只生成一个
        }
    }
}

void RacingPage::checkCollision() {
    // 玩家车辆的屏幕位置和尺寸
    int playerScreenX = 64 + (int)(playerX * 25.0f);
    int playerScreenY = 56;
    
    // 玩家车辆的外接矩形（根据实际绘制的车辆大小）
    int playerLeft = playerScreenX - 6;
    int playerRight = playerScreenX + 6;
    int playerTop = playerScreenY - 12;
    int playerBottom = playerScreenY + 3;
    
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;
        
        float relativeZ = obstacles[i].z - cameraZ;
        
        // 只检查屏幕上可见的障碍物
        if (relativeZ < 0.5f || relativeZ > 50.0f) continue;
        
        // 计算障碍物的屏幕Y位置
        int obsScreenY = 64 - (int)(relativeZ * 1.2f);
        if (obsScreenY < 10 || obsScreenY >= 58) continue;
        
        // 计算障碍物的屏幕X位置
        float depthFactor = (float)(obsScreenY - 8) / 55.0f;
        if (depthFactor < 0.0f) depthFactor = 0.0f;
        if (depthFactor > 1.0f) depthFactor = 1.0f;
        
        float curve = getCurveAtZ(obstacles[i].z);
        int roadCenterX = 64 + (int)(curve * 20.0f * depthFactor);
        int laneOffsetX = (int)(obstacles[i].x * 40.0f * depthFactor);
        int obsScreenX = roadCenterX + laneOffsetX;
        
        // 计算障碍物大小
        int objSize = (int)(8.0f * depthFactor);
        if (objSize < 2) objSize = 2;
        
        // 障碍物外接矩形（根据类型）
        int obsLeft, obsRight, obsTop, obsBottom;
        
        switch (obstacles[i].type) {
            case OBS_CAR:
                obsLeft = obsScreenX - objSize/2;
                obsRight = obsScreenX + objSize/2;
                obsTop = obsScreenY - objSize;
                obsBottom = obsScreenY;
                break;
                
            case OBS_TRUCK:
                obsLeft = obsScreenX - objSize;
                obsRight = obsScreenX + objSize;
                obsTop = obsScreenY - objSize*3/2;
                obsBottom = obsScreenY;
                break;
                
            case OBS_OIL:
                obsLeft = obsScreenX - objSize/2;
                obsRight = obsScreenX + objSize/2;
                obsTop = obsScreenY - objSize/2;
                obsBottom = obsScreenY + objSize/2;
                break;
                
            case OBS_COIN:
                obsLeft = obsScreenX - objSize/2;
                obsRight = obsScreenX + objSize/2;
                obsTop = obsScreenY - objSize;
                obsBottom = obsScreenY;
                break;
                
            default:
                continue;
        }
        
        // 矩形碰撞检测（AABB）
        bool collision = !(playerRight < obsLeft || 
                          playerLeft > obsRight || 
                          playerBottom < obsTop || 
                          playerTop > obsBottom);
        
        if (collision) {
            switch (obstacles[i].type) {
                case OBS_CAR:
                case OBS_TRUCK:
                    // 撞车
                    crashed = true;
                    crashTimer = 60; // 1秒
                    playerSpeed *= 0.2f;
                    spawnParticles(64, 50, 10);
                    score = max(0, score - 50);
                    break;
                    
                case OBS_OIL:
                    // 打滑减速
                    playerSpeed *= 0.5f;
                    spawnParticles(64, 50, 5);
                    break;
                    
                case OBS_COIN:
                    // 收集金币
                    coins++;
                    score += 20;
                    fuel = min(100, fuel + 5);
                    break;
            }
            
            obstacles[i].active = false;
        }
    }
}

void RacingPage::checkRoadBoundary() {
    // 玩家车辆的Y位置（固定在屏幕下方）
    int playerScreenY = 56;
    
    // 计算当前位置的透视因子
    float depthFactor = (float)(playerScreenY - 8) / 55.0f;
    if (depthFactor < 0.0f) depthFactor = 0.0f;
    if (depthFactor > 1.0f) depthFactor = 1.0f;
    
    // 道路宽度（近处）
    int roadWidth = 20 + (int)(80.0f * depthFactor);
    
    // 计算道路中心位置（考虑弯道）
    float worldZ = (63 - playerScreenY) * 1.5f;
    float curve = getCurveAtZ(cameraZ + worldZ);
    int roadCenterX = 64 + (int)(curve * 20.0f * depthFactor);
    
    // 道路边界（像素坐标）
    int roadLeft = roadCenterX - roadWidth / 2;
    int roadRight = roadCenterX + roadWidth / 2;
    
    // 将playerX从世界坐标转换为屏幕坐标
    int playerScreenX = 64 + (int)(playerX * 25.0f);
    
    // 玩家车辆的宽度（左右各6像素）
    int playerCarLeft = playerScreenX - 6;
    int playerCarRight = playerScreenX + 6;
    
    // 计算允许的X范围（世界坐标）
    // 左边界：roadLeft + 6（车宽的一半）
    // 右边界：roadRight - 6（车宽的一半）
    float minX = (roadLeft + 6 - 64) / 25.0f;
    float maxX = (roadRight - 6 - 64) / 25.0f;
    
    // 限制playerX在道路范围内
    if (playerX < minX) {
        playerX = minX;
        // 减速惩罚
        if (!crashed && playerSpeed > 0.1f) {
            playerSpeed *= 0.7f;
        }
    }
    
    if (playerX > maxX) {
        playerX = maxX;
        // 减速惩罚
        if (!crashed && playerSpeed > 0.1f) {
            playerSpeed *= 0.7f;
        }
    }
}

void RacingPage::spawnParticles(float x, float y, int count) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < MAX_PARTICLES; j++) {
            if (!particles[j].active) {
                particles[j].active = true;
                particles[j].x = x + random(-5, 5);
                particles[j].y = y + random(-5, 5);
                particles[j].vx = random(-20, 20) / 10.0f;
                particles[j].vy = random(-20, 20) / 10.0f;
                particles[j].life = random(10, 20);
                break;
            }
        }
    }
}

// ========== 绘制函数 ==========

void RacingPage::drawRoad(U8G2* u8g2) {
    // 绘制道路（从远到近，从上到下）
    // 道路越远（上方）越窄，越近（下方）越宽
    for (int y = 8; y < 64; y++) {
        drawRoadSegment(u8g2, y, cameraZ);
    }
}

void RacingPage::drawRoadSegment(U8G2* u8g2, int screenY, float camZ) {
    // 透视计算：上方（y小）= 远处 = 窄，下方（y大）= 近处 = 宽
    // 梯形：上窄下宽
    float depthFactor = (float)(screenY - 8) / 55.0f; // 0.0（上/远）~ 1.0（下/近）
    if (depthFactor < 0.0f) depthFactor = 0.0f;
    if (depthFactor > 1.0f) depthFactor = 1.0f;
    
    // 道路宽度：上方窄（最小20），下方宽（最大100）
    int roadWidth = 20 + (int)(80.0f * depthFactor);
    
    // 计算这一行对应的世界深度
    float worldZ = (63 - screenY) * 1.5f;
    float curve = getCurveAtZ(camZ + worldZ);
    
    // 道路中心（考虑弯曲）
    int roadCenterX = 64 + (int)(curve * 20.0f * depthFactor);
    
    int roadLeft = roadCenterX - roadWidth / 2;
    int roadRight = roadCenterX + roadWidth / 2;
    
    // 限制不超出屏幕
    if (roadLeft < 0) roadLeft = 0;
    if (roadRight > 127) roadRight = 127;
    
    // 绘制左侧草地（填充点状纹理）
    for (int x = 0; x < roadLeft; x += 2) {
        if ((x + screenY + roadAnimOffset) % 4 == 0) {
            u8g2->drawPixel(x, screenY);
        }
    }
    
    // 绘制右侧草地（填充点状纹理）
    for (int x = roadRight + 1; x < 128; x += 2) {
        if ((x + screenY + roadAnimOffset) % 4 == 0) {
            u8g2->drawPixel(x, screenY);
        }
    }
    
    // 绘制道路左边线（白色实线）
    u8g2->drawPixel(roadLeft, screenY);
    
    // 绘制道路右边线（白色实线）
    u8g2->drawPixel(roadRight, screenY);
    
    // 道路中间是空白的（可行驶区域）
    
    // 绘制中心虚线（只有一条，2车道）
    int dashPos = (int)(camZ * 10.0f + screenY * 2) % 8;
    if (dashPos < 4 && roadWidth > 15) {
        u8g2->drawPixel(roadCenterX, screenY);
    }
}

void RacingPage::drawObstacles(U8G2* u8g2) {
    // 从远到近绘制障碍物
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;
        
        float relativeZ = obstacles[i].z - cameraZ;
        if (relativeZ < 0.5f || relativeZ > 50.0f) continue;
        
        // 计算屏幕Y位置（透视：远处在上方，近处在下方）
        int screenY = 64 - (int)(relativeZ * 1.2f);
        if (screenY < 10 || screenY >= 58) continue;
        
        // 计算深度因子（梯形透视）
        float depthFactor = (float)(screenY - 8) / 55.0f;
        if (depthFactor < 0.0f) depthFactor = 0.0f;
        if (depthFactor > 1.0f) depthFactor = 1.0f;
        
        // 计算道路中心和障碍物位置
        float curve = getCurveAtZ(obstacles[i].z);
        int roadCenterX = 64 + (int)(curve * 20.0f * depthFactor);
        
        // 车道偏移（2车道）
        int laneOffsetX = (int)(obstacles[i].x * 40.0f * depthFactor);
        int screenX = roadCenterX + laneOffsetX;
        
        // 物体大小随距离变化
        int objSize = (int)(8.0f * depthFactor);
        if (objSize < 2) objSize = 2;
        
        // 根据类型绘制（汽车尾部视角）
        switch (obstacles[i].type) {
            case OBS_CAR:
                // 绘制轿车尾部视角
                if (objSize > 4) {
                    // 车身主体
                    u8g2->drawBox(screenX - objSize/2, screenY - objSize, objSize, objSize);
                    
                    // 尾灯（两个白点）
                    u8g2->setDrawColor(0);
                    u8g2->drawPixel(screenX - objSize/3, screenY - 2);
                    u8g2->drawPixel(screenX + objSize/3, screenY - 2);
                    u8g2->setDrawColor(1);
                    
                    // 后窗（上方的白色矩形）
                    u8g2->setDrawColor(0);
                    u8g2->drawBox(screenX - objSize/3, screenY - objSize + 1, objSize*2/3, objSize/3);
                    u8g2->setDrawColor(1);
                    u8g2->drawFrame(screenX - objSize/3, screenY - objSize + 1, objSize*2/3, objSize/3);
                } else if (objSize > 2) {
                    // 小尺寸：简化版
                    u8g2->drawBox(screenX - objSize/2, screenY - objSize, objSize, objSize);
                    u8g2->setDrawColor(0);
                    u8g2->drawPixel(screenX - 1, screenY - 1);
                    u8g2->drawPixel(screenX + 1, screenY - 1);
                    u8g2->setDrawColor(1);
                } else {
                    // 最小尺寸
                    u8g2->drawBox(screenX - 1, screenY - 2, 2, 2);
                }
                break;
                
            case OBS_TRUCK:
                // 绘制卡车尾部（更大更方）
                if (objSize > 4) {
                    u8g2->drawBox(screenX - objSize, screenY - objSize*3/2, objSize*2, objSize*3/2);
                    // 货箱门
                    u8g2->setDrawColor(0);
                    u8g2->drawBox(screenX - objSize/2, screenY - objSize, objSize, objSize*2/3);
                    u8g2->setDrawColor(1);
                    u8g2->drawFrame(screenX - objSize/2, screenY - objSize, objSize, objSize*2/3);
                } else {
                    u8g2->drawBox(screenX - objSize, screenY - objSize, objSize*2, objSize);
                }
                break;
                
            case OBS_OIL:
                // 绘制油渍（椭圆形污渍）
                if (objSize > 2) {
                    for (int dx = -objSize/2; dx <= objSize/2; dx++) {
                        u8g2->drawPixel(screenX + dx, screenY - 1);
                        if (abs(dx) < objSize/3) {
                            u8g2->drawPixel(screenX + dx, screenY);
                            u8g2->drawPixel(screenX + dx, screenY - 2);
                        }
                    }
                } else {
                    u8g2->drawPixel(screenX, screenY);
                }
                break;
                
            case OBS_COIN:
                // 绘制金币（美元符号$）
                if (frameCount % 10 < 5 || objSize < 4) { // 小尺寸不闪烁
                    if (objSize >= 6) {
                        // 大尺寸：绘制$符号
                        u8g2->setFont(u8g2_font_4x6_tf);
                        u8g2->drawStr(screenX - 2, screenY - objSize/2 + 3, "$");
                    } else if (objSize >= 4) {
                        // 中尺寸：简化$符号
                        u8g2->drawLine(screenX - 1, screenY - objSize, screenX + 1, screenY - objSize);
                        u8g2->drawPixel(screenX, screenY - objSize + 1);
                        u8g2->drawLine(screenX - 1, screenY - objSize + 2, screenX + 1, screenY - objSize + 2);
                        u8g2->drawLine(screenX, screenY - objSize, screenX, screenY - objSize + 2);
                    } else {
                        // 小尺寸：点
                        u8g2->drawBox(screenX - 1, screenY - 1, 2, 2);
                    }
                }
                break;
        }
    }
}

void RacingPage::drawPlayer(U8G2* u8g2) {
    int playerScreenX = 64 + (int)(playerX * 25.0f);
    int playerScreenY = 56;
    
    if (crashed && (frameCount % 4 < 2)) {
        // 撞车闪烁
        return;
    }
    
    // 绘制玩家赛车（F1赛车外观，俯视角度）
    // 车尾扰流板
    u8g2->drawBox(playerScreenX - 5, playerScreenY + 1, 10, 2);
    
    // 后轮（外露）
    u8g2->drawBox(playerScreenX - 6, playerScreenY - 2, 2, 4);
    u8g2->drawBox(playerScreenX + 4, playerScreenY - 2, 2, 4);
    
    // 车身主体（梯形，前窄后宽）
    u8g2->drawBox(playerScreenX - 4, playerScreenY - 8, 8, 10);
    
    // 驾驶舱（内凹效果）
    u8g2->setDrawColor(0);
    u8g2->drawBox(playerScreenX - 2, playerScreenY - 5, 4, 5);
    u8g2->setDrawColor(1);
    
    // 驾驶舱边框
    u8g2->drawFrame(playerScreenX - 2, playerScreenY - 5, 4, 5);
    
    // 前轮（部分遮挡）
    u8g2->drawBox(playerScreenX - 5, playerScreenY - 9, 2, 3);
    u8g2->drawBox(playerScreenX + 3, playerScreenY - 9, 2, 3);
    
    // 车头鼻锥（尖端）
    u8g2->drawLine(playerScreenX - 3, playerScreenY - 9, playerScreenX, playerScreenY - 11);
    u8g2->drawLine(playerScreenX + 3, playerScreenY - 9, playerScreenX, playerScreenY - 11);
    u8g2->drawPixel(playerScreenX, playerScreenY - 12);
    
    // 车身装饰线
    u8g2->drawLine(playerScreenX - 1, playerScreenY - 7, playerScreenX - 1, playerScreenY);
    u8g2->drawLine(playerScreenX + 1, playerScreenY - 7, playerScreenX + 1, playerScreenY);
}

void RacingPage::drawParticles(U8G2* u8g2) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        
        u8g2->drawPixel((int)particles[i].x, (int)particles[i].y);
    }
}

void RacingPage::drawHUD(U8G2* u8g2) {
    u8g2->setFont(u8g2_font_4x6_tf);
    
    // 速度（左上角）
    char speedStr[12];
    snprintf(speedStr, sizeof(speedStr), "%dKM", speed);
    u8g2->drawStr(2, 6, speedStr);
    
    // 距离（中左）
    char distStr[12];
    snprintf(distStr, sizeof(distStr), "%dm", distance / 10);
    u8g2->drawStr(40, 6, distStr);
    
    // 金币（中右）
    char coinStr[8];
    snprintf(coinStr, sizeof(coinStr), "$%d", coins);
    u8g2->drawStr(75, 6, coinStr);
    
    // 燃料条（右上角）
    u8g2->drawFrame(102, 1, 24, 5);
    int fuelWidth = (fuel * 22) / 100;
    if (fuelWidth > 0) {
        u8g2->drawBox(103, 2, fuelWidth, 3);
    }
}

void RacingPage::drawSpeedometer(U8G2* u8g2) {
    // 速度表已经在HUD中显示，这里不需要了
}

void RacingPage::drawStartScreen(U8G2* u8g2) {
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    
    // 标题
    const char* title = "极速赛车";
    int titleWidth = u8g2->getUTF8Width(title);
    u8g2->drawUTF8((128 - titleWidth) / 2, 18, title);
    
    // 绘制赛车图标
    int carX = 64;
    int carY = 32;
    u8g2->drawBox(carX - 6, carY - 4, 12, 8);
    u8g2->setDrawColor(0);
    u8g2->drawBox(carX - 4, carY - 2, 8, 4);
    u8g2->setDrawColor(1);
    u8g2->drawLine(carX - 6, carY - 5, carX - 6, carY - 3);
    u8g2->drawLine(carX + 6, carY - 5, carX + 6, carY - 3);
    u8g2->drawLine(carX - 6, carY + 3, carX - 6, carY + 5);
    u8g2->drawLine(carX + 6, carY + 3, carX + 6, carY + 5);
    
    // 操作说明
    u8g2->setFont(u8g2_font_4x6_tf);
    u8g2->drawStr(20, 48, "4/6:TURN 2/8:SPEED");
    
    // 开始提示
    if (frameCount % 60 < 40) {
        u8g2->setFont(u8g2_font_wqy12_t_gb2312);
        const char* startText = "按确认键开始";
        int startWidth = u8g2->getUTF8Width(startText);
        u8g2->drawUTF8((128 - startWidth) / 2, 62, startText);
    }
}

void RacingPage::drawCountdown(U8G2* u8g2) {
    int countNum = (countdown / 30) + 1; // 改为每30帧一个数字
    if (countNum > 3) return;
    
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    
    // 绘制倒计数字
    char countStr[4];
    snprintf(countStr, sizeof(countStr), "%d", countNum);
    int textWidth = u8g2->getUTF8Width(countStr);
    
    // 简单居中显示
    u8g2->drawUTF8((128 - textWidth) / 2, 36, countStr);
}

void RacingPage::drawGameOverScreen(U8G2* u8g2) {
    // 半透明遮罩
    for (int y = 15; y < 50; y += 2) {
        for (int x = 10 + (y % 4) / 2; x < 118; x += 2) {
            u8g2->setDrawColor(0);
            u8g2->drawPixel(x, y);
        }
    }
    u8g2->setDrawColor(1);
    
    // 游戏结束框
    u8g2->drawFrame(8, 13, 112, 38);
    u8g2->setDrawColor(0);
    u8g2->drawBox(9, 14, 110, 36);
    u8g2->setDrawColor(1);
    u8g2->drawFrame(10, 15, 108, 34);
    
    // 标题
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    const char* gameOverText = "完赛!";
    int textWidth = u8g2->getUTF8Width(gameOverText);
    u8g2->drawUTF8((128 - textWidth) / 2, 28, gameOverText);
    
    // 显示成绩
    u8g2->setFont(u8g2_font_5x7_tf);
    char resultStr[20];
    snprintf(resultStr, sizeof(resultStr), "DIST:%dm", distance / 10);
    int resultWidth = strlen(resultStr) * 5;
    u8g2->drawStr((128 - resultWidth) / 2, 38, resultStr);
    
    // 重新开始
    if (frameCount % 60 < 40) {
        u8g2->setFont(u8g2_font_4x6_tf);
        const char* restartText = "ENTER TO RESTART";
        int restartWidth = strlen(restartText) * 4;
        u8g2->drawStr((128 - restartWidth) / 2, 47, restartText);
    }
}

void RacingPage::drawPausedScreen(U8G2* u8g2) {
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    const char* pauseText = "暂停";
    int textWidth = u8g2->getUTF8Width(pauseText);
    u8g2->drawBox((128 - textWidth) / 2 - 2, 20, textWidth + 4, 14);
    u8g2->setDrawColor(0);
    u8g2->drawUTF8((128 - textWidth) / 2, 32, pauseText);
    u8g2->setDrawColor(1);
}

// ========== 辅助函数 ==========

float RacingPage::getCurveAtZ(float z) {
    // 限制弯道幅度，防止道路偏出屏幕
    float curvature = roadCurve * z * 0.05f; // 降低弯曲系数
    // 限制最大弯曲值
    if (curvature > 0.8f) curvature = 0.8f;
    if (curvature < -0.8f) curvature = -0.8f;
    return curvature;
}

void RacingPage::projectToScreen(float worldX, float worldZ, int& screenX, int& screenY, int& width) {
    // 透视投影
    if (worldZ < 0.1f) worldZ = 0.1f;
    
    float perspective = 50.0f / (worldZ + 5.0f);
    
    screenX = 64 + (int)(worldX * 40.0f * perspective);
    screenY = 20 + (int)(worldZ * 2.0f);
    width = (int)(80.0f * perspective);
    
    // 限制范围
    screenX = constrain(screenX, 0, 127);
    screenY = constrain(screenY, 10, 60);
    width = constrain(width, 2, 100);
}

int RacingPage::getLaneX(int lane) {
    // 返回车道中心X坐标（2车道：0=左，1=右）
    // 实际上只有两个位置：左车道和右车道
    switch (lane) {
        case 0: return 48;  // 左车道
        case 1: return 64;  // 中间（不使用）
        case 2: return 80;  // 右车道
        default: return 64;
    }
}

// ========== 按钮事件 ==========

void RacingPage::onButtonBack(void* context) {
    if (gameState == PLAYING) {
        gameState = PAUSED;
    } else {
        uiEngine.navigateBack();
    }
}

void RacingPage::onButtonEnter(void* context) {
    switch (gameState) {
        case START:
            gameState = COUNTDOWN;
            break;
        case COUNTDOWN:
            // 跳过倒计时
            gameState = PLAYING;
            break;
        case PAUSED:
            gameState = PLAYING;
            break;
        case GAME_OVER:
            initGame();
            gameState = COUNTDOWN;
            break;
    }
}

void RacingPage::onButtonMenu(void* context) {
    if (gameState == PLAYING) {
        gameState = PAUSED;
    }
}

void RacingPage::onButton1(void* context) {
    // 1键 - 向左移动
    if (gameState == PLAYING && !crashed) {
        playerX -= 0.05f; // 自由移动
    }
}

void RacingPage::onButton2(void* context) {
    // 2键 - 加速
    if (gameState == PLAYING && !crashed && fuel > 0) {
        playerTargetSpeed = min(1.0f, playerTargetSpeed + 0.1f);
    }
}

void RacingPage::onButton3(void* context) {
    // 3键 - 向右移动
    if (gameState == PLAYING && !crashed) {
        playerX += 0.05f; // 自由移动
    }
}

void RacingPage::onButton4(void* context) {
    // 4键 - 向左移动
    if (gameState == PLAYING && !crashed) {
        playerX -= 0.05f;
    }
}

void RacingPage::onButton5(void* context) {
    // 5键 - 保持
}

void RacingPage::onButton6(void* context) {
    // 6键 - 向右移动
    if (gameState == PLAYING && !crashed) {
        playerX += 0.05f;
    }
}

void RacingPage::onButton7(void* context) {
    // 7键 - 向左移动
    if (gameState == PLAYING && !crashed) {
        playerX -= 0.05f;
    }
}

void RacingPage::onButton8(void* context) {
    // 8键 - 减速/刹车
    if (gameState == PLAYING && !crashed) {
        playerTargetSpeed = max(0.0f, playerTargetSpeed - 0.1f);
    }
}

void RacingPage::onButton9(void* context) {
    // 9键 - 向右移动
    if (gameState == PLAYING && !crashed) {
        playerX += 0.05f;
    }
}
