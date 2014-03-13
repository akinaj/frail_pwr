/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "IActorController.h"
#include "utils.h"

class TestActorController : public IActorController
{
public:
    explicit TestActorController(ActorAI* ai);

    virtual void onCreate() { }
    virtual void onTakeDamage(const SDamageInfo& dmg_info);
    virtual void onUpdate(float dt);
    virtual void onDebugDraw();

private:
    void updateRandomDirectionChange();
    void updateJumpingOnObstacle();

    float m_lastTurnTime;
    mkVec3 m_curBaseDir;
    mkVec3 m_curDestDir;
};
