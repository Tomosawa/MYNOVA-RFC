/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// ShooterPage.cpp
#include "ShooterPage.h"
#include "../GUI/UIEngine.h"
#include "../ButtonDetector.h"
#include "clib/u8g2.h"
#include <Arduino.h>

extern UIEngine uiEngine;

ShooterPage::ShooterPage() : UIPage(0, 0, SHOOTER_SCREEN_WIDTH, SHOOTER_SCREEN_HEIGHT) {
    initGame();
}

ShooterPage::~ShooterPage() {
    ButtonDetector::setFastResponseMode(false);
    ButtonDetector::setLongPressEnabled(false);
}

void ShooterPage::showPage() {
    ButtonDetector::setFastResponseMode(true);
    ButtonDetector::setLongPressEnabled(true);
}

void ShooterPage::initGame() {
    gameState = START;
    
    // 玩家初始化
    player.x = 20.0f;
    player.y = 28.0f;
    player.hp = 3;
    player.maxHp = 5;
    player.weapon = WEAPON_NORMAL;
    player.weaponLevel = 1;
    player.weaponTimer = 0;
    player.shielded = false;
    player.shieldTimer = 0;
    player.invincible = false;
    player.invincibleTimer = 0;
    
    // 清空子弹
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = false;
    }
    playerShootCooldown = 0;
    laserBeamTimer = 0;
    
    // 清空敌机
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
    }
    
    // 清空道具
    for (int i = 0; i < MAX_POWERUPS; i++) {
        powerUps[i].active = false;
    }
    
    // 清空特效
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        explosions[i].active = false;
    }
    
    // 游戏数据
    score = 0;
    level = 1;
    enemiesKilled = 0;
    bombs = 3;
    frameCount = 0;
    
    // 关卡数据
    levelProgress = 0;
    bossActive = false;
    spawnTimer = 0;
    bossLaserTimer = 0;
    bossLaserY = 0;
    bossRotatingAngle = 0;
    
    bgScroll = 0;
}

void ShooterPage::initLevel() {
    levelProgress = 0;
    bossActive = false;
    spawnTimer = 0;
    
    // 清空敌机和子弹
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
    }
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].isPlayerBullet) {
            bullets[i].active = false;
        }
    }
}

void ShooterPage::render(U8G2* u8g2) {
    u8g2->clearBuffer();
    
    switch (gameState) {
        case START:
            drawStartScreen(u8g2);
            break;
            
        case PLAYING:
            drawBackground(u8g2);
            drawParticles(u8g2);
            drawBullets(u8g2);
            drawEnemies(u8g2);
            drawPlayer(u8g2);
            drawPowerUps(u8g2);
            drawExplosions(u8g2);
            drawHUD(u8g2);
            break;
            
        case PAUSED:
            drawBackground(u8g2);
            drawBullets(u8g2);
            drawEnemies(u8g2);
            drawPlayer(u8g2);
            drawHUD(u8g2);
            drawPausedScreen(u8g2);
            break;
            
        case LEVEL_CLEAR:
            drawLevelClear(u8g2);
            break;
            
        case GAME_OVER:
            drawGameOverScreen(u8g2);
            break;
    }
    
    u8g2->sendBuffer();
}

void ShooterPage::update() {
    frameCount++;
    
    if (gameState == PLAYING) {
        updateGame();
    }
}

void ShooterPage::updateGame() {
    // 更新玩家
    updatePlayer();
    
    // 更新子弹
    updateBullets();
    
    // 更新敌机
    updateEnemies();
    
    // 更新道具
    updatePowerUps();
    
    // 更新特效
    updateParticles();
    updateExplosions();
    
    // 更新BOSS激光（跟随BOSS移动）
    if (bossLaserTimer > 0) {
        bossLaserTimer--;
        // 找到BOSS并更新激光Y坐标
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active && enemies[i].type == ENEMY_BOSS) {
                bossLaserY = enemies[i].y;
                break;
            }
        }
    }
    
    // 碰撞检测
    checkCollisions();
    
    // 生成敌机
    spawnTimer++;
    if (!bossActive) {
        // 普通敌机生成
        if (spawnTimer >= 40) {
            spawnEnemy();
            spawnTimer = 0;
        }
        
        // 检查是否到BOSS
        levelProgress++;
        if (levelProgress > 1500) {
            bossActive = true;
            spawnEnemy(); // 生成BOSS
        }
    }
    
    // 背景滚动
    bgScroll = (bgScroll + 1) % 128;
    
    // 检查玩家死亡
    if (player.hp <= 0) {
        gameState = GAME_OVER;
    }
}

// 继续实现各个update函数...
void ShooterPage::updatePlayer() {
    // 武器计时
    if (player.weaponTimer > 0) {
        player.weaponTimer--;
        if (player.weaponTimer == 0) {
            player.weapon = WEAPON_NORMAL;
            player.weaponLevel = 1;
        }
    }
    
    // 护盾计时
    if (player.shieldTimer > 0) {
        player.shieldTimer--;
        if (player.shieldTimer == 0) {
            player.shielded = false;
        }
    }
    
    // 无敌时间
    if (player.invincibleTimer > 0) {
        player.invincibleTimer--;
        if (player.invincibleTimer == 0) {
            player.invincible = false;
        }
    }
    
    // 自动射击冷却
    if (playerShootCooldown > 0) {
        playerShootCooldown--;
    }
    
    // 激光束持续时间
    if (laserBeamTimer > 0) {
        laserBeamTimer--;
    }
    
    // 限制移动范围（全屏可移动，留出飞机大小）
    if (player.x < 0) player.x = 0;
    if (player.x > 118) player.x = 118;  // 128 - 10（飞机宽度）
    if (player.y < 10) player.y = 10;    // 顶部留HUD空间
    if (player.y > 59) player.y = 59;    // 底部留边界
}

void ShooterPage::updateBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        
        bullets[i].x += bullets[i].vx;
        bullets[i].y += bullets[i].vy;
        
        // 导弹追踪逻辑
        if (bullets[i].isPlayerBullet && bullets[i].type == WEAPON_MISSILE) {
            Enemy* target = findNearestEnemy();
            if (target != nullptr) {
                float dx = target->x - bullets[i].x;
                float dy = target->y - bullets[i].y;
                float dist = sqrt(dx*dx + dy*dy);
                
                if (dist > 0) {
                    float speed = 2.5f;
                    bullets[i].vx = (dx / dist) * speed;
                    bullets[i].vy = (dy / dist) * speed;
                }
            }
        }
        
        // 移出屏幕则移除
        if (bullets[i].x < 0 || bullets[i].x > 128 ||
            bullets[i].y < 0 || bullets[i].y > 64) {
            bullets[i].active = false;
        }
    }
}

void ShooterPage::updateEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;
        
        Enemy* e = &enemies[i];
        
        // 移动模式
        e->moveTimer += 0.1f;
        
        switch (e->movePattern) {
            case 0: // 直线飞行
                e->x += e->vx;
                e->y += e->vy;
                break;
                
            case 1: // 波浪飞行
                e->x += e->vx;
                e->y += sin(e->moveTimer) * 1.5f;
                break;
                
            case 2: // 追踪玩家
                if (frameCount % 30 == 0) {
                    float dx = player.x - e->x;
                    float dy = player.y - e->y;
                    float dist = sqrt(dx*dx + dy*dy);
                    if (dist > 0) {
                        e->vx = (dx / dist) * 0.5f;
                        e->vy = (dy / dist) * 0.5f;
                    }
                }
                e->x += e->vx;
                e->y += e->vy;
                break;
        }
        
        // 射击计时
        e->shootTimer--;
        if (e->shootTimer <= 0) {
            if (e->type == ENEMY_BOSS) {
                // BOSS满天星弹幕
                enemyShootBoss(e);
                e->shootTimer = 30 + random(20);  // BOSS射击更频繁
            } else {
                enemyShoot(e);
                e->shootTimer = 60 + random(60);
            }
        }
        
        // 移出屏幕左侧则移除
        if (e->x < -10 || e->y < -10 || e->y > 74) {
            e->active = false;
        }
    }
}

void ShooterPage::updatePowerUps() {
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (!powerUps[i].active) continue;
        
        // 更新位置
        powerUps[i].y += powerUps[i].vy;
        powerUps[i].x += powerUps[i].vx;
        
        // 碰到上下边界反弹
        if (powerUps[i].y <= 10) {
            powerUps[i].y = 10;
            powerUps[i].vy = -powerUps[i].vy;
        }
        if (powerUps[i].y >= 62) {
            powerUps[i].y = 62;
            powerUps[i].vy = -powerUps[i].vy;
        }
        
        // 碰到左右边界反弹
        if (powerUps[i].x <= 2) {
            powerUps[i].x = 2;
            powerUps[i].vx = -powerUps[i].vx;
        }
        if (powerUps[i].x >= 126) {
            powerUps[i].x = 126;
            powerUps[i].vx = -powerUps[i].vx;
        }
        
        // 生命周期递减
        powerUps[i].lifetime--;
        if (powerUps[i].lifetime <= 0) {
            powerUps[i].active = false;
            spawnParticles(powerUps[i].x, powerUps[i].y, 3, 1);
        }
    }
}

void ShooterPage::updateParticles() {
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

void ShooterPage::updateExplosions() {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!explosions[i].active) continue;
        
        explosions[i].frame++;
        if (explosions[i].frame >= explosions[i].maxFrame) {
            explosions[i].active = false;
        }
    }
}

// 射击相关函数
void ShooterPage::playerShoot() {
    if (playerShootCooldown > 0) return;
    
    // 根据武器类型射击
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].active = true;
            bullets[i].isPlayerBullet = true;
            bullets[i].x = player.x + 6;
            bullets[i].y = player.y;
            bullets[i].damage = player.weaponLevel;
            bullets[i].type = player.weapon;
            
            switch (player.weapon) {
                case WEAPON_NORMAL:
                    bullets[i].vx = 3.0f;
                    bullets[i].vy = 0;
                    playerShootCooldown = 8;
                    break;
                    
                case WEAPON_MISSILE:
                    bullets[i].vx = 2.5f;
                    bullets[i].vy = 0;
                    playerShootCooldown = 12;
                    break;
                    
                case WEAPON_LASER:
                    // 激光是持续光束，不是普通子弹
                    laserBeamTimer = 60;  // 持续1秒（60帧）
                    playerShootCooldown = 70;  // 射击间隔稍长
                    bullets[i].active = false;  // 不使用子弹
                    break;
                    
                case WEAPON_SPREAD:
                    // 散弹：5发密集散射
                    if (i + 4 < MAX_BULLETS) {
                        // 中间
                        bullets[i].vx = 3.0f;
                        bullets[i].vy = 0;
                        // 上方15度
                        bullets[i+1].active = true;
                        bullets[i+1].isPlayerBullet = true;
                        bullets[i+1].x = player.x + 6;
                        bullets[i+1].y = player.y;
                        bullets[i+1].vx = 2.9f;
                        bullets[i+1].vy = -0.8f;
                        bullets[i+1].damage = player.weaponLevel;
                        bullets[i+1].type = player.weapon;
                        // 下方15度
                        bullets[i+2].active = true;
                        bullets[i+2].isPlayerBullet = true;
                        bullets[i+2].x = player.x + 6;
                        bullets[i+2].y = player.y;
                        bullets[i+2].vx = 2.9f;
                        bullets[i+2].vy = 0.8f;
                        bullets[i+2].damage = player.weaponLevel;
                        bullets[i+2].type = player.weapon;
                        // 上方30度
                        bullets[i+3].active = true;
                        bullets[i+3].isPlayerBullet = true;
                        bullets[i+3].x = player.x + 6;
                        bullets[i+3].y = player.y;
                        bullets[i+3].vx = 2.6f;
                        bullets[i+3].vy = -1.5f;
                        bullets[i+3].damage = player.weaponLevel;
                        bullets[i+3].type = player.weapon;
                        // 下方30度
                        bullets[i+4].active = true;
                        bullets[i+4].isPlayerBullet = true;
                        bullets[i+4].x = player.x + 6;
                        bullets[i+4].y = player.y;
                        bullets[i+4].vx = 2.6f;
                        bullets[i+4].vy = 1.5f;
                        bullets[i+4].damage = player.weaponLevel;
                        bullets[i+4].type = player.weapon;
                    }
                    playerShootCooldown = 15;
                    break;
                    
                case WEAPON_BEAM:
                    bullets[i].vx = 3.5f;
                    bullets[i].vy = 0;
                    bullets[i].damage = player.weaponLevel + 1;  // 光束伤害更高
                    bullets[i].explosionCenterX = bullets[i].x;  // 记录发射初始X坐标，用于计算扩散
                    playerShootCooldown = 6;  // 射速快
                    break;
                    
                case WEAPON_HOMING:
                    bullets[i].vx = 2.0f;
                    bullets[i].vy = 0;
                    bullets[i].chainExplosions = 5;  // 连环爆炸5次
                    bullets[i].isExploding = false;  // 初始未爆炸
                    bullets[i].explosionTimer = 0;
                    playerShootCooldown = 10;
                    break;
                    
                default:
                    bullets[i].vx = 3.0f;
                    bullets[i].vy = 0;
                    playerShootCooldown = 8;
                    break;
            }
            
            break;
        }
    }
}

void ShooterPage::enemyShoot(ShooterPage::Enemy* enemy) {
    // 敌机射击
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].active = true;
            bullets[i].isPlayerBullet = false;
            bullets[i].x = enemy->x;
            bullets[i].y = enemy->y;
            bullets[i].damage = 1;
            
            // 朝玩家方向射击
            float dx = player.x - enemy->x;
            float dy = player.y - enemy->y;
            float dist = sqrt(dx*dx + dy*dy);
            if (dist > 0) {
                bullets[i].vx = (dx / dist) * 2.0f;
                bullets[i].vy = (dy / dist) * 2.0f;
            } else {
                bullets[i].vx = -2.0f;
                bullets[i].vy = 0;
            }
            
            break;
        }
    }
}

void ShooterPage::enemyShootBoss(ShooterPage::Enemy* enemy) {
    // BOSS随机选择攻击模式
    int attackMode = random(100);
    
    if (attackMode < 40) {
        // 模式1：扇形弹幕（40%概率）
        int bulletsFired = 0;
        int maxBullets = 7;
        
        for (int i = 0; i < MAX_BULLETS && bulletsFired < maxBullets; i++) {
            if (!bullets[i].active) {
                bullets[i].active = true;
                bullets[i].isPlayerBullet = false;
                bullets[i].x = enemy->x - 5;
                bullets[i].y = enemy->y;
                bullets[i].damage = 1;
                
                float angle = -60.0f + (bulletsFired * 20.0f);
                float rad = angle * 3.14159f / 180.0f;
                bullets[i].vx = cos(rad) * -2.5f;
                bullets[i].vy = sin(rad) * 2.5f;
                
                bulletsFired++;
            }
        }
    } else if (attackMode < 70) {
        // 模式2：旋转弹幕（30%概率）
        int bulletsFired = 0;
        int maxBullets = 8;
        
        for (int i = 0; i < MAX_BULLETS && bulletsFired < maxBullets; i++) {
            if (!bullets[i].active) {
                bullets[i].active = true;
                bullets[i].isPlayerBullet = false;
                bullets[i].x = enemy->x - 5;
                bullets[i].y = enemy->y;
                bullets[i].damage = 1;
                
                // 旋转弹幕：360度均匀分布 + 旋转偏移
                float angle = (bulletsFired * 45.0f) + bossRotatingAngle;
                float rad = angle * 3.14159f / 180.0f;
                bullets[i].vx = cos(rad) * -2.0f;
                bullets[i].vy = sin(rad) * 2.0f;
                
                bulletsFired++;
            }
        }
        bossRotatingAngle += 15;  // 每次旋转15度
        if (bossRotatingAngle >= 360) bossRotatingAngle = 0;
    } else {
        // 模式3：激光蓄力（30%概率）
        if (bossLaserTimer <= 0) {
            bossLaserTimer = 90;  // 激光持续90帧（1.5秒）
            bossLaserY = enemy->y;  // 记录BOSS当前Y坐标
        }
    }
}

void ShooterPage::useBomb() {
    if (bombs <= 0) return;
    
    bombs--;
    
    // 清除所有敌方子弹
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active && !bullets[i].isPlayerBullet) {
            bullets[i].active = false;
            spawnParticles(bullets[i].x, bullets[i].y, 3, 1);
        }
    }
    
    // 伤害所有敌机
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            damageEnemy(&enemies[i], 5);
        }
    }
    
    // 华丽的全屏闪光效果
    for (int i = 0; i < 20; i++) {
        spawnParticles(random(128), random(64), 1, 2);
    }
}

// 碰撞检测
void ShooterPage::checkCollisions() {
    // 玩家激光束 vs 敌机
    if (laserBeamTimer > 0) {
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!enemies[j].active) continue;
            
            float dy = player.y - enemies[j].y;
            
            // 检测敌机是否在激光线上（Y坐标接近）
            if (abs(dy) < 3 && enemies[j].x > player.x) {
                damageEnemy(&enemies[j], 1);  // 每帧造成1点伤害
                if (frameCount % 5 == 0) {  // 每5帧一次粒子
                    spawnParticles(enemies[j].x, enemies[j].y, 2, 1);
                }
            }
        }
    }
    
    // BOSS激光 vs 玩家
    if (bossLaserTimer > 0 && !player.invincible) {
        float dy = player.y - bossLaserY;
        
        // 检测玩家是否在BOSS激光线上（Y坐标接近，X在左侧）
        if (abs(dy) < 4 && player.x < 100) {
            // 每10帧造成1点伤害
            if (frameCount % 10 == 0) {
                player.hp--;
                if (player.hp <= 0) {
                    player.hp = 0;
                    gameState = GAME_OVER;
                } else {
                    player.invincible = true;
                    player.invincibleTimer = 60;
                }
                spawnParticles(player.x, player.y, 5, 2);
            }
        }
    }
    
    // 玩家子弹 vs 敌机
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active || !bullets[i].isPlayerBullet) continue;
        
        // HOMING连环爆炸处理
        if (bullets[i].type == WEAPON_HOMING) {
            if (!bullets[i].isExploding) {
                // 飞行阶段：检测是否击中敌人
                for (int j = 0; j < MAX_ENEMIES; j++) {
                    if (!enemies[j].active) continue;
                    
                    float dx = bullets[i].x - enemies[j].x;
                    float dy = bullets[i].y - enemies[j].y;
                    
                    // 碰撞检测
                    if (abs(dx) < 8 && abs(dy) < 6) {
                        // 击中！开始连环爆炸
                        bullets[i].isExploding = true;
                        bullets[i].explosionCenterX = bullets[i].x;
                        bullets[i].explosionCenterY = bullets[i].y;
                        bullets[i].explosionTimer = 0;
                        bullets[i].vx = 0;  // 停止移动
                        bullets[i].vy = 0;
                        
                        // 第一次爆炸
                        damageEnemy(&enemies[j], bullets[i].damage);
                        spawnExplosion(bullets[i].x, bullets[i].y, 8);
                        bullets[i].chainExplosions--;
                        break;
                    }
                }
            } else {
                // 连环爆炸阶段
                bullets[i].explosionTimer++;
                if (bullets[i].explosionTimer >= 8) {  // 每8帧爆炸一次
                    bullets[i].explosionTimer = 0;
                    
                    // 在中心点附近随机位置爆炸
                    float offsetX = (random(21) - 10);  // -10到+10
                    float offsetY = (random(21) - 10);
                    bullets[i].x = bullets[i].explosionCenterX + offsetX;
                    bullets[i].y = bullets[i].explosionCenterY + offsetY;
                    
                    // 爆炸伤害附近敌人
                    for (int j = 0; j < MAX_ENEMIES; j++) {
                        if (!enemies[j].active) continue;
                        
                        float dx = bullets[i].x - enemies[j].x;
                        float dy = bullets[i].y - enemies[j].y;
                        
                        // 爆炸范围12px
                        if (abs(dx) < 12 && abs(dy) < 10) {
                            damageEnemy(&enemies[j], bullets[i].damage);
                            spawnParticles(enemies[j].x, enemies[j].y, 5, 2);
                        }
                    }
                    
                    // 产生爆炸特效
                    spawnExplosion(bullets[i].x, bullets[i].y, 8);
                    bullets[i].chainExplosions--;
                    
                    // 爆炸完毕
                    if (bullets[i].chainExplosions <= 0) {
                        bullets[i].active = false;
                    }
                }
            }
        } else {
            // 普通子弹
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (!enemies[j].active) continue;
                
                float dx = bullets[i].x - enemies[j].x;
                float dy = bullets[i].y - enemies[j].y;
                
                int hitW = 8;
                int hitH = 6;
                
                // BEAM武器判定范围随距离变大
                if (bullets[i].type == WEAPON_BEAM) {
                    float dist = bullets[i].x - bullets[i].explosionCenterX;
                    if (dist < 0) dist = 0;
                    hitW = 8 + (int)(dist / 12.0f);
                    hitH = 8 + (int)(dist / 8.0f);
                    if (hitW > 20) hitW = 20;
                    if (hitH > 30) hitH = 30;
                }
                
                // 碰撞检测
                if (abs(dx) < hitW && abs(dy) < hitH) {
                    damageEnemy(&enemies[j], bullets[i].damage);
                    
                    // 光束和散弹不穿透
                    bullets[i].active = false;
                    
                    spawnParticles(bullets[i].x, bullets[i].y, 3, 1);
                    break;
                }
            }
        }
    }
    
    // 敌方子弹 vs 玩家
    if (!player.invincible) {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].active || bullets[i].isPlayerBullet) continue;
            
            float dx = bullets[i].x - player.x;
            float dy = bullets[i].y - player.y;
            
            if (abs(dx) < 5 && abs(dy) < 4) {
                bullets[i].active = false;
                
                if (player.shielded) {
                    player.shielded = false;
                    player.shieldTimer = 0;
                } else {
                    player.hp--;
                    player.invincible = true;
                    player.invincibleTimer = 90;
                    spawnExplosion(player.x, player.y, 2);
                    // 死亡后重置炸弹
                    if (player.hp > 0) {
                        bombs = 3;
                    }
                }
                break;
            }
        }
    }
    
    // 玩家 vs 敌机（撞机）
    if (!player.invincible) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!enemies[i].active) continue;
            
            float dx = player.x - enemies[i].x;
            float dy = player.y - enemies[i].y;
            
            if (abs(dx) < 7 && abs(dy) < 5) {
                if (player.shielded) {
                    player.shielded = false;
                    damageEnemy(&enemies[i], 999);
                } else {
                    player.hp--;
                    player.invincible = true;
                    player.invincibleTimer = 90;
                    spawnExplosion(player.x, player.y, 2);
                    damageEnemy(&enemies[i], 2);
                    // 死亡后重置炸弹
                    if (player.hp > 0) {
                        bombs = 3;
                    }
                }
                break;
            }
        }
    }
    
    // 玩家 vs 道具
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (!powerUps[i].active) continue;
        
        float dx = player.x - powerUps[i].x;
        float dy = player.y - powerUps[i].y;
        
        // 道具碰撞范围调整为8x8
        if (abs(dx) < 10 && abs(dy) < 8) {
            powerUps[i].active = false;
            
            // 应用道具效果
            switch (powerUps[i].type) {
                case POWER_MISSILE:
                    player.weapon = WEAPON_MISSILE;
                    player.weaponTimer = 600;
                    player.weaponLevel = 2;
                    break;
                case POWER_LASER:
                    player.weapon = WEAPON_LASER;
                    player.weaponTimer = 600;
                    player.weaponLevel = 3;
                    break;
                case POWER_SPREAD:
                    player.weapon = WEAPON_SPREAD;
                    player.weaponTimer = 600;
                    player.weaponLevel = 2;
                    break;
                case POWER_BEAM:
                    player.weapon = WEAPON_BEAM;
                    player.weaponTimer = 600;
                    player.weaponLevel = 2;
                    break;
                case POWER_HOMING:
                    player.weapon = WEAPON_HOMING;
                    player.weaponTimer = 600;
                    player.weaponLevel = 2;
                    break;
                case POWER_LIFE:
                    if (player.hp < player.maxHp) {
                        player.hp++;
                    }
                    break;
                case POWER_BOMB:
                    bombs = min(9, bombs + 1);
                    break;
                case POWER_SHIELD:
                    player.shielded = true;
                    player.shieldTimer = 300;
                    break;
            }
            
            // 吃道具直接加分，无特效
            score += 50;
            break;
        }
    }
}

void ShooterPage::damageEnemy(ShooterPage::Enemy* enemy, int damage) {
    enemy->hp -= damage;
    
    if (enemy->hp <= 0) {
        enemy->active = false;
        enemiesKilled++;
        
        // 爆炸效果
        int explosionSize = (enemy->type == ENEMY_BOSS) ? 5 : (enemy->type == ENEMY_HEAVY ? 3 : 2);
        spawnExplosion(enemy->x, enemy->y, explosionSize);
        spawnParticles(enemy->x, enemy->y, 10, 2);
        
        // 分数
        score += (enemy->type == ENEMY_BOSS) ? 1000 : ((enemy->type == ENEMY_HEAVY) ? 100 : 50);
        
        // 掉落道具
        int dropChance = random(100);
        if (enemy->type == ENEMY_BOSS || dropChance < 20) {
            PowerUpType dropType = (PowerUpType)random(1, 9);
            spawnPowerUp(enemy->x, enemy->y, dropType);
        }
        
        // BOSS死亡
        if (enemy->type == ENEMY_BOSS) {
            bossActive = false;
            gameState = LEVEL_CLEAR;
        }
    }
}

// 生成函数
void ShooterPage::spawnEnemy() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].active = true;
            enemies[i].x = 128;
            enemies[i].y = 10 + random(44);
            enemies[i].shootTimer = 60;
            enemies[i].moveTimer = 0;
            
            if (bossActive) {
                // 生成BOSS
                enemies[i].type = ENEMY_BOSS;
                enemies[i].x = 120;
                enemies[i].y = 28;
                enemies[i].hp = 50 * level;
                enemies[i].maxHp = enemies[i].hp;
                enemies[i].vx = -0.3f;
                enemies[i].vy = 0;
                enemies[i].movePattern = 2;
            } else {
                // 普通敌机
                int rnd = random(100);
                if (rnd < 50) {
                    enemies[i].type = ENEMY_SMALL;
                    enemies[i].hp = 1;
                    enemies[i].vx = -1.5f;
                    enemies[i].vy = 0;
                    enemies[i].movePattern = 0;
                } else if (rnd < 80) {
                    enemies[i].type = ENEMY_MEDIUM;
                    enemies[i].hp = 2;
                    enemies[i].vx = -1.0f;
                    enemies[i].vy = 0;
                    enemies[i].movePattern = 1;
                } else {
                    enemies[i].type = ENEMY_HEAVY;
                    enemies[i].hp = 3; 
                    enemies[i].vx = -0.8f;
                    enemies[i].vy = 0;
                    enemies[i].movePattern = 2;
                }
                enemies[i].maxHp = enemies[i].hp;
            }
            break;
        }
    }
}

void ShooterPage::spawnPowerUp(float x, float y, PowerUpType type) {
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (!powerUps[i].active) {
            powerUps[i].active = true;
            powerUps[i].x = x;
            powerUps[i].y = y;
            // 随机初始速度
            powerUps[i].vx = random(-15, 15) / 10.0f;
            powerUps[i].vy = random(-15, 15) / 10.0f;
            powerUps[i].type = type;
            powerUps[i].lifetime = 600;  // 10秒 (60fps * 10)
            break;
        }
    }
}

void ShooterPage::spawnExplosion(float x, float y, int size) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!explosions[i].active) {
            explosions[i].active = true;
            explosions[i].x = x;
            explosions[i].y = y;
            explosions[i].frame = 0;
            explosions[i].maxFrame = 8;
            break;
        }
    }
}

void ShooterPage::spawnParticles(float x, float y, int count, int size) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < MAX_PARTICLES; j++) {
            if (!particles[j].active) {
                particles[j].active = true;
                particles[j].x = x;
                particles[j].y = y;
                particles[j].vx = random(-20, 20) / 10.0f;
                particles[j].vy = random(-20, 20) / 10.0f;
                particles[j].life = 20;
                particles[j].size = size;
                break;
            }
        }
    }
}

// 辅助函数
ShooterPage::Enemy* ShooterPage::findNearestEnemy() {
    ShooterPage::Enemy* nearest = nullptr;
    float minDist = 9999;
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;
        
        float dx = enemies[i].x - player.x;
        float dy = enemies[i].y - player.y;
        float dist = sqrt(dx*dx + dy*dy);
        
        if (dist < minDist) {
            minDist = dist;
            nearest = &enemies[i];
        }
    }
    
    return nearest;
}

// ========== 绘制函数 ==========

void ShooterPage::drawBackground(U8G2* u8g2) {
    // 滚动背景（星空效果）- 向左移动
    for (int i = 0; i < 20; i++) {
        int x = (128 - bgScroll + i * 15) % 128;
        int y = (i * 17) % 64;
        u8g2->drawPixel(x, y);
    }
    
    // 远景星星 - 向左移动（速度较慢）
    for (int i = 0; i < 10; i++) {
        int x = (128 - bgScroll / 2 + i * 25) % 128;
        int y = (i * 23) % 64;
        u8g2->drawPixel(x, y);
    }
}

void ShooterPage::drawPlayer(U8G2* u8g2) {
    if (player.invincible && (frameCount % 4 < 2)) {
        return; // 无敌闪烁
    }
    
    int px = (int)player.x;
    int py = (int)player.y;
    
    // 只在屏幕内绘制（飞机宽度约12px，左侧-3到右侧+9）
    if (px < -5 || px > 125 || py < 8 || py > 60) {
        return;
    }
    
    // 绘制玩家飞机（尖锐的战斗机）
    // 机头
    if (px + 8 >= 0 && px + 6 < 128) {
        u8g2->drawLine(px + 6, py, px + 8, py);
    }
    // 机身
    if (px >= 0 && px + 6 <= 128) {
        u8g2->drawBox(px, py - 2, 6, 4);
    }
    // 驾驶舱
    if (px + 2 >= 0 && px + 2 < 128) {
        u8g2->setDrawColor(0);
        u8g2->drawPixel(px + 2, py);
        u8g2->setDrawColor(1);
    }
    // 机翼（关键：确保两端都在屏幕内）
    if (px - 2 >= 0 && px + 2 < 128) {
        u8g2->drawLine(px - 2, py - 3, px + 2, py - 3);
        u8g2->drawLine(px - 2, py + 3, px + 2, py + 3);
    }
    if (px - 3 >= 0 && px - 3 < 128) {
        u8g2->drawPixel(px - 3, py - 4);
        u8g2->drawPixel(px - 3, py + 4);
    }
    // 尾翼
    if (px - 1 >= 0 && px - 1 < 128) {
        u8g2->drawPixel(px - 1, py - 2);
        u8g2->drawPixel(px - 1, py + 2);
    }
    
    // 护盾效果
    if (player.shielded && frameCount % 8 < 4 && px + 3 >= 0 && px + 3 < 128) {
        u8g2->drawCircle(px + 3, py, 6);
    }
    
    // 武器指示（机头发光）
    if (player.weapon != WEAPON_NORMAL && frameCount % 4 < 2) {
        if (px + 8 >= 0 && px + 8 < 128) u8g2->drawPixel(px + 8, py);
        if (px + 9 >= 0 && px + 9 < 128) u8g2->drawPixel(px + 9, py);
    }
}

void ShooterPage::drawBullets(U8G2* u8g2) {
    // 玩家激光束（持续光束效果）
    if (laserBeamTimer > 0) {
        int py = (int)player.y;
        int px = (int)player.x + 6;
        // 画一条从飞机到屏幕右侧的激光束
        u8g2->drawLine(px, py, 128, py);
        // 闪烁效果
        if (frameCount % 4 < 2) {
            u8g2->drawLine(px, py - 1, 128, py - 1);
            u8g2->drawLine(px, py + 1, 128, py + 1);
        }
    }
    
    // BOSS激光（从BOSS发出的红色粗激光）
    if (bossLaserTimer > 0) {
        // 找到BOSS位置
        int bossX = -1;
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active && enemies[i].type == ENEMY_BOSS) {
                bossX = (int)enemies[i].x;
                break;
            }
        }
        
        if (bossX >= 0) {
            int ly = (int)bossLaserY;
            int laserStartX = bossX - 8;  // 从BOSS机头发出
            
            // 蓄力阶段（前30帧）
            if (bossLaserTimer > 60) {
                // 闪烁警告线（从BOSS到屏幕左侧）
                if (frameCount % 8 < 4) {
                    u8g2->drawLine(0, ly, laserStartX, ly);
                }
            } else {
                // 发射阶段（粗激光，从BOSS到屏幕左侧）
                u8g2->drawLine(0, ly - 1, laserStartX, ly - 1);
                u8g2->drawLine(0, ly, laserStartX, ly);
                u8g2->drawLine(0, ly + 1, laserStartX, ly + 1);
                // 闪烁加强效果
                if (frameCount % 4 < 2) {
                    u8g2->drawLine(0, ly - 2, laserStartX, ly - 2);
                    u8g2->drawLine(0, ly + 2, laserStartX, ly + 2);
                }
            }
        }
    }
    
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        
        int bx = (int)bullets[i].x;
        int by = (int)bullets[i].y;
        
        if (bullets[i].isPlayerBullet) {
            // 玩家子弹样式
            switch (bullets[i].type) {
                case WEAPON_NORMAL:
                    u8g2->drawLine(bx, by, bx + 2, by);
                    break;
                    
                case WEAPON_MISSILE:
                    // 导弹（带尾焰）
                    u8g2->drawBox(bx, by - 1, 3, 2);
                    if (frameCount % 2 == 0) {
                        u8g2->drawPixel(bx - 1, by);
                    }
                    break;
                    
                case WEAPON_SPREAD:
                    // 散弹
                    u8g2->drawBox(bx, by, 2, 2);
                    break;
                    
                case WEAPON_HOMING: {
                    // 如果正在连环爆炸，不显示导弹本体，只显示爆炸特效（由drawExplosions处理）
                    if (bullets[i].isExploding) {
                        break;
                    }

                    // 制导导弹（旋转效果）
                    int rotation = (frameCount / 3) % 4;  // 0, 1, 2, 3循环
                    // 导弹本体
                    u8g2->drawBox(bx, by - 1, 3, 2);
                    u8g2->drawPixel(bx + 3, by);
                    // 旋转尾焰
                    switch (rotation) {
                        case 0:
                            u8g2->drawPixel(bx - 1, by);
                            u8g2->drawPixel(bx - 2, by);
                            break;
                        case 1:
                            u8g2->drawPixel(bx - 1, by - 1);
                            u8g2->drawPixel(bx - 2, by - 1);
                            break;
                        case 2:
                            u8g2->drawPixel(bx - 1, by);
                            u8g2->drawPixel(bx - 2, by);
                            break;
                        case 3:
                            u8g2->drawPixel(bx - 1, by + 1);
                            u8g2->drawPixel(bx - 2, by + 1);
                            break;
                    }
                    break;
                }
                    
                case WEAPON_BEAM: {
                    // 光束（扩散电波圈）
                    // 随着距离变远，圈越来越大
                    float dist = bullets[i].x - bullets[i].explosionCenterX;
                    if (dist < 0) dist = 0;
                    
                    // 计算扩散大小
                    // 初始大小 rx=2, ry=3
                    // 扩散系数：每10px增加1px半径
                    int rx = 2 + (int)(dist / 12.0f);
                    int ry = 3 + (int)(dist / 8.0f);
                    
                    // 限制最大大小，防止过于夸张
                    if (rx > 12) rx = 12;
                    if (ry > 18) ry = 18;
                    
                    // 绘制空心椭圆
                    u8g2->drawEllipse(bx, by, rx, ry, U8G2_DRAW_ALL);
                    
                    // 内部再画一个小一点的圈，增加层次感
                    if (rx > 3 && ry > 4) {
                        if (frameCount % 4 < 2) { // 闪烁效果
                            u8g2->drawEllipse(bx, by, rx - 2, ry - 2, U8G2_DRAW_ALL);
                        }
                    } else {
                        // 距离很近时，画个中心点
                        u8g2->drawPixel(bx, by);
                    }
                    break;
                }
                    
                default:
                    u8g2->drawPixel(bx, by);
                    break;
            }
        } else {
            // 敌方子弹（圆形）
            u8g2->drawCircle(bx, by, 1);
        }
    }
}

void ShooterPage::drawEnemies(U8G2* u8g2) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;
        
        int ex = (int)enemies[i].x;
        int ey = (int)enemies[i].y;
        
        // 只绘制屏幕内的敌机（根据敌机类型判断边界）
        // BOSS最宽（-10到+10），其他敌机较小（-7到+2）
        if (enemies[i].type == ENEMY_BOSS) {
            if (ex < -12 || ex > 140 || ey < -6 || ey > 70) continue;
        } else {
            if (ex < -8 || ex > 130 || ey < -5 || ey > 70) continue;
        }
        
        switch (enemies[i].type) {
            case ENEMY_SMALL:
                // 小型敌机（简单三角形）
                // 只在线条两端都在屏幕内时绘制
                if (ex - 2 >= 0 && ex + 2 < 128) {
                    u8g2->drawLine(ex - 2, ey, ex + 2, ey);
                    u8g2->drawPixel(ex - 1, ey - 1);
                    u8g2->drawPixel(ex - 1, ey + 1);
                    u8g2->drawPixel(ex, ey);
                }
                break;
                
            case ENEMY_MEDIUM:
                // 中型敌机（带翅膀）
                if (ex - 2 >= 0 && ex + 2 < 128) {
                    u8g2->drawBox(ex - 2, ey - 1, 4, 2);
                }
                if (ex - 3 >= 0) {
                    u8g2->drawPixel(ex - 3, ey - 2);
                    u8g2->drawPixel(ex - 3, ey + 2);
                }
                if (ex - 2 >= 0) {
                    u8g2->drawPixel(ex - 2, ey);
                }
                if (ex >= 0 && ex < 128) {
                    u8g2->setDrawColor(0);
                    u8g2->drawPixel(ex, ey);
                    u8g2->setDrawColor(1);
                }
                break;
                
            case ENEMY_HEAVY:
                // 重型敌机（飞机造型）
                // 机身
                if (ex - 3 >= 0 && ex + 2 < 128) {
                    u8g2->drawBox(ex - 3, ey - 1, 5, 2);
                }
                // 机头（关键：确保两端都在屏幕内）
                if (ex - 5 >= 0 && ex - 4 < 128) {
                    u8g2->drawLine(ex - 4, ey, ex - 5, ey);
                }
                // 上机翼（关键：确保两端都在屏幕内）
                if (ex - 2 >= 0 && ex + 1 < 128) {
                    u8g2->drawLine(ex - 2, ey - 2, ex + 1, ey - 2);
                }
                if (ex - 1 >= 0) {
                    u8g2->drawPixel(ex - 1, ey - 3);
                }
                // 下机翼（关键：确保两端都在屏幕内）
                if (ex - 2 >= 0 && ex + 1 < 128) {
                    u8g2->drawLine(ex - 2, ey + 2, ex + 1, ey + 2);
                }
                if (ex - 1 >= 0) {
                    u8g2->drawPixel(ex - 1, ey + 3);
                }
                // 尾翼
                if (ex + 2 < 128) {
                    u8g2->drawPixel(ex + 2, ey - 1);
                    u8g2->drawPixel(ex + 2, ey + 1);
                }
                // 驾驶舱
                if (ex - 1 >= 0 && ex - 1 < 128) {
                    u8g2->setDrawColor(0);
                    u8g2->drawPixel(ex - 1, ey);
                    u8g2->setDrawColor(1);
                }
                break;
                
            case ENEMY_BOSS:
                // BOSS（霸气战机）
                // 主机身
                if (ex - 6 >= 0 && ex + 3 < 128) {
                    u8g2->drawBox(ex - 6, ey - 3, 9, 6);
                }
                // 驾驶舱
                if (ex - 3 >= 0 && ex < 128) {
                    u8g2->setDrawColor(0);
                    u8g2->drawBox(ex - 3, ey - 1, 3, 2);
                    u8g2->setDrawColor(1);
                }
                // 机头（关键：确保两端都在屏幕内）
                if (ex - 8 >= 0 && ex - 7 < 128) {
                    u8g2->drawLine(ex - 7, ey - 1, ex - 8, ey);
                    u8g2->drawLine(ex - 7, ey + 1, ex - 8, ey);
                }
                if (ex - 9 >= 0 && ex - 9 < 128) {
                    u8g2->drawPixel(ex - 9, ey);
                }
                // 上翼（关键：确保两端都在屏幕内）
                if (ex - 5 >= 0 && ex + 1 < 128) {
                    u8g2->drawLine(ex - 5, ey - 4, ex + 1, ey - 4);
                }
                if (ex - 4 >= 0 && ex < 128) {
                    u8g2->drawLine(ex - 4, ey - 5, ex, ey - 5);
                }
                if (ex - 3 >= 0 && ex - 3 < 128) {
                    u8g2->drawPixel(ex - 3, ey - 6);
                }
                // 下翼（关键：确保两端都在屏幕内）
                if (ex - 5 >= 0 && ex + 1 < 128) {
                    u8g2->drawLine(ex - 5, ey + 4, ex + 1, ey + 4);
                }
                if (ex - 4 >= 0 && ex < 128) {
                    u8g2->drawLine(ex - 4, ey + 5, ex, ey + 5);
                }
                if (ex - 3 >= 0 && ex - 3 < 128) {
                    u8g2->drawPixel(ex - 3, ey + 6);
                }
                // 引擎
                if (ex + 3 < 128) {
                    u8g2->drawPixel(ex + 3, ey - 2);
                    u8g2->drawPixel(ex + 3, ey + 2);
                    if (frameCount % 4 < 2 && ex + 4 < 128) {
                        u8g2->drawPixel(ex + 4, ey - 2);
                        u8g2->drawPixel(ex + 4, ey + 2);
                    }
                }
                // 装甲板
                if (ex - 2 >= 0 && ex - 2 < 128) {
                    u8g2->drawPixel(ex - 2, ey - 3);
                    u8g2->drawPixel(ex - 2, ey + 3);
                }
                if (ex + 1 >= 0 && ex + 1 < 128) {
                    u8g2->drawPixel(ex + 1, ey - 3);
                    u8g2->drawPixel(ex + 1, ey + 3);
                }
                
                // BOSS血条（在BOSS上方，距离2px）
                if (ex >= 12 && ex <= 116) {
                    int hpBarWidth = (enemies[i].hp * 18) / enemies[i].maxHp;
                    // 血条背景
                    u8g2->drawFrame(ex - 9, ey - 9, 20, 3);
                    // 血条填充（红色效果用实心）
                    if (hpBarWidth > 0) {
                        u8g2->drawBox(ex - 8, ey - 8, hpBarWidth, 1);
                    }
                }
                break;
        }
    }
}

void ShooterPage::drawPowerUps(U8G2* u8g2) {
    u8g2->setFont(u8g2_font_4x6_tf);
    
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (!powerUps[i].active) continue;
        
        int px = (int)powerUps[i].x;
        int py = (int)powerUps[i].y;
        
        // 道具外框（8x8，避免与字母重叠）
        u8g2->drawFrame(px - 4, py - 4, 8, 8);
        
        // 根据类型绘制图标
        const char* icon = "";
        switch (powerUps[i].type) {
            case POWER_MISSILE: icon = "M"; break;
            case POWER_LASER: icon = "L"; break;
            case POWER_SPREAD: icon = "S"; break;
            case POWER_BEAM: icon = "E"; break;
            case POWER_HOMING: icon = "H"; break;
            case POWER_BOMB: icon = "B"; break;
            case POWER_SHIELD: icon = "O"; break;
            case POWER_LIFE:
                // 爱心（外框内绘制，居中）
                u8g2->drawFrame(px - 4, py - 4, 8, 8);
                u8g2->drawPixel(px - 1, py - 2);
                u8g2->drawPixel(px + 1, py - 2);
                u8g2->drawLine(px - 2, py - 1, px + 2, py - 1);
                u8g2->drawLine(px - 2, py, px + 2, py);
                u8g2->drawLine(px - 1, py + 1, px + 1, py + 1);
                u8g2->drawPixel(px, py + 2);
                continue;
        }
        
        // 字母完全居中绘制（4x6字体，在8x8框内）
        u8g2->drawStr(px - 2, py + 2, icon);
    }
}

void ShooterPage::drawParticles(U8G2* u8g2) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        
        int px = (int)particles[i].x;
        int py = (int)particles[i].y;
        
        // 防止坐标越界导致绘制错误
        if (px < 0 || px >= 128 || py < 0 || py >= 64) continue;
        
        if (particles[i].size == 1) {
            u8g2->drawPixel(px, py);
        } else {
            u8g2->drawBox(px, py, particles[i].size, particles[i].size);
        }
    }
}

void ShooterPage::drawExplosions(U8G2* u8g2) {
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        if (!explosions[i].active) continue;
        
        int ex = (int)explosions[i].x;
        int ey = (int)explosions[i].y;
        
        // 只绘制屏幕内的爆炸
        if (ex < -10 || ex > 138 || ey < -10 || ey > 74) continue;
        
        int frame = explosions[i].frame;
        int radius = frame * 2;
        
        // 爆炸圆环
        if (frame < 4 && ex >= 0 && ey >= 0) {
            u8g2->drawCircle(ex, ey, radius);
        }
        
        // 爆炸射线（确保不超出屏幕）
        // 修正：增加安全边距，防止 radius+2 导致坐标溢出或负数
        // 最大半径 radius=8 (frame=4), 延伸长度=2, 总需 10px 边距
        if (frame < 5 && ex >= 10 && ex < 118 && ey >= 10 && ey < 54) {
            u8g2->drawLine(ex - radius, ey, ex - radius - 2, ey);
            u8g2->drawLine(ex + radius, ey, ex + radius + 2, ey);
            u8g2->drawLine(ex, ey - radius, ex, ey - radius - 2);
            u8g2->drawLine(ex, ey + radius, ex, ey + radius + 2);
        }
    }
}

void ShooterPage::drawHUD(U8G2* u8g2) {
    u8g2->setFont(u8g2_font_4x6_tf);
    
    // 生命值（左上）- 爱心样式
    char hpStr[8];
    snprintf(hpStr, sizeof(hpStr), "HP:");
    u8g2->drawStr(2, 6, hpStr);
    // 爱心与HP文字对齐：文字baseline在6，高度6px，顶部约1px，底部6px
    // 爱心高度5px，应该从y=1开始到y=5结束，与文字对齐
    for (int i = 0; i < player.hp; i++) {
        int hx = 14 + i * 6;
        int hy = 1;  // 顶部对齐
        // 绘制爱心（5px高）
        u8g2->drawPixel(hx, hy);
        u8g2->drawPixel(hx + 2, hy);
        u8g2->drawLine(hx - 1, hy + 1, hx + 3, hy + 1);
        u8g2->drawLine(hx - 1, hy + 2, hx + 3, hy + 2);
        u8g2->drawLine(hx, hy + 3, hx + 2, hy + 3);
        u8g2->drawPixel(hx + 1, hy + 4);
    }
    
    // 分数（中上）
    char scoreStr[12];
    snprintf(scoreStr, sizeof(scoreStr), "%d", score);
    int scoreWidth = strlen(scoreStr) * 4;
    u8g2->drawStr(45, 6, scoreStr);
    
    // 关卡（中右上）
    char levelStr[8];
    snprintf(levelStr, sizeof(levelStr), "L%d", level);
    u8g2->drawStr(78, 6, levelStr);
    
    // 炸弹数（右上）
    char bombStr[8];
    snprintf(bombStr, sizeof(bombStr), "B:%d", bombs);
    u8g2->drawStr(100, 6, bombStr);
    
    // 当前武器（左上下方）
    if (player.weapon != WEAPON_NORMAL) {
        const char* weaponName = "";
        switch (player.weapon) {
            case WEAPON_MISSILE: weaponName = "M"; break;
            case WEAPON_LASER: weaponName = "L"; break;
            case WEAPON_SPREAD: weaponName = "S"; break;
            case WEAPON_BEAM: weaponName = "E"; break;
            case WEAPON_HOMING: weaponName = "H"; break;
            default: break;
        }
        u8g2->drawFrame(1, 8, 8, 8);
        u8g2->drawStr(3, 14, weaponName);
    }
}

void ShooterPage::drawStartScreen(U8G2* u8g2) {
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    
    // 标题
    const char* title = "雷霆战机";
    int titleWidth = u8g2->getUTF8Width(title);
    u8g2->drawUTF8((128 - titleWidth) / 2, 20, title);
    
    // 装饰性飞机
    u8g2->drawBox(50, 28, 6, 4);
    u8g2->drawLine(56, 30, 58, 30);
    u8g2->drawLine(48, 27, 50, 27);
    u8g2->drawLine(48, 33, 50, 33);
    
    // 操作说明
    u8g2->setFont(u8g2_font_4x6_tf);
    u8g2->drawStr(20, 42, "2/8/4/6:MOVE");
    u8g2->drawStr(20, 50, "5:SHOOT 9:BOMB");
    
    // 开始提示
    if (frameCount % 60 < 40) {
        u8g2->setFont(u8g2_font_wqy12_t_gb2312);
        const char* startText = "按确认键开始";
        int startWidth = u8g2->getUTF8Width(startText);
        u8g2->drawUTF8((128 - startWidth) / 2, 62, startText);
    }
}

void ShooterPage::drawLevelClear(U8G2* u8g2) {
    // 半透明遮罩
    for (int y = 15; y < 50; y += 2) {
        for (int x = 10 + (y % 4) / 2; x < 118; x += 2) {
            u8g2->setDrawColor(0);
            u8g2->drawPixel(x, y);
        }
    }
    u8g2->setDrawColor(1);
    
    // 通关框
    u8g2->drawFrame(8, 13, 112, 38);
    u8g2->setDrawColor(0);
    u8g2->drawBox(9, 14, 110, 36);
    u8g2->setDrawColor(1);
    u8g2->drawFrame(10, 15, 108, 34);
    
    // 标题
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    const char* clearText = "关卡完成!";
    int textWidth = u8g2->getUTF8Width(clearText);
    u8g2->drawUTF8((128 - textWidth) / 2, 28, clearText);
    
    // 分数
    u8g2->setFont(u8g2_font_5x7_tf);
    char scoreStr[20];
    snprintf(scoreStr, sizeof(scoreStr), "SCORE:%d", score);
    int scoreWidth = strlen(scoreStr) * 5;
    u8g2->drawStr((128 - scoreWidth) / 2, 38, scoreStr);
    
    // 下一关提示
    if (frameCount % 60 < 40) {
        u8g2->setFont(u8g2_font_4x6_tf);
        const char* nextText = "ENTER FOR NEXT";
        int nextWidth = strlen(nextText) * 4;
        u8g2->drawStr((128 - nextWidth) / 2, 47, nextText);
    }
}

void ShooterPage::drawGameOverScreen(U8G2* u8g2) {
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
    const char* gameOverText = "GAME OVER";
    int textWidth = u8g2->getUTF8Width(gameOverText);
    u8g2->drawUTF8((128 - textWidth) / 2, 28, gameOverText);
    
    // 最终分数
    u8g2->setFont(u8g2_font_5x7_tf);
    char finalScore[20];
    snprintf(finalScore, sizeof(finalScore), "SCORE:%d", score);
    int scoreWidth = strlen(finalScore) * 5;
    u8g2->drawStr((128 - scoreWidth) / 2, 38, finalScore);
    
    // 重新开始提示
    if (frameCount % 60 < 40) {
        u8g2->setFont(u8g2_font_4x6_tf);
        const char* restartText = "ENTER TO RESTART";
        int restartWidth = strlen(restartText) * 4;
        u8g2->drawStr((128 - restartWidth) / 2, 47, restartText);
    }
}

void ShooterPage::drawPausedScreen(U8G2* u8g2) {
    u8g2->setFont(u8g2_font_wqy12_t_gb2312);
    const char* pauseText = "暂停";
    int textWidth = u8g2->getUTF8Width(pauseText);
    u8g2->drawBox((128 - textWidth) / 2 - 2, 26, textWidth + 4, 14);
    u8g2->setDrawColor(0);
    u8g2->drawUTF8((128 - textWidth) / 2, 38, pauseText);
    u8g2->setDrawColor(1);
}

// ========== 按钮事件处理 ==========

void ShooterPage::onButtonBack(void* context) {
    if (gameState == PLAYING) {
        gameState = PAUSED;
    } else {
        uiEngine.navigateBack();
    }
}

void ShooterPage::onButtonEnter(void* context) {
    switch (gameState) {
        case START:
            gameState = PLAYING;
            break;
        case PAUSED:
            gameState = PLAYING;
            break;
        case LEVEL_CLEAR:
            level++;
            initLevel();
            player.x = 20;
            player.y = 28;
            gameState = PLAYING;
            break;
        case GAME_OVER:
            initGame();
            gameState = PLAYING;
            break;
    }
}

void ShooterPage::onButtonMenu(void* context) {
    if (gameState == PLAYING) {
        gameState = PAUSED;
    }
}

void ShooterPage::onButton1(void* context) {
    // 左上移动
    if (gameState == PLAYING) {
        player.x -= 3.5f;
        player.y -= 3.5f;
    }
}

void ShooterPage::onButton2(void* context) {
    // 上移
    if (gameState == PLAYING) {
        player.y -= 3.5f;
    }
}

void ShooterPage::onButton3(void* context) {
    // 右上移动
    if (gameState == PLAYING) {
        player.x += 3.5f;
        player.y -= 3.5f;
    }
}

void ShooterPage::onButton4(void* context) {
    // 左移
    if (gameState == PLAYING) {
        player.x -= 3.5f;
    }
}

void ShooterPage::onButton5(void* context) {
    // 射击（长按连续射击）
    if (gameState == PLAYING) {
        playerShoot();
    }
}

void ShooterPage::onButton6(void* context) {
    // 右移
    if (gameState == PLAYING) {
        player.x += 3.5f;
    }
}

void ShooterPage::onButton7(void* context) {
    // 左下移动
    if (gameState == PLAYING) {
        player.x -= 3.5f;
        player.y += 3.5f;
    }
}

void ShooterPage::onButton8(void* context) {
    // 下移
    if (gameState == PLAYING) {
        player.y += 3.5f;
    }
}

void ShooterPage::onButton9(void* context) {
    // 炸弹
    if (gameState == PLAYING) {
        useBomb();
    }
}

