/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "Game.h"
#include "StateMachineActorController.h"
#include "contrib/DebugDrawer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
StateMachineActorController::StateMachineActorController( ActorAI* ai )
    : IActorController(ai)
    , m_nextState(NULL)
    , m_currentState(NULL)
{
    
}

StateMachineActorController::~StateMachineActorController()
{
    delete m_currentState;
    delete m_nextState;
}

void StateMachineActorController::onCreate()
{
    scheduleTransitionInNextFrame(new sm::PatrolState(this, mkVec3::UNIT_Z));
    updateStateTransition(); // force update
}

void StateMachineActorController::updateStateTransition()
{
    if (m_nextState)
    {
        if (m_currentState)
            m_currentState->onLeave(m_nextState);

        m_nextState->onEnter(m_currentState);

        delete m_currentState;
        m_currentState = m_nextState;
        m_nextState = NULL;
    }
}

void StateMachineActorController::onTakeDamage(const SDamageInfo& dmg_info)
{
    m_currentState->onTakeDamage();
}

void StateMachineActorController::onUpdate( float dt )
{
    updateStateTransition();
    m_currentState->onUpdate(dt);
}

void StateMachineActorController::onDebugDraw()
{
    getAI()->drawSensesInfo();
    m_currentState->onDebugDraw();
}

void StateMachineActorController::scheduleTransitionInNextFrame( sm::State* next )
{
    MK_ASSERT(next);
    MK_ASSERT(m_currentState != next);
    MK_ASSERT_MSG(m_nextState == NULL, "Tried to schedule state transition twice in one frame");

    m_nextState = next;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
sm::PatrolState::PatrolState(StateMachineActorController* controller, const mkVec3& direction)
    : sm::State(controller)
    , m_direction(direction)
    , m_stateStartTime(-1.f)
{
    
}

void sm::PatrolState::onUpdate( float dt )
{
    __super::onUpdate(dt);

    const Character* enemy = findEnemyInSight();
    if (enemy)
    {
        sm::ChaseState* next_state = new sm::ChaseState(getController(), enemy);
        getController()->scheduleTransitionInNextFrame(next_state);
    }
    else
    {
        const float time_to_change_state = 4000;
        const float cur_time = g_game->getTimeMs();
        if (cur_time - m_stateStartTime > time_to_change_state)
        {
            mkVec3 new_direction = getRandomHorizontalDir();
            sm::PatrolState* next_state = new sm::PatrolState(getController(), new_direction);
            getController()->scheduleTransitionInNextFrame(next_state);
        }
    }
}

void sm::PatrolState::onEnter( State* prev_state )
{
    __super::onEnter(prev_state);

    m_stateStartTime = g_game->getTimeMs();
    getAI()->setDirection(m_direction);
    getAI()->setSpeed(0.5f);
}

const Character* sm::PatrolState::findEnemyInSight() const
{
    const CharacterVec& spotted_actors = getAI()->getSpottedActors();
    for (size_t i = 0; i < spotted_actors.size(); ++i)
    {
        const Character* other_character = spotted_actors[i];
        if (getAI()->isEnemy(other_character) && !other_character->isDead())
            return other_character;
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
sm::ChaseState::ChaseState( StateMachineActorController* controller, const Character* enemy )
    : sm::State(controller)
    , m_enemy(enemy)
{

}

void sm::ChaseState::onUpdate( float dt )
{
    __super::onUpdate(dt);

    if (getAI()->isCharacterVisible(m_enemy))
    {
        const float dist_to_enemy = (m_enemy->getSimPos() - getAI()->getSimPos()).length();
        const float shoot_range_part_to_shoot = 0.7f;
        if (dist_to_enemy < getAI()->getShootingRange() * shoot_range_part_to_shoot)
        {
            sm::ShootState* next_state = new sm::ShootState(getController(), m_enemy);
            getController()->scheduleTransitionInNextFrame(next_state);
        }

        getAI()->lookAt(m_enemy->getSimPos());
    }
    else
    {
        // TODO enemy lost state?
        sm::PatrolState* next_state = new sm::PatrolState(getController(), getRandomHorizontalDir());
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void sm::ChaseState::onEnter( State* prev_state )
{
    __super::onEnter(prev_state);

    getAI()->setSpeed(1.f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
sm::ShootState::ShootState( StateMachineActorController* controller, const Character* enemy )
    : sm::State(controller)
    , m_enemy(enemy)
    , m_lastShotTime(-1.f)
{

}

void sm::ShootState::onUpdate( float dt )
{
    __super::onUpdate(dt);

    getAI()->lookAt(m_enemy->getSimPos());

    if (isEnemyDeadOrLost())
        goToPatrol();
    else if (!isEnemyInShootingRange())
        resumeChase();
    else if (getShotPrepareProgress() >= 1.f)
        shoot();
}

void sm::ShootState::onEnter( State* prev_state )
{
    __super::onEnter(prev_state);
    
    getAI()->setSpeed(0.f);
    m_lastShotTime = g_game->getTimeMs();
}

void sm::ShootState::onDebugDraw()
{
    const float shot_progress = getShotPrepareProgress();
    Ogre::ColourValue colour(0.f, 0.f, 0.f, shot_progress);
    DebugDrawer::getSingleton().drawCircle(getAI()->getSimPos(), getAI()->getShootingRange(), 15, colour, true);
}

float sm::ShootState::getShotPrepareProgress() const
{
    const float shot_prepare_time = 2000.f;
    const float cur_time = g_game->getTimeMs();
    const float time_passed = cur_time - m_lastShotTime;

    return time_passed / shot_prepare_time;
}

void sm::ShootState::shoot()
{
    getAI()->shoot();
    m_lastShotTime = g_game->getTimeMs();
}

bool sm::ShootState::isEnemyDeadOrLost() const
{
    return m_enemy->isDead() || !getAI()->isCharacterVisible(m_enemy);
}

bool sm::ShootState::isEnemyInShootingRange() const
{
    const float dist2_to_enemy = (m_enemy->getSimPos() - getAI()->getSimPos()).squaredLength();
    return (dist2_to_enemy < getAI()->getSquaredShootingRange());
}

void sm::ShootState::goToPatrol()
{
    sm::PatrolState* next_state = new sm::PatrolState(getController(), getAI()->getSimDir());
    getController()->scheduleTransitionInNextFrame(next_state);
}

void sm::ShootState::resumeChase()
{
    sm::ChaseState* next_state = new sm::ChaseState(getController(), m_enemy);
    getController()->scheduleTransitionInNextFrame(next_state);
}
