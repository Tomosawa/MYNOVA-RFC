/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

// RacingPage.h
#ifndef RacingPage_h
#define RacingPage_h

#include "../GUI/UIPage.h"
#include "../GUIRender.h"
#include <U8g2lib.h>

// 游戏区域配置
#define RACING_SCREEN_WIDTH     128
#define RACING_SCREEN_HEIGHT    64
#define RACING_ROAD_SEGMENTS    32      // 道路分段数（用于透视效果）
#define RACING_VIEW_DISTANCE    20      // 视野距离

class RacingPage : public UIPage {
public:
    RacingPage();
    ~RacingPage();
    
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
    // 游戏状态
    enum GameState {
        START,
        COUNTDOWN,
        PLAYING,
        PAUSED,
        GAME_OVER
    };
    
    // 道路类型
    enum RoadType {
        ROAD_STRAIGHT = 0,
        ROAD_LEFT_CURVE,
        ROAD_RIGHT_CURVE
    };
    
    // 障碍物类型
    enum ObstacleType {
        OBS_NONE = 0,
        OBS_CAR,        // 敌车
        OBS_TRUCK,      // 卡车（更宽）
        OBS_OIL,        // 油渍（碰到减速）
        OBS_COIN        // 金币（加分）
    };
    
    // 道路段结构
    struct RoadSegment {
        float curve;        // 弯曲度
        float y;            // Y坐标（世界坐标）
        RoadType type;      // 道路类型
    };
    
    // 障碍物结构
    struct Obstacle {
        float x;            // X位置（-1到1，0为中心）
        float z;            // Z位置（深度）
        ObstacleType type;  // 障碍物类型
        bool active;        // 是否激活
        int lane;           // 车道（0=左，1=中，2=右）
    };
    
    // 粒子效果（速度线、爆炸等）
    struct Particle {
        float x, y;
        float vx, vy;
        int life;
        bool active;
    };
    
    GameState gameState;
    
    // 玩家数据
    float playerX;          // 玩家X位置（-1到1）
    float playerSpeed;      // 当前速度
    float playerTargetSpeed;// 目标速度
    int playerLane;         // 当前车道
    bool crashed;           // 是否撞车
    int crashTimer;         // 撞车计时
    
    // 世界数据
    float cameraZ;          // 摄像机Z位置（行驶距离）
    float roadCurve;        // 当前道路弯曲度
    float roadCurveTarget;  // 目标弯曲度
    
    // 游戏数据
    int score;
    int distance;           // 行驶距离（米）
    int speed;              // 显示速度（km/h）
    int fuel;               // 燃料（0-100）
    int coins;              // 金币数
    int countdown;          // 倒计时
    
    // 障碍物
    static const int MAX_OBSTACLES = 8;
    Obstacle obstacles[MAX_OBSTACLES];
    
    // 粒子效果
    static const int MAX_PARTICLES = 20;
    Particle particles[MAX_PARTICLES];
    
    // 动画
    int frameCount;
    int roadAnimOffset;     // 道路动画偏移
    
    // 游戏逻辑
    void initGame();
    void updateGame();
    void updatePlayer();
    void updateCamera();
    void updateObstacles();
    void updateParticles();
    void generateObstacles();
    void checkCollision();
    void checkRoadBoundary();
    void spawnParticles(float x, float y, int count);
    void shakeScreen();
    
    // 渲染函数
    void drawRoad(U8G2* u8g2);
    void drawRoadSegment(U8G2* u8g2, int segmentIndex, float camZ);
    void drawObstacles(U8G2* u8g2);
    void drawPlayer(U8G2* u8g2);
    void drawParticles(U8G2* u8g2);
    void drawHUD(U8G2* u8g2);
    void drawSpeedometer(U8G2* u8g2);
    void drawStartScreen(U8G2* u8g2);
    void drawCountdown(U8G2* u8g2);
    void drawGameOverScreen(U8G2* u8g2);
    void drawPausedScreen(U8G2* u8g2);
    
    // 辅助函数
    float getCurveAtZ(float z);
    void projectToScreen(float worldX, float worldZ, int& screenX, int& screenY, int& width);
    int getLaneX(int lane);
};

#endif
