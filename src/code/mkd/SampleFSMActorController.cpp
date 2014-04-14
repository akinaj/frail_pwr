#include "pch.h"
#include "Game.h"
#include "SampleFSMActorController.h"
#include "contrib/DebugDrawer.h"
#include "Level.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
SampleFSMActorController::SampleFSMActorController( ActorAI* ai )
    : StateMachineActorController(ai)
{

}

void SampleFSMActorController::onCreate()
{
    getAI()->lookAt(mkVec3::ZERO);
    scheduleTransitionInNextFrame(new sample_sm::IdleState(this));
    updateStateTransition();
}

void SampleFSMActorController::onDebugDraw()
{
    __super::onDebugDraw();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sample_sm::BaseState::BaseState( SampleFSMActorController* controller )
    : sm::State(controller)
{

}

SampleFSMActorController* sample_sm::BaseState::getController() const
{
    return static_cast<SampleFSMActorController*>(sm::State::getController());
}
////////////////////////////////////////////////////////////////////////////////////////////////////
sample_sm::IdleState::IdleState(SampleFSMActorController* controller)
    : sample_sm::BaseState(controller)
    , m_stateStartTime(-1.f)
{

}

void sample_sm::IdleState::onUpdate(float dt)
{
    __super::onUpdate(dt);

    Character* target = getAI()->findClosestEnemyInSight();
    if(target){
        getController()->scheduleTransitionInNextFrame(new sample_sm::AttackState(getController(),target));
    }

    const float time_to_change_state = 2000;
    const float cur_time = g_game->getTimeMs();
    if (cur_time - m_stateStartTime > time_to_change_state)
    {
        sample_sm::IdleState* next_state = new sample_sm::IdleState(getController());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void sample_sm::IdleState::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();
    getAI()->setSpeed(0.f);
}

void sample_sm::IdleState::onTakeDamage()
{
    getAI()->jump();
}

void sample_sm::IdleState::onDebugDraw()
{

}

void sample_sm::IdleState::onLeave( State* next_state )
{
    getAI()->runAnimation("Backflip",800);
}

//////////////////////////////////////////////////////////////////////////

sample_sm::AttackState::AttackState(SampleFSMActorController* controller, Character* target)
    : sample_sm::BaseState(controller)
    , m_target(target)
    , m_stateStartTime(-1.f)
{

}

void sample_sm::AttackState::onUpdate(float dt)
{
    __super::onUpdate(dt);

    Character* target = getAI()->findClosestEnemyInSight();
    if(!target){
        getController()->scheduleTransitionInNextFrame(new sample_sm::IdleState(getController()));
    }

    const float time_to_change_state = 1200;
    const float cur_time = g_game->getTimeMs();
    if (cur_time - m_stateStartTime > time_to_change_state)
    {
        getAI()->runAnimation("Attack3",time_to_change_state);
        sample_sm::AttackState* next_state = new sample_sm::AttackState(getController(),target);
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void sample_sm::AttackState::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);
    m_stateStartTime = g_game->getTimeMs();
    getAI()->setSpeed(0.f);
}

void sample_sm::AttackState::onLeave( State* next_state )
{
    getAI()->hitMelee();
}
