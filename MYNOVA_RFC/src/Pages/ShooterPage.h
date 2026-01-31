/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// ShooterPage.h
#ifndef ShooterPage_h
#define ShooterPage_h

#include "../GUI/UIPage.h"
#include <U8g2lib.h>

// 游戏配置
#define SHOOTER_SCREEN_WIDTH    128
#define SHOOTER_SCREEN_HEIGHT   64

class ShooterPage : public UIPage {
public:
    // 游戏状态
    enum GameState {
        START,
        PLAYING,
        PAUSED,
        LEVEL_CLEAR,
        GAME_OVER
    };
    
    // 武器类型
    enum WeaponType {
        WEAPON_NORMAL = 0,  // 普通子弹
        WEAPON_MISSILE,     // 导弹M（追踪）
        WEAPON_LASER,       // 激光L（穿透）
        WEAPON_SPREAD,      // 散弹S（扇形）
        WEAPON_BEAM,        // 光束E（光圈弹范围大）
        WEAPON_HOMING       // 制导H（连续爆炸）
    };
    
    // 道具类型
    enum PowerUpType {
        POWER_NONE = 0,
        POWER_MISSILE,      // M - 导弹
        POWER_LASER,        // L - 激光
        POWER_SPREAD,       // S - 散弹
        POWER_BEAM,         // E - 光束
        POWER_HOMING,       // H - 制导
        POWER_LIFE,         // ❤ - 生命
        POWER_BOMB,         // B - 炸弹
        POWER_SHIELD        // O - 护盾
    };
    
    // 敌机类型
    enum EnemyType {
        ENEMY_SMALL = 0,    // 小型敌机（直线飞行）
        ENEMY_MEDIUM,       // 中型敌机（波浪飞行+射击）
        ENEMY_HEAVY,        // 重型敌机（追踪+密集弹幕）
        ENEMY_BOSS          // BOSS
    };
    
    // 玩家飞机
    struct Player {
        float x, y;
        int hp;
        int maxHp;
        WeaponType weapon;
        int weaponLevel;
        int weaponTimer;
        bool shielded;
        int shieldTimer;
        bool invincible;
        int invincibleTimer;
    };
    
    // 子弹结构
    struct Bullet {
        float x, y;
        float vx, vy;
        int damage;
        WeaponType type;
        bool active;
        bool isPlayerBullet;
        int chainExplosions;  // HOMING导弹的连环爆炸次数
        bool isExploding;     // HOMING导弹是否正在连环爆炸
        float explosionCenterX, explosionCenterY;  // 连环爆炸中心点
        int explosionTimer;   // 爆炸间隔计时器
    };
    
    // 敌机结构
    struct Enemy {
        float x, y;
        float vx, vy;
        int hp;
        int maxHp;
        EnemyType type;
        int shootTimer;
        int movePattern;
        float moveTimer;
        bool active;
    };
    
    // 道具结构
    struct PowerUp {
        float x, y;
        float vx, vy;
        PowerUpType type;
        bool active;
        int lifetime;  // 生命周期（帧数）
    };
    
    // 粒子效果
    struct Particle {
        float x, y;
        float vx, vy;
        int life;
        int size;
        bool active;
    };
    
    // 爆炸效果
    struct Explosion {
        float x, y;
        int frame;
        int maxFrame;
        bool active;
    };
    
    ShooterPage();
    ~ShooterPage();
    
    void showPage() override;
    void render(U8G2* u8g2) override;
    void update() override;
    
    // 按钮事件
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
    
    GameState gameState;
    
    // 玩家数据
    Player player;
    
    // 子弹池
    static const int MAX_BULLETS = 30;
    Bullet bullets[MAX_BULLETS];
    int playerShootCooldown;
    int laserBeamTimer;  // 激光束持续时间
    
    // 敌机池
    static const int MAX_ENEMIES = 10;
    Enemy enemies[MAX_ENEMIES];
    
    // 道具池
    static const int MAX_POWERUPS = 5;
    PowerUp powerUps[MAX_POWERUPS];
    
    // 特效池
    static const int MAX_PARTICLES = 40;
    Particle particles[MAX_PARTICLES];
    
    static const int MAX_EXPLOSIONS = 8;
    Explosion explosions[MAX_EXPLOSIONS];
    
    // 游戏数据
    int score;
    int level;
    int enemiesKilled;
    int bombs;
    int frameCount;
    
    // 关卡数据
    int levelProgress;
    bool bossActive;
    int spawnTimer;
    int bossLaserTimer;  // BOSS激光计时器
    int bossLaserY;      // BOSS激光Y坐标
    int bossRotatingAngle;  // BOSS旋转弹幕角度
    
    // 背景滚动
    int bgScroll;
    
    // 游戏逻辑
    void initGame();
    void initLevel();
    void updateGame();
    void updatePlayer();
    void updateBullets();
    void updateEnemies();
    void updatePowerUps();
    void updateParticles();
    void updateExplosions();
    
    // 战斗逻辑
    void playerShoot();
    void enemyShoot(Enemy* enemy);
    void enemyShootBoss(Enemy* enemy);
    void useBomb();
    void checkCollisions();
    void spawnEnemy();
    void spawnPowerUp(float x, float y, PowerUpType type);
    void spawnExplosion(float x, float y, int size);
    void spawnParticles(float x, float y, int count, int size);
    
    // 渲染函数
    void drawBackground(U8G2* u8g2);
    void drawPlayer(U8G2* u8g2);
    void drawBullets(U8G2* u8g2);
    void drawEnemies(U8G2* u8g2);
    void drawPowerUps(U8G2* u8g2);
    void drawParticles(U8G2* u8g2);
    void drawExplosions(U8G2* u8g2);
    void drawHUD(U8G2* u8g2);
    void drawStartScreen(U8G2* u8g2);
    void drawLevelClear(U8G2* u8g2);
    void drawGameOverScreen(U8G2* u8g2);
    void drawPausedScreen(U8G2* u8g2);
    
    // 辅助函数
    Enemy* findNearestEnemy();
    void damageEnemy(Enemy* enemy, int damage);
};

#endif
