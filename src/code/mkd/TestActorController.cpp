/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "TestActorController.h"
#include "Game.h"

TestActorController::TestActorController( ActorAI* ai )
    : IActorController(ai)
    , m_lastTurnTime(-1.f)
{

}

void TestActorController::onTakeDamage(const SDamageInfo& dmg_info)
{

}

void TestActorController::onUpdate( float dt )
{
    updateRandomDirectionChange();
    updateJumpingOnObstacle();
}

void TestActorController::onDebugDraw()
{
    getAI()->drawSensesInfo();
}

void TestActorController::updateRandomDirectionChange()
{
    const float cur_time = g_game->getTimeMs();
    const float turn_period = 10000.f;
    const float interpolation_time = 750.f;

    const float time_since_dir_change = m_lastTurnTime < 0 ? turn_period : cur_time - m_lastTurnTime;

    if (time_since_dir_change >= turn_period)
    {
        m_curBaseDir = getAI()->getSimDir();
        m_curDestDir = getRandomHorizontalDir();
        m_lastTurnTime = cur_time;
    }
    else if (time_since_dir_change <= interpolation_time)
    {
        const float t = time_since_dir_change / interpolation_time;
        const mkVec3 cur_dir = slerp_horz_direction(t, m_curBaseDir, m_curDestDir);
        getAI()->setDirection(cur_dir);
    }
    else
    {
        getAI()->setDirection(m_curDestDir);
    }
}

void TestActorController::updateJumpingOnObstacle()
{
    if (getAI()->canJump())
    {
        RayCastResult ray_feet = getAI()->raycast(getAI()->getSimDir(), 0.25f, 2.5f);
        if (ray_feet.hit && ray_feet.collision_type == RayCastResult::Environment)
        {
            RayCastResult ray_eyes = getAI()->raycast(getAI()->getSimDir(), 1.f, 2.f);
            if (!ray_eyes.hit)
                getAI()->jump();
        }
    }
}