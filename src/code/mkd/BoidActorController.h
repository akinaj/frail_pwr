/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "IActorController.h"

class BoidActorController : public IActorController
{
public:
    explicit BoidActorController(ActorAI* ai);
    ~BoidActorController();

    virtual void onCreate();
    virtual void onTakeDamage(const SDamageInfo& dmg_info);
    virtual void onUpdate(float dt);
    virtual void onDebugDraw();

private:
    mkVec3 m_wantedVelocity;
    std::vector<Character*> m_neighbors;

    struct Obstacle
    {
        mkVec3 pos;
    };

    std::vector<Obstacle> m_obstacles;

    mkVec3 m_dbgAlignmentVec;
    mkVec3 m_dbgCohesionVec;
    mkVec3 m_dbgSeparationVec;

    void updateObstacles();
    void updateNeighbors();
    void updateWantedVelocity(float dt);
    void applyWantedVelocity();
    mkVec3 calcAcceleration();

    mkVec3 calcAlignmentAcc();
    mkVec3 calcCohesionAcc();
    mkVec3 calcSeparationAcc();
};
