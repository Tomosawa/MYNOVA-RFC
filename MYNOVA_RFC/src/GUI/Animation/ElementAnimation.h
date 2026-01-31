/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef ELEMENT_ANIMATION_H
#define ELEMENT_ANIMATION_H

#include <Arduino.h>
#include "Animation.h"

class ElementAnimation {
    public:
        ElementAnimation(AnimationType type = ANIME_NONE, uint16_t duration = 200);
        virtual ~ElementAnimation() {}

        virtual void start();
        virtual void update();
        virtual void stop();
        
        void setType(AnimationType type);
        void setDuration(uint16_t duration);
        AnimationType getType();
        uint16_t getDuration();
        
        // 添加基类中的 easeInOut 方法，使其可以被子类覆盖
        virtual float easeInOut(float t);
        
        bool isRunning();
        float getProgress();
        
    protected:
        bool running;
        uint32_t startTime;
        uint16_t duration;
        float progress;
        AnimationType type;
};

#endif // ELEMENT_ANIMATION_H