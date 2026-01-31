/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// TankBattlePage.h
#ifndef TankBattlePage_h
#define TankBattlePage_h

#include "../GUI/UIPage.h"
#include "../GUIRender.h"
#include <U8g2lib.h>

// 游戏区域配置
#define TANK_BOARD_WIDTH    26   // 游戏区域宽度（格子数）
#define TANK_BOARD_HEIGHT   12   // 游戏区域高度（格子数）- 减少1行，留空间给顶部信息
#define TANK_CELL_SIZE      4    // 每个格子的像素大小
#define TANK_BOARD_X        13   // 游戏区域起始X坐标（居中：(128-26*4)/2）
#define TANK_BOARD_Y        10   // 游戏区域起始Y坐标 - 下移，顶部留给信息栏

// 地图元素
#define TILE_EMPTY          0    // 空地
#define TILE_BRICK          1    // 砖墙（可摧毁）
#define TILE_STEEL          2    // 钢墙（不可摧毁）
#define TILE_WATER          3    // 水域（玩家不可通过）
#define TILE_GRASS          4    // 草地（装饰）
#define TILE_BASE           5    // 基地

// 游戏配置
#define TANK_MAX_BULLETS         4    // 最大子弹数
#define TANK_MAX_ENEMIES         3    // 同时存在的最大敌人数
#define TANK_TOTAL_ENEMIES       10   // 每关总敌人数

class TankBattlePage : public UIPage {
public:
    TankBattlePage();
    ~TankBattlePage();
    
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
    
    // 方向枚举
    enum Direction {
        DIR_UP = 0,
        DIR_DOWN,
        DIR_LEFT,
        DIR_RIGHT
    };
    
    // 坦克结构
    struct Tank {
        float x, y;          // 位置（格子坐标）
        Direction dir;       // 方向
        bool active;         // 是否存在
        int hp;              // 生命值
        int moveCounter;     // 移动计数器
        int shootCooldown;   // 射击冷却
    };
    
    // 子弹结构
    struct Bullet {
        float x, y;
        Direction dir;
        bool active;
        bool isPlayer;       // 是否是玩家的子弹
    };
    
    GameState gameState;
    
    // 地图数据
    uint8_t map[TANK_BOARD_HEIGHT][TANK_BOARD_WIDTH];
    
    // 玩家坦克
    Tank player;
    
    // 敌人坦克
    Tank enemies[TANK_MAX_ENEMIES];
    int enemiesAlive;        // 存活的敌人数
    int enemiesRemaining;    // 剩余待出现的敌人数
    int spawnCounter;        // 生成计数器
    int nextSpawnPos;        // 下一个生成位置索引（0=左，1=中，2=右）
    
    // 子弹
    Bullet bullets[TANK_MAX_BULLETS];
    
    // 基地位置
    int baseX, baseY;
    bool baseDestroyed;
    
    // 游戏数据
    int score;
    int level;
    int enemiesKilled;       // 本关击杀数
    
    // 动画相关
    int frameCount;
    int explosionTimer;
    int explosionX, explosionY;
    
    // 游戏初始化
    void initGame();
    void initLevel();
    void generateMap(int levelNum);
    
    // 游戏逻辑
    void updateGame();
    void updatePlayer();
    void updateEnemies();
    void updateBullets();
    void spawnEnemy();
    
    // 移动和碰撞
    bool canMove(float x, float y);
    bool checkTankCollision(float x, float y, Tank* exceptTank);
    void moveTank(Tank* tank, Direction dir);
    void shoot(Tank* tank, bool isPlayer);
    
    // AI
    void enemyAI(Tank* enemy);
    
    // 爆炸效果
    void createExplosion(int x, int y);
    void destroyTile(int x, int y);
    
    // 绘制函数
    void drawMap(U8G2* u8g2);
    void drawTank(U8G2* u8g2, Tank* tank, bool isPlayer);
    void drawBullets(U8G2* u8g2);
    void drawExplosion(U8G2* u8g2);
    void drawHUD(U8G2* u8g2);
    void drawBorder(U8G2* u8g2);
    void drawStartScreen(U8G2* u8g2);
    void drawGameOverScreen(U8G2* u8g2);
    void drawPausedScreen(U8G2* u8g2);
    void drawLevelClearScreen(U8G2* u8g2);
    
    // 辅助函数
    int getPixelX(int gridX);
    int getPixelY(int gridY);
};

#endif
