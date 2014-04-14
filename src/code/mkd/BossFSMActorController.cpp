#include "pch.h"
#include "Game.h"
#include "BossFSMActorController.h"
#include "contrib/DebugDrawer.h"
#include "Level.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
BossFSMActorController::BossFSMActorController( ActorAI* ai )
    : StateMachineActorController(ai)
{

}

void BossFSMActorController::onCreate()
{
    scheduleTransitionInNextFrame(new boss_sm::PatrolState(this, mkVec3::UNIT_Z));
    updateStateTransition();
}

void BossFSMActorController::onDebugDraw()
{
    __super::onDebugDraw();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

boss_sm::BaseState::BaseState( BossFSMActorController* controller )
    : sm::State(controller)
{

}

BossFSMActorController* boss_sm::BaseState::getController() const
{
    return static_cast<BossFSMActorController*>(sm::State::getController());
}
////////////////////////////////////////////////////////////////////////////////////////////////////
boss_sm::PatrolState::PatrolState(BossFSMActorController* controller, const mkVec3& dir)
    : boss_sm::BaseState(controller)
    , m_direction(dir)
    , m_stateStartTime(-1.f)
{

}

void boss_sm::PatrolState::onUpdate(float dt)
{
    __super::onUpdate(dt);

    //if (getController()->shouldInterceptOwnFlag())
    //{
    //    getController()->scheduleTransitionInNextFrame(new ctf_sm::Defense_InterceptOwnFlag(getController()));
    //    return;
    //}
    //else if (getController()->shouldDefend())
    //{
    //    getController()->scheduleTransitionInNextFrame(new ctf_sm::Defense_GoToDefensePosCtfState(getController()));
    //    return;
    //}
    //else if (getController()->shouldGrabEnemyFlag())
    //{
    //    const mkVec3 enemy_flag_pos = getCtfMgr()->getTeamFlag(getController()->getEnemyTeam())->getWorldPosition();
    //    getController()->scheduleTransitionInNextFrame(new ctf_sm::Attack_GoToEnemyFlag(getController(), getController()->getEnemyTeam(), enemy_flag_pos));
    //    return;
    //}

    const float time_to_change_state = 4000;
    const float cur_time = g_game->getTimeMs();
    if (cur_time - m_stateStartTime > time_to_change_state)
    {
        mkVec3 new_direction = getRandomHorizontalDir();
        boss_sm::PatrolState* next_state = new boss_sm::PatrolState(getController(), new_direction);
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void boss_sm::PatrolState::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);

    m_stateStartTime = g_game->getTimeMs();
    getAI()->setDirection(m_direction);
    getAI()->setSpeed(0.5f);
}