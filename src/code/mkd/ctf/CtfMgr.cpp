/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "CtfMgr.h"
#include "utils.h"
#include "TeamFlag.h"
#include "Level.h"

IMPLEMENT_RTTI_NOSCRIPT(CtfMgr, GameObject);

START_RTTI_INIT(CtfMgr);
{

}
END_RTTI_INIT();

CtfMgr::CtfMgr()
{
    zero_array(m_teamSize);
}

void CtfMgr::onPostCreate()
{
    __super::onPostCreate();

    TGameObjectVec flags;
    getLevel()->findObjectsOfType(flags, &TeamFlag::Type);

    for (size_t i = 0; i < flags.size(); ++i)
    {
        TeamFlag* flag = static_cast<TeamFlag*>(flags[i]);

        if (m_flags[flag->getTeam()].isSet())
            log_error("Multiple flags defined for team %s", EConflictSide::toString(flag->getTeam()).c_str());

        m_flags[flag->getTeam()].setPtr(flag);
    }
}

void CtfMgr::onUpdate()
{
    __super::onUpdate();
}

TeamFlag* CtfMgr::getTeamFlag( EConflictSide::TYPE team ) const
{
    MK_ASSERT(team >= EConflictSide::_FIRST);
    MK_ASSERT(team < EConflictSide::_COUNT);
    MK_ASSERT(m_flags[team].isValid());
    
    return m_flags[team].fetchPtr();
}

const mkVec3& CtfMgr::getTeamBasePos( EConflictSide::TYPE team ) const
{
    MK_ASSERT(team >= EConflictSide::_FIRST);
    MK_ASSERT(team < EConflictSide::_COUNT);

    return getTeamFlag(team)->getInitialWorldPos();
}

int CtfMgr::getTeamSize( EConflictSide::TYPE team ) const
{
    MK_ASSERT(team >= EConflictSide::_FIRST);
    MK_ASSERT(team < EConflictSide::_COUNT);

    return m_teamSize[team];
}

bool CtfMgr::isTeamFlagAlreadyCarried( EConflictSide::TYPE team ) const
{
    MK_ASSERT(team >= EConflictSide::_FIRST);
    MK_ASSERT(team < EConflictSide::_COUNT);

    return getTeamFlag(team)->isCarried();
}

int CtfMgr::getNextTeamNumber( EConflictSide::TYPE team )
{
    return m_teamSize[team]++;
}

bool CtfMgr::isInCtfMode() const
{
    return (m_flags[EConflictSide::RedTeam].isValid());
}
