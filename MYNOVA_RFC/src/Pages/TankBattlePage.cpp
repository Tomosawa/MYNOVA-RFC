/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// TankBattlePage.cpp
#include "TankBattlePage.h"
#include "../GUI/UIEngine.h"
#include "../ButtonDetector.h"
#include "clib/u8g2.h"

extern UIEngine uiEngine;

TankBattlePage::TankBattlePage() : UIPage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) {
    initGame();
}

TankBattlePage::~TankBattlePage() {
    // 退出游戏页面时，恢复普通按键响应模式
    ButtonDetector::setFastResponseMode(false);
    ButtonDetector::setLongPressEnabled(false);
}

void TankBattlePage::showPage() {
    // 进入游戏页面时，启用快速按键响应模式和长按模式
    ButtonDetector::setFastResponseMode(true);
    ButtonDetector::setLongPressEnabled(true);
}

void TankBattlePage::initGame() {
    gameState = START;
    score = 0;
    level = 1;
    enemiesKilled = 0;
    frameCount = 0;
    explosionTimer = 0;
    baseDestroyed = false;
    
    initLevel();
}

void TankBattlePage::initLevel() {
    // 生成地图
    generateMap(level);
    
    // 初始化玩家坦克 - 固定出生点：底部左侧
    player.x = 9; // 基地左侧
    player.y = TANK_BOARD_HEIGHT - 2;
    player.dir = DIR_UP;
    player.active = true;
    player.hp = 3;
    player.moveCounter = 0;
    player.shootCooldown = 0;
    
    // 初始化敌人
    enemiesAlive = 0;
    enemiesRemaining = TANK_TOTAL_ENEMIES;
    spawnCounter = 0;
    nextSpawnPos = 0;  // 从左侧开始
    for (int i = 0; i < TANK_MAX_ENEMIES; i++) {
        enemies[i].active = false;
    }
    
    // 立即生成初始的3个敌人（按左、中、右顺序）
    for (int i = 0; i < TANK_MAX_ENEMIES; i++) {
        spawnEnemy();
    }
    
    // 初始化子弹
    for (int i = 0; i < TANK_MAX_BULLETS; i++) {
        bullets[i].active = false;
    }
    
    enemiesKilled = 0;
}

void TankBattlePage::generateMap(int levelNum) {
    // 清空地图
    for (int y = 0; y < TANK_BOARD_HEIGHT; y++) {
        for (int x = 0; x < TANK_BOARD_WIDTH; x++) {
            map[y][x] = TILE_EMPTY;
        }
    }
    
    // 基地位置（底部中间）
    baseX = TANK_BOARD_WIDTH / 2;
    baseY = TANK_BOARD_HEIGHT - 1;
    map[baseY][baseX] = TILE_BASE;
    
    // 根据关卡生成不同布局
    switch (levelNum % 3) {
        case 1: // 第一关 - 简单布局
            // 顶部砖墙
            for (int x = 5; x < 21; x++) {
                if (x % 3 != 0) {
                    map[2][x] = TILE_BRICK;
                }
            }
            // 中间障碍
            for (int y = 5; y < 8; y++) {
                map[y][8] = TILE_BRICK;
                map[y][17] = TILE_BRICK;
            }
            // 基地周围保护墙
            map[baseY - 1][baseX - 1] = TILE_BRICK;
            map[baseY - 1][baseX] = TILE_BRICK;
            map[baseY - 1][baseX + 1] = TILE_BRICK;
            map[baseY][baseX - 1] = TILE_BRICK;
            map[baseY][baseX + 1] = TILE_BRICK;
            break;
            
        case 2: // 第二关 - 钢墙布局
            // 钢墙障碍
            for (int x = 3; x < 23; x += 4) {
                for (int y = 3; y < 10; y += 3) {
                    map[y][x] = TILE_STEEL;
                }
            }
            // 基地钢墙保护
            map[baseY - 1][baseX - 1] = TILE_STEEL;
            map[baseY - 1][baseX + 1] = TILE_STEEL;
            break;
            
        case 0: // 第三关及以上 - 复杂布局
            // 迷宫式布局
            for (int x = 4; x < 22; x += 6) {
                for (int y = 1; y < 11; y += 2) {
                    map[y][x] = TILE_BRICK;
                    if (y < 10) {
                        map[y][x + 2] = TILE_BRICK;
                    }
                }
            }
            // 钢墙点缀
            map[4][13] = TILE_STEEL;
            map[7][8] = TILE_STEEL;
            map[7][18] = TILE_STEEL;
            // 基地混合保护
            map[baseY - 1][baseX - 1] = TILE_STEEL;
            map[baseY - 1][baseX] = TILE_BRICK;
            map[baseY - 1][baseX + 1] = TILE_STEEL;
            break;
    }
}

void TankBattlePage::render(U8G2* u8g2) {
    u8g2->clearBuffer();
    
    switch (gameState) {
        case START:
            drawStartScreen(u8g2);
            break;
            
        case PLAYING:
            drawBorder(u8g2);
            drawMap(u8g2);
            drawBullets(u8g2);
            drawTank(u8g2, &player, true);
            for (int i = 0; i < TANK_MAX_ENEMIES; i++) {
                if (enemies[i].active) {
                    drawTank(u8g2, &enemies[i], false);
                }
            }
            drawExplosion(u8g2);
            drawHUD(u8g2);
            break;
            
        case PAUSED:
            drawBorder(u8g2);
            drawMap(u8g2);
            drawTank(u8g2, &player, true);
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

void TankBattlePage::update() {
    frameCount++;
    
    switch (gameState) {
        case PLAYING:
            updateGame();
            break;
        default:
            break;
    }
}

void TankBattlePage::updateGame() {
    updatePlayer();
    updateEnemies();
    updateBullets();
    
    // 生成新敌人 - 快速补充，保持3个在场
    if (enemiesAlive < TANK_MAX_ENEMIES && enemiesRemaining > 0) {
        spawnCounter++;
        if (spawnCounter >= 30) {  // 每0.5秒检查一次
            spawnEnemy();
            spawnCounter = 0;
        }
    }
    
    // 更新爆炸效果
    if (explosionTimer > 0) {
        explosionTimer--;
    }
    
    // 射击冷却
    if (player.shootCooldown > 0) {
        player.shootCooldown--;
    }
    
    // 检查胜利条件
    if (enemiesKilled >= TANK_TOTAL_ENEMIES && enemiesAlive == 0) {
        gameState = LEVEL_CLEAR;
    }
    
    // 检查失败条件
    if (baseDestroyed) {
        gameState = GAME_OVER;
    } else if (player.hp <= 0 && player.active) {
        player.active = false;
        gameState = GAME_OVER;
    } else if (!player.active && player.hp > 0) {
        // 玩家被击中但还有生命，复活在初始位置
        player.x = 2;
        player.y = TANK_BOARD_HEIGHT - 2;
        player.dir = DIR_UP;
        player.active = true;
        player.moveCounter = 0;
        player.shootCooldown = 0;
    }
}

void TankBattlePage::updatePlayer() {
    if (!player.active) return;
    
    if (player.moveCounter > 0) {
        player.moveCounter--;
    }
}

void TankBattlePage::updateEnemies() {
    for (int i = 0; i < TANK_MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;
        
        // 射击冷却
        if (enemies[i].shootCooldown > 0) {
            enemies[i].shootCooldown--;
        }
        
        // AI 控制
        enemies[i].moveCounter++;
        if (enemies[i].moveCounter >= 30) {
            enemyAI(&enemies[i]);
            enemies[i].moveCounter = 0;
        }
    }
}

void TankBattlePage::updateBullets() {
    for (int i = 0; i < TANK_MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        
        // 移动子弹
        float speed = 0.5f;
        switch (bullets[i].dir) {
            case DIR_UP:    bullets[i].y -= speed; break;
            case DIR_DOWN:  bullets[i].y += speed; break;
            case DIR_LEFT:  bullets[i].x -= speed; break;
            case DIR_RIGHT: bullets[i].x += speed; break;
        }
        
        // 检查边界
        if (bullets[i].x < 0 || bullets[i].x >= TANK_BOARD_WIDTH ||
            bullets[i].y < 0 || bullets[i].y >= TANK_BOARD_HEIGHT) {
            bullets[i].active = false;
            continue;
        }
        
        int bx = (int)bullets[i].x;
        int by = (int)bullets[i].y;
        
        // 检查地图碰撞
        if (map[by][bx] != TILE_EMPTY && map[by][bx] != TILE_GRASS) {
            if (map[by][bx] == TILE_BRICK) {
                destroyTile(bx, by);
            } else if (map[by][bx] == TILE_BASE) {
                baseDestroyed = true;
                createExplosion(bx, by);
            }
            bullets[i].active = false;
            continue;
        }
        
        // 检查玩家碰撞
        if (!bullets[i].isPlayer && player.active) {
            if ((int)player.x == bx && (int)player.y == by) {
                player.hp--;
                bullets[i].active = false;
                createExplosion(bx, by);
                if (player.hp <= 0) {
                    player.active = false;
                }
                continue;
            }
        }
        
        // 检查敌人碰撞
        if (bullets[i].isPlayer) {
            for (int j = 0; j < TANK_MAX_ENEMIES; j++) {
                if (enemies[j].active && 
                    (int)enemies[j].x == bx && (int)enemies[j].y == by) {
                    enemies[j].active = false;
                    enemiesAlive--;
                    enemiesKilled++;
                    score += 100;
                    bullets[i].active = false;
                    createExplosion(bx, by);
                    break;
                }
            }
        }
    }
}

void TankBattlePage::spawnEnemy() {
    // 固定三个生成位置：左(2)、中(13)、右(24)
    int spawnPositions[3] = {2, 13, 24};
    
    // 按顺序轮流从三个位置生成
    int spawnX = spawnPositions[nextSpawnPos];
    
    // 找一个空闲槽位
    for (int i = 0; i < TANK_MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].x = spawnX;
            enemies[i].y = 0;
            enemies[i].dir = DIR_DOWN;
            enemies[i].active = true;
            enemies[i].hp = 1;
            enemies[i].moveCounter = 0;
            enemies[i].shootCooldown = 0;
            
            enemiesAlive++;
            enemiesRemaining--;
            
            // 移动到下一个生成位置（循环：0→1→2→0→1→2...）
            nextSpawnPos = (nextSpawnPos + 1) % 3;
            break;
        }
    }
}

void TankBattlePage::enemyAI(Tank* enemy) {
    // 改进AI：更活跃，更智能
    int action = random(100);
    
    if (action < 15) {  // 15% 改变方向
        enemy->dir = (Direction)random(4);
    } else if (action < 70) {  // 55% 前进（更活跃）
        moveTank(enemy, enemy->dir);
    } else if (action < 85 && enemy->shootCooldown == 0) {  // 15% 射击
        shoot(enemy, false);
    }
    // 25% 什么都不做
}

bool TankBattlePage::canMove(float x, float y) {
    int ix = (int)x;
    int iy = (int)y;
    
    // 边界检查
    if (ix < 0 || ix >= TANK_BOARD_WIDTH || iy < 0 || iy >= TANK_BOARD_HEIGHT) {
        return false;
    }
    
    // 地图障碍检查
    uint8_t tile = map[iy][ix];
    if (tile == TILE_BRICK || tile == TILE_STEEL || tile == TILE_BASE) {
        return false;
    }
    
    return true;
}

bool TankBattlePage::checkTankCollision(float x, float y, Tank* exceptTank) {
    int ix = (int)x;
    int iy = (int)y;
    
    // 检查玩家碰撞
    if (&player != exceptTank && player.active) {
        if ((int)player.x == ix && (int)player.y == iy) {
            return true;
        }
    }
    
    // 检查敌人碰撞
    for (int i = 0; i < TANK_MAX_ENEMIES; i++) {
        if (&enemies[i] != exceptTank && enemies[i].active) {
            if ((int)enemies[i].x == ix && (int)enemies[i].y == iy) {
                return true;
            }
        }
    }
    
    return false;
}

void TankBattlePage::moveTank(Tank* tank, Direction dir) {
    tank->dir = dir;
    
    float newX = tank->x;
    float newY = tank->y;
    
    switch (dir) {
        case DIR_UP:    newY--; break;
        case DIR_DOWN:  newY++; break;
        case DIR_LEFT:  newX--; break;
        case DIR_RIGHT: newX++; break;
    }
    
    if (canMove(newX, newY) && !checkTankCollision(newX, newY, tank)) {
        tank->x = newX;
        tank->y = newY;
        tank->moveCounter = 5;  // 移动冷却
    }
}

void TankBattlePage::shoot(Tank* tank, bool isPlayer) {
    if (tank->shootCooldown > 0) return;
    
    // 找一个空闲子弹槽
    for (int i = 0; i < TANK_MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = tank->x;
            bullets[i].y = tank->y;
            bullets[i].dir = tank->dir;
            bullets[i].active = true;
            bullets[i].isPlayer = isPlayer;
            
            tank->shootCooldown = isPlayer ? 15 : 30;  // 玩家射速更快
            break;
        }
    }
}

void TankBattlePage::createExplosion(int x, int y) {
    explosionX = x;
    explosionY = y;
    explosionTimer = 10;
}

void TankBattlePage::destroyTile(int x, int y) {
    map[y][x] = TILE_EMPTY;
    createExplosion(x, y);
}

// ========== 绘制函数 ==========

int TankBattlePage::getPixelX(int gridX) {
    return TANK_BOARD_X + gridX * TANK_CELL_SIZE;
}

int TankBattlePage::getPixelY(int gridY) {
    return TANK_BOARD_Y + gridY * TANK_CELL_SIZE;
}

void TankBattlePage::drawMap(U8G2* u8g2) {
    for (int y = 0; y < TANK_BOARD_HEIGHT; y++) {
        for (int x = 0; x < TANK_BOARD_WIDTH; x++) {
            int px = getPixelX(x);
            int py = getPixelY(y);
            
            switch (map[y][x]) {
                case TILE_BRICK:
                    // 砖墙 - 点阵图案
                    u8g2->drawPixel(px, py);
                    u8g2->drawPixel(px + 2, py);
                    u8g2->drawPixel(px + 1, py + 1);
                    u8g2->drawPixel(px + 3, py + 1);
                    u8g2->drawPixel(px, py + 2);
                    u8g2->drawPixel(px + 2, py + 2);
                    u8g2->drawPixel(px + 1, py + 3);
                    u8g2->drawPixel(px + 3, py + 3);
                    break;
                    
                case TILE_STEEL:
                    // 钢墙 - 实心方块
                    u8g2->drawBox(px, py, TANK_CELL_SIZE, TANK_CELL_SIZE);
                    break;
                    
                case TILE_BASE:
                    // 基地 - 旗帜图案
                    if (!baseDestroyed) {
                        u8g2->drawFrame(px, py, TANK_CELL_SIZE, TANK_CELL_SIZE);
                        u8g2->drawPixel(px + 1, py + 1);
                        u8g2->drawPixel(px + 2, py + 1);
                    } else {
                        // 被摧毁的基地
                        u8g2->drawLine(px, py, px + 3, py + 3);
                        u8g2->drawLine(px + 3, py, px, py + 3);
                    }
                    break;
            }
        }
    }
}

void TankBattlePage::drawTank(U8G2* u8g2, Tank* tank, bool isPlayer) {
    if (!tank->active) return;
    
    int px = getPixelX((int)tank->x);
    int py = getPixelY((int)tank->y);
    
    if (isPlayer) {
        // 玩家坦克 - 根据方向绘制坦克形状
        switch (tank->dir) {
            case DIR_UP:
                // 炮管向上
                u8g2->drawLine(px + 1, py - 1, px + 2, py - 1);  // 炮管突出
                u8g2->drawBox(px + 1, py, 2, 2);                  // 炮塔
                u8g2->drawLine(px, py + 1, px, py + 4);           // 左履带
                u8g2->drawLine(px + 3, py + 1, px + 3, py + 4);   // 右履带
                u8g2->drawBox(px + 1, py + 2, 2, 2);              // 车体
                break;
                
            case DIR_DOWN:
                // 炮管向下
                u8g2->drawBox(px + 1, py, 2, 2);                  // 车体
                u8g2->drawLine(px, py, px, py + 3);               // 左履带
                u8g2->drawLine(px + 3, py, px + 3, py + 3);       // 右履带
                u8g2->drawBox(px + 1, py + 2, 2, 2);              // 炮塔
                u8g2->drawLine(px + 1, py + 4, px + 2, py + 4);   // 炮管突出
                break;
                
            case DIR_LEFT:
                // 炮管向左
                u8g2->drawLine(px - 1, py + 1, px - 1, py + 2);   // 炮管突出
                u8g2->drawBox(px, py + 1, 2, 2);                  // 炮塔
                u8g2->drawLine(px + 1, py, px + 4, py);           // 上履带
                u8g2->drawLine(px + 1, py + 3, px + 4, py + 3);   // 下履带
                u8g2->drawBox(px + 2, py + 1, 2, 2);              // 车体
                break;
                
            case DIR_RIGHT:
                // 炮管向右
                u8g2->drawBox(px, py + 1, 2, 2);                  // 车体
                u8g2->drawLine(px, py, px + 3, py);               // 上履带
                u8g2->drawLine(px, py + 3, px + 3, py + 3);       // 下履带
                u8g2->drawBox(px + 2, py + 1, 2, 2);              // 炮塔
                u8g2->drawLine(px + 4, py + 1, px + 4, py + 2);   // 炮管突出
                break;
        }
    } else {
        // 敌人坦克 - 简化版，但也有方向指示
        switch (tank->dir) {
            case DIR_UP:
                u8g2->drawPixel(px + 1, py - 1);
                u8g2->drawPixel(px + 2, py - 1);
                u8g2->drawFrame(px, py, TANK_CELL_SIZE, TANK_CELL_SIZE);
                break;
            case DIR_DOWN:
                u8g2->drawFrame(px, py, TANK_CELL_SIZE, TANK_CELL_SIZE);
                u8g2->drawPixel(px + 1, py + 4);
                u8g2->drawPixel(px + 2, py + 4);
                break;
            case DIR_LEFT:
                u8g2->drawPixel(px - 1, py + 1);
                u8g2->drawPixel(px - 1, py + 2);
                u8g2->drawFrame(px, py, TANK_CELL_SIZE, TANK_CELL_SIZE);
                break;
            case DIR_RIGHT:
                u8g2->drawFrame(px, py, TANK_CELL_SIZE, TANK_CELL_SIZE);
                u8g2->drawPixel(px + 4, py + 1);
                u8g2->drawPixel(px + 4, py + 2);
                break;
        }
    }
}

void TankBattlePage::drawBullets(U8G2* u8g2) {
    for (int i = 0; i < TANK_MAX_BULLETS; i++) {
        if (bullets[i].active) {
            int px = getPixelX((int)bullets[i].x) + 1;
            int py = getPixelY((int)bullets[i].y) + 1;
            
            // 子弹 - 2x2像素
            u8g2->drawBox(px, py, 2, 2);
        }
    }
}

void TankBattlePage::drawExplosion(U8G2* u8g2) {
    if (explosionTimer > 0) {
        int px = getPixelX(explosionX) + 2;  // 中心点
        int py = getPixelY(explosionY) + 2;
        int size = (10 - explosionTimer) / 2;
        
        // 限制爆炸范围，防止超出边界
        if (size > 3) size = 3;
        
        // 爆炸效果 - 扩散的十字（限制范围）
        u8g2->drawLine(px - size, py, px + size, py);
        u8g2->drawLine(px, py - size, px, py + size);
    }
}

void TankBattlePage::drawHUD(U8G2* u8g2) {
    u8g2->setFont(u8g2_font_5x7_tf);
    
    // 顶部信息栏（y=8，在游戏区域上方）
    // 左侧：生命值
    u8g2->drawStr(2, 8, "HP:");
    for (int i = 0; i < player.hp; i++) {
        u8g2->drawBox(18 + i * 4, 2, 3, 3);
    }
    
    // 中间偏左：分数
    char scoreStr[16];
    snprintf(scoreStr, sizeof(scoreStr), "SC:%d", score);
    u8g2->drawStr(36, 8, scoreStr);
    
    // 中间偏右：关卡
    char levelStr[8];
    snprintf(levelStr, sizeof(levelStr), "LV:%d", level);
    u8g2->drawStr(75, 8, levelStr);
    
    // 右侧：剩余敌人
    u8g2->drawStr(99, 8, "EN:");
    char enemyStr[3];
    snprintf(enemyStr, sizeof(enemyStr), "%d", enemiesRemaining + enemiesAlive);
    u8g2->drawStr(115, 8, enemyStr);
}

void TankBattlePage::drawBorder(U8G2* u8g2) {
    // 绘制游戏区域边框
    int borderX = TANK_BOARD_X - 1;
    int borderY = TANK_BOARD_Y - 1;
    int borderW = TANK_BOARD_WIDTH * TANK_CELL_SIZE + 2;
    int borderH = TANK_BOARD_HEIGHT * TANK_CELL_SIZE + 2;
    
    u8g2->drawFrame(borderX, borderY, borderW, borderH);
}

void TankBattlePage::drawStartScreen(U8G2* u8g2) {
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    
    // 标题
    const char* title = "坦克大战";
    int titleWidth = u8g2->getUTF8Width(title);
    u8g2->drawUTF8((SCREEN_WIDTH - titleWidth) / 2, 16, title);
    
    // 绘制装饰坦克
    u8g2->drawBox(SCREEN_WIDTH / 2 - 8, 24, 4, 4);
    u8g2->drawBox(SCREEN_WIDTH / 2 + 4, 24, 4, 4);
    
    // 操作说明
    u8g2->setFont(u8g2_font_5x7_tf);
    const char* op1 = "2468:MOVE 5:SHOOT";
    int op1Width = strlen(op1) * 5;
    u8g2->drawStr((SCREEN_WIDTH - op1Width) / 2, 40, op1);
    
    // 开始提示
    if (frameCount % 60 < 40) {
        u8g2->setFont(u8g2_font_wqy12_t_gb2312);
        const char* startText = "按确认键开始";
        int startWidth = u8g2->getUTF8Width(startText);
        u8g2->drawUTF8((SCREEN_WIDTH - startWidth) / 2, 56, startText);
    }
}

void TankBattlePage::drawGameOverScreen(U8G2* u8g2) {
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    
    const char* gameOverText = "GAME OVER";
    int textWidth = u8g2->getUTF8Width(gameOverText);
    u8g2->drawUTF8((SCREEN_WIDTH - textWidth) / 2, 22, gameOverText);
    
    const char* scoreText = "分数: ";
    char scoreNum[8];
    snprintf(scoreNum, sizeof(scoreNum), "%d", score);
    String fullScore = String(scoreText) + String(scoreNum);
    textWidth = u8g2->getUTF8Width(fullScore.c_str());
    u8g2->drawUTF8((SCREEN_WIDTH - textWidth) / 2, 36, fullScore.c_str());
    
    const char* restartText = "按确认键重玩";
    textWidth = u8g2->getUTF8Width(restartText);
    u8g2->drawUTF8((SCREEN_WIDTH - textWidth) / 2, 50, restartText);
}

void TankBattlePage::drawPausedScreen(U8G2* u8g2) {
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    
    const char* pausedText = "暂停";
    int textWidth = u8g2->getUTF8Width(pausedText);
    u8g2->drawBox((SCREEN_WIDTH - textWidth) / 2 - 2, 20, textWidth + 4, 14);
    u8g2->setDrawColor(0);
    u8g2->drawUTF8((SCREEN_WIDTH - textWidth) / 2, 32, pausedText);
    u8g2->setDrawColor(1);
}

void TankBattlePage::drawLevelClearScreen(U8G2* u8g2) {
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    
    const char* clearText = "关卡完成!";
    int textWidth = u8g2->getUTF8Width(clearText);
    u8g2->drawUTF8((SCREEN_WIDTH - textWidth) / 2, 20, clearText);
    
    const char* enemiesText = "击杀: ";
    char enemiesNum[4];
    snprintf(enemiesNum, sizeof(enemiesNum), "%d", enemiesKilled);
    String fullEnemies = String(enemiesText) + String(enemiesNum);
    textWidth = u8g2->getUTF8Width(fullEnemies.c_str());
    u8g2->drawUTF8((SCREEN_WIDTH - textWidth) / 2, 34, fullEnemies.c_str());
    
    const char* nextText = "按确认进入下关";
    textWidth = u8g2->getUTF8Width(nextText);
    u8g2->drawUTF8((SCREEN_WIDTH - textWidth) / 2, 50, nextText);
}

// ========== 按钮事件处理 ==========

void TankBattlePage::onButtonBack(void* context) {
    if (gameState == PLAYING) {
        gameState = PAUSED;
    } else {
        uiEngine.navigateBack();
    }
}

void TankBattlePage::onButtonMenu(void* context) {
    if (gameState == PLAYING) {
        gameState = PAUSED;
    } else if (gameState == PAUSED) {
        gameState = PLAYING;
    }
}

void TankBattlePage::onButtonEnter(void* context) {
    switch (gameState) {
        case START:
            gameState = PLAYING;
            break;
            
        case PLAYING:
            shoot(&player, true);
            break;
            
        case PAUSED:
            gameState = PLAYING;
            break;
            
        case LEVEL_CLEAR:
            level++;
            initLevel();
            gameState = PLAYING;
            break;
            
        case GAME_OVER:
            initGame();
            gameState = PLAYING;
            break;
    }
}

void TankBattlePage::onButton1(void* context) {
    // 1键 - 射击（备用）
    if (gameState == PLAYING) {
        shoot(&player, true);
    }
}

void TankBattlePage::onButton2(void* context) {
    // 2键 - 上
    if (gameState == PLAYING) {
        moveTank(&player, DIR_UP);
    }
}

void TankBattlePage::onButton3(void* context) {
    // 3键 - 射击（备用）
    if (gameState == PLAYING) {
        shoot(&player, true);
    }
}

void TankBattlePage::onButton4(void* context) {
    // 4键 - 左
    if (gameState == PLAYING) {
        moveTank(&player, DIR_LEFT);
    }
}

void TankBattlePage::onButton5(void* context) {
    // 5键 - 射击
    if (gameState == PLAYING) {
        shoot(&player, true);
    }
}

void TankBattlePage::onButton6(void* context) {
    // 6键 - 右
    if (gameState == PLAYING) {
        moveTank(&player, DIR_RIGHT);
    }
}

void TankBattlePage::onButton7(void* context) {
    // 7键 - 射击（备用）
    if (gameState == PLAYING) {
        shoot(&player, true);
    }
}

void TankBattlePage::onButton8(void* context) {
    // 8键 - 下
    if (gameState == PLAYING) {
        moveTank(&player, DIR_DOWN);
    }
}

void TankBattlePage::onButton9(void* context) {
    // 9键 - 射击（备用）
    if (gameState == PLAYING) {
        shoot(&player, true);
    }
}
