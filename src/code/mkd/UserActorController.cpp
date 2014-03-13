/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "UserActorController.h"
#include "Game.h"
#include "Level.h"
#include "ctf/CtfMgr.h"

OIS::KeyCode UserActorController::s_inputCodes[] = { OIS::KC_UP, OIS::KC_DOWN, OIS::KC_LEFT, OIS::KC_RIGHT };
mkVec3       UserActorController::s_dirValues [] = { mkVec3::UNIT_Z, mkVec3::NEGATIVE_UNIT_Z, mkVec3::UNIT_X, mkVec3::NEGATIVE_UNIT_X };

UserActorController::UserActorController( ActorAI* ai )
    : IActorController(ai)
    , m_inputPending(false)
{
    for (int i = 0; i < DIR_COUNT; ++i)
        m_hadInput[i] = false;
}

void UserActorController::onUpdate( float dt )
{
    if (!m_inputPending)
        getAI()->setSpeed(0.f);
    else
        processInput();
}

void UserActorController::onDebugDraw()
{
    getAI()->drawSensesInfo();
}

void UserActorController::onDbgKeyDown( OIS::KeyCode key )
{
    for (int i = 0; i < DIR_COUNT; ++i)
    {
        if (s_inputCodes[i] == key)
        {
            m_hadInput[i] = true;
            m_inputPending = true;
        }
    }

    switch(key)
    {
    case OIS::KC_SPACE:
        if (getAI()->canJump())
            getAI()->jump();
        break;

    case OIS::KC_LSHIFT:
    case OIS::KC_RSHIFT:
        m_shift = true;
        break;

    case OIS::KC_TAB:
        if (getAI()->isDead())
            getAI()->revive();
        break;

    case OIS::KC_LCONTROL:
        if (getAI()->getLevel()->getCtfMgr() && getAI()->getLevel()->getCtfMgr()->isInCtfMode())
        {
            if (!getAI()->isCarryingFlag())
                getAI()->pickUpFlag(getAI()->getConflictSide());
            if (!getAI()->isCarryingFlag()) getAI()->pickUpFlag(EConflictSide::BlueTeam);
            if (!getAI()->isCarryingFlag()) getAI()->pickUpFlag(EConflictSide::RedTeam);
            if (!getAI()->isCarryingFlag()) getAI()->pickUpFlag(EConflictSide::Unknown);
        }
        break;

    case OIS::KC_RCONTROL:
        getAI()->dropFlag();
        break;
    }
}

void UserActorController::processInput()
{
    updateDirectionAndSpeed();
    clearInputs();
    m_inputPending = false;
}

void UserActorController::updateDirectionAndSpeed()
{
    mkVec3 result_dir = mkVec3::ZERO;
    
    for (int i = 0; i < DIR_COUNT; ++i)
        result_dir += m_hadInput[i] ? s_dirValues[i] : mkVec3::ZERO;

    if (result_dir.isZeroLength())
    {
        getAI()->setSpeed(0.f);
    }
    else
    {
        getAI()->setDirection(result_dir);
        getAI()->setSpeed(m_shift ? 1.f : .5f);
        m_shift = false;
    }
}

void UserActorController::clearInputs()
{
    for (int i = 0; i < DIR_COUNT; ++i)
        m_hadInput[i] = false;
}
