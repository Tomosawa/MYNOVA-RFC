/* 
* Copyright (c) 2026 Tomosawa 
* https://github.com/Tomosawa/ 
* All rights reserved 
*/

#ifndef SELECTION_ANIMATION_H
#define SELECTION_ANIMATION_H
#include "Animation.h"
#include "ElementAnimation.h"

class SelectionAnimation : public ElementAnimation {
public:
    SelectionAnimation(AnimationType type = ANIME_NONE, uint16_t duration = 200);
    ~SelectionAnimation();
    
    // 正确地覆盖基类方法
    float easeInOut(float t) override;

    bool getBlinkState();
};

#endif // SELECTION_ANIMATION_H