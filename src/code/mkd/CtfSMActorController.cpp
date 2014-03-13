/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "Game.h"
#include "CtfSMActorController.h"
#include "contrib/DebugDrawer.h"
#include "Level.h"
#include "MeshObject.h"
#include "ctf/CtfMgr.h"
#include "ctf/TeamFlag.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
CtfSMActorController::CtfSMActorController( ActorAI* ai )
    : StateMachineActorController(ai)
    , m_ctfMgr(ai->getLevel()->getCtfMgr())
{

}

void CtfSMActorController::onCreate()
{
    m_myBasePos = getCtfMgr()->getTeamBasePos(getAI()->getConflictSide());
    const float defense_radius = 10.f;
    m_defensePos = m_myBasePos + getRandomHorizontalDir() * defense_radius;

    scheduleTransitionInNextFrame(new ctf_sm::PatrolCtfState(this, mkVec3::UNIT_Z));
    //scheduleTransitionInNextFrame(new ctf_sm::Defense_GoToDefensePosCtfState(this));
    updateStateTransition(); // force update
}

void CtfSMActorController::onDebugDraw()
{
    __super::onDebugDraw();

    DebugDrawer::getSingleton().drawCircle(m_myBasePos, 10.f, 15, Ogre::ColourValue(1.f, 1.f, 1.f, 0.2f), false);
    DebugDrawer::getSingleton().drawCircle(m_defensePos, 1.f, 5, Ogre::ColourValue(0.f, 0.f, 0.f, 0.7f), false);
}

const Character* CtfSMActorController::findEnemyInSight() const
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

const Character* CtfSMActorController::findVisibleEnemyInShootingRange( float range_part ) const
{
    const CharacterVec& spotted_actors = getAI()->getSpottedActors();
    for (size_t i = 0; i < spotted_actors.size(); ++i)
    {
        const Character* other_character = spotted_actors[i];
        if (getAI()->isEnemy(other_character) && !other_character->isDead() && getAI()->isPosInShootingRange(other_character->getSimPos(), range_part))
            return other_character;
    }

    return NULL;
}

bool CtfSMActorController::shouldDefend()
{
    const int team_size = getCtfMgr()->getTeamSize(getAI()->getConflictSide());
    const int team_number = getAI()->getTeamNumber();

    return (team_number <= team_size / 3) && !shouldInterceptOwnFlag();
}

bool CtfSMActorController::shouldGrabEnemyFlag()
{
    if (shouldDefend())
        return false;

    CharacterVec my_team;
    getAI()->getCharactersFromSameTeam(my_team);

    for (size_t i = 0; i < my_team.size(); ++i)
    {
        ActorAI* actor = dynamic_cast<ActorAI*>(my_team[i]);
        if (actor && actor->isCarryingFlag() && !actor->isCarryingOwnFlag())
            return false;
    }

    int max_alive_number = 0;
    for (size_t i = 0; i < my_team.size(); ++i)
    {
        ActorAI* actor = dynamic_cast<ActorAI*>(my_team[i]);
        if (actor && !actor->isDead())
            max_alive_number = actor->getTeamNumber();
    }

    if (getAI()->getTeamNumber() >= max_alive_number)
        return true;

    return false;
}

bool CtfSMActorController::shouldInterceptOwnFlag()
{
    if (getAI()->isCarryingOwnFlag())
        return false;

    const float max_safe_dist_from_base = 3.f;
    TeamFlag* my_flag = getCtfMgr()->getTeamFlag(getAI()->getConflictSide());

    const mkVec3 flag_pos = my_flag->getWorldPosition();
    const mkVec3 base_pos = getCtfMgr()->getTeamBasePos(getAI()->getConflictSide());
    const mkVec3 base_pos_2d = mkVec3(base_pos.x, flag_pos.y, base_pos.z);

    if (flag_pos.squaredDistance(base_pos_2d) < max_safe_dist_from_base * max_safe_dist_from_base)
        return false;

    if (isFlagCarriedByUs())
        return false;

    return true;
}

EConflictSide::TYPE CtfSMActorController::getEnemyTeam() const
{
    for (int i = 0; i < EConflictSide::_COUNT; ++i)
    {
        EConflictSide::TYPE team = (EConflictSide::TYPE)i;

        if (team != getAI()->getConflictSide() && EConflictSide::areEnemies(getAI()->getConflictSide(), team))
            return team;
    }

    return EConflictSide::Unknown;
}

bool CtfSMActorController::isFlagCarriedByUs()
{
    CharacterVec my_team;
    getAI()->getCharactersFromSameTeam(my_team);

    for (size_t i = 0; i < my_team.size(); ++i)
    {
        ActorAI* actor = dynamic_cast<ActorAI*>(my_team[i]);
        if (actor && actor->isCarryingFlag() && actor->isCarryingOwnFlag())
            return true;
    }

    return false;
}

mkVec3 CtfSMActorController::getOurFlagPos() const
{
    return getCtfMgr()->getTeamFlag(getAI()->getConflictSide())->getWorldPosition();
}

CtfMgr* CtfSMActorController::getCtfMgr() const
{
    return m_ctfMgr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ctf_sm::BaseCtfState::BaseCtfState( CtfSMActorController* controller )
    : sm::State(controller)
{

}

CtfSMActorController* ctf_sm::BaseCtfState::getController() const
{
    return static_cast<CtfSMActorController*>(sm::State::getController());
}

CtfMgr* ctf_sm::BaseCtfState::getCtfMgr() const
{
    return getController()->getCtfMgr();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ctf_sm::PatrolCtfState::PatrolCtfState(CtfSMActorController* controller, const mkVec3& dir)
    : ctf_sm::BaseCtfState(controller)
    , m_direction(dir)
    , m_stateStartTime(-1.f)
{

}

void ctf_sm::PatrolCtfState::onUpdate(float dt)
{
    __super::onUpdate(dt);

    if (getController()->shouldInterceptOwnFlag())
    {
        getController()->scheduleTransitionInNextFrame(new ctf_sm::Defense_InterceptOwnFlag(getController()));
        return;
    }
    else if (getController()->shouldDefend())
    {
        getController()->scheduleTransitionInNextFrame(new ctf_sm::Defense_GoToDefensePosCtfState(getController()));
        return;
    }
    else if (getController()->shouldGrabEnemyFlag())
    {
        const mkVec3 enemy_flag_pos = getCtfMgr()->getTeamFlag(getController()->getEnemyTeam())->getWorldPosition();
        getController()->scheduleTransitionInNextFrame(new ctf_sm::Attack_GoToEnemyFlag(getController(), getController()->getEnemyTeam(), enemy_flag_pos));
        return;
    }

    const float time_to_change_state = 4000;
    const float cur_time = g_game->getTimeMs();
    if (cur_time - m_stateStartTime > time_to_change_state)
    {
        mkVec3 new_direction = getRandomHorizontalDir();
        ctf_sm::PatrolCtfState* next_state = new ctf_sm::PatrolCtfState(getController(), new_direction);
        getController()->scheduleTransitionInNextFrame(next_state);
    }
}

void ctf_sm::PatrolCtfState::onEnter(State* prev_state)
{
    __super::onEnter(prev_state);

    m_stateStartTime = g_game->getTimeMs();
    getAI()->setDirection(m_direction);
    getAI()->setSpeed(0.5f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ctf_sm::GoToState::GoToState(CtfSMActorController* controller, const mkVec3& dest_pos, float radius /* = 0.5f */)
    : ctf_sm::BaseCtfState(controller)
    , m_destPos(dest_pos)
    , m_destRadius(radius)
{

}

void ctf_sm::GoToState::onUpdate( float dt )
{
    __super::onUpdate(dt);

    rotateToDest();

    const mkVec3 my_pos = getAI()->getSimPos();
    const mkVec3 dest_pos_2d = mkVec3(m_destPos.x, my_pos.y, m_destPos.z);
    const float dist2_to_dest = my_pos.squaredDistance(dest_pos_2d);

    if (dist2_to_dest < m_destRadius * m_destRadius)
        onArrived();

    // Stop if further movement will increase dist
    const mkVec3 my_future_pos = my_pos + getAI()->getSimDir() * 0.01f;
    const float dist2_to_dest_future = my_future_pos.squaredDistance(dest_pos_2d);
    if (dist2_to_dest_future > dist2_to_dest)
        getAI()->setSpeed(0.f);
}

void ctf_sm::GoToState::onEnter( State* prev_state )
{
    __super::onEnter(prev_state);

    rotateToDest();
    getAI()->setSpeed(0.5f);
}

void ctf_sm::GoToState::rotateToDest()
{
    const mkVec3 my_pos = getAI()->getSimPos();

    mkVec3 direction = m_destPos - my_pos;
    direction.y = 0;
    direction.normalise();

    getAI()->setDirection(direction);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ctf_sm::Defense_GoToDefensePosCtfState::Defense_GoToDefensePosCtfState( CtfSMActorController* controller )
    : ctf_sm::GoToState(controller, controller->getDefensePos(), .5f)
{

}

void ctf_sm::Defense_GoToDefensePosCtfState::onArrived()
{
    __super::onArrived();

    getController()->scheduleTransitionInNextFrame(new ctf_sm::Defense_LookAround(getController()));
}

void ctf_sm::Defense_GoToDefensePosCtfState::onUpdate( float dt )
{
    __super::onUpdate(dt);

    if (getController()->shouldInterceptOwnFlag())
    {
        getController()->scheduleTransitionInNextFrame(new ctf_sm::Defense_InterceptOwnFlag(getController()));
        return;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ctf_sm::Defense_LookAround::Defense_LookAround( CtfSMActorController* controller )
    : ctf_sm::BaseCtfState(controller)
    , m_lastDirectionChangeTime(-1.f)
    , m_lastDirection(mkVec3::ZERO)
    , m_wantedDirection(mkVec3::ZERO)
{

}

void ctf_sm::Defense_LookAround::onUpdate( float dt )
{
    __super::onUpdate(dt);

    const float dir_lerp_time = 2000.f;
    const float cur_time = g_game->getTimeMs();
    const float time_from_dir_change = cur_time - m_lastDirectionChangeTime;
    const float lerp_progress = time_from_dir_change / dir_lerp_time;

    const mkVec3 cur_dir = slerp_horz_direction(lerp_progress, m_lastDirection, m_wantedDirection);
    getAI()->setDirection(cur_dir);

    if (lerp_progress > 1.f)
        changeWantedDir();

    const Character* enemy = getController()->findVisibleEnemyInShootingRange(0.75f);
    if (enemy != NULL)
    {
        getController()->scheduleTransitionInNextFrame(new ctf_sm::Defense_Shoot(getController(), enemy));
        return;
    }

    if (getController()->shouldInterceptOwnFlag())
    {
        getController()->scheduleTransitionInNextFrame(new ctf_sm::Defense_InterceptOwnFlag(getController()));
        return;
    }
}

void ctf_sm::Defense_LookAround::onEnter( State* prev_state )
{
    __super::onEnter(prev_state);

    getAI()->setSpeed(0.f);
    changeWantedDir();
}

void ctf_sm::Defense_LookAround::changeWantedDir()
{
    m_lastDirectionChangeTime = g_game->getTimeMs();
    m_lastDirection = getAI()->getSimDir();
    m_wantedDirection = getRandomHorizontalDir();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ctf_sm::Defense_Shoot::Defense_Shoot( CtfSMActorController* controller, const Character* enemy )
    : ctf_sm::BaseCtfState(controller)
    , m_enemy(enemy)
{

}

void ctf_sm::Defense_Shoot::onUpdate( float dt )
{
    __super::onUpdate(dt);

    if (m_enemy->isDead() || !getAI()->isPosInShootingRange(m_enemy->getSimPos()))
        getController()->scheduleTransitionInNextFrame(new ctf_sm::Defense_LookAround(getController()));
    else
    {
        getAI()->lookAt(m_enemy->getSimPos());

        const float min_hit_prob_to_shoot = 0.7f;
        if (getAI()->getCurrentShotHitProb() >= min_hit_prob_to_shoot)
            getAI()->shoot();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ctf_sm::Attack_GoToEnemyFlag::Attack_GoToEnemyFlag( CtfSMActorController* controller, EConflictSide::TYPE enemy_team, const mkVec3& flag_pos )
    : ctf_sm::GoToState(controller, flag_pos, 2.5f)
    , m_enemyTeam(enemy_team)
{

}

void ctf_sm::Attack_GoToEnemyFlag::onArrived()
{
    __super::onArrived();

    if (getAI()->isCarryingFlag() || getAI()->pickUpFlag(m_enemyTeam))
        getController()->scheduleTransitionInNextFrame(new ctf_sm::Attack_ReturnWithEnemyFlag(getController()));
}

void ctf_sm::Attack_GoToEnemyFlag::onUpdate( float dt )
{
    __super::onUpdate(dt);

    mkVec3 new_flag_pos = getCtfMgr()->getTeamFlag(m_enemyTeam)->getWorldPosition();
    updateDestPos(new_flag_pos);

    const Character* enemy = getController()->findVisibleEnemyInShootingRange(1.f);
    const float min_hit_prob = .7f;
    if (enemy)
    {
        getAI()->setSpeed(0.75f);
        if (getAI()->getCurrentShotHitProb() >= min_hit_prob)
        {
            getAI()->lookAt(enemy->getSimPos());
            getAI()->shoot();
            rotateToDest();
        }
    }
    else
    {
        getAI()->setSpeed(1.f);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ctf_sm::Attack_ReturnWithEnemyFlag::Attack_ReturnWithEnemyFlag( CtfSMActorController* controller )
    : ctf_sm::GoToState(controller, controller->getBasePos())
{

}

void ctf_sm::Attack_ReturnWithEnemyFlag::onArrived()
{
    __super::onArrived();

    getAI()->dropFlag();
    getController()->scheduleTransitionInNextFrame(new ctf_sm::PatrolCtfState(getController(), getRandomHorizontalDir()));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ctf_sm::Defense_InterceptOwnFlag::Defense_InterceptOwnFlag( CtfSMActorController* controller )
    : ctf_sm::GoToState(controller, controller->getOurFlagPos(), 2.5f)
{

}

void ctf_sm::Defense_InterceptOwnFlag::onArrived()
{
    __super::onArrived();

    if (getAI()->isCarryingOwnFlag() || getAI()->pickUpFlag(getAI()->getConflictSide()))
        getController()->scheduleTransitionInNextFrame(new ctf_sm::Defense_ReturnFlagToBase(getController()));
}

void ctf_sm::Defense_InterceptOwnFlag::onUpdate( float dt )
{
    __super::onUpdate(dt);

    updateDestPos(getController()->getOurFlagPos());
    getAI()->setSpeed(1.f);

    const Character* enemy = getController()->findVisibleEnemyInShootingRange(1.f);
    const float min_hit_prob = .5f;
    if (enemy && getAI()->getCurrentShotHitProb() >= min_hit_prob)
    {
        getAI()->lookAt(enemy->getSimPos());
        getAI()->shoot();
        rotateToDest();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ctf_sm::Defense_ReturnFlagToBase::Defense_ReturnFlagToBase( CtfSMActorController* controller )
    : ctf_sm::GoToState(controller, controller->getBasePos())
{

}

void ctf_sm::Defense_ReturnFlagToBase::onArrived()
{
    __super::onArrived();

    getAI()->dropFlag();
    getController()->scheduleTransitionInNextFrame(new ctf_sm::PatrolCtfState(getController(), getRandomHorizontalDir()));
}

void ctf_sm::Defense_ReturnFlagToBase::onUpdate( float dt )
{
    __super::onUpdate(dt);
    getAI()->setSpeed(1.f);
}
