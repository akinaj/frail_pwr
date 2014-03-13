/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "TeamFlag.h"
#include "ActorAI.h"

IMPLEMENT_RTTI_NOSCRIPT(TeamFlag, ModelObject);

START_RTTI_INIT(TeamFlag);
{
    FIELD_ENUM_GEN(m_team, EConflictSide);
    FIELD_VEC3(m_initialWorldPos);
}
END_RTTI_INIT();

TeamFlag::TeamFlag()
    : m_team(EConflictSide::Unknown)
{

}

void TeamFlag::onCreate()
{
    __super::onCreate();

    setWorldPosition(m_initialWorldPos);
}

void TeamFlag::setCarrier( ActorAI* new_carrier )
{
    m_carrier.setPtr(new_carrier);
}

void TeamFlag::resetCarrier()
{
    setCarrier(NULL);
}

void TeamFlag::onUpdate()
{
    __super::onUpdate();

    ActorAI* carrier = m_carrier.fetchPtr();
    if (carrier)
    {
        const mkVec3 carrier_pos = carrier->getWorldPosition();
        const mkVec3 my_new_pos = carrier_pos + mkVec3(0, 2, 0);

        setWorldPosition(my_new_pos);
    }
}

const mkVec3& TeamFlag::getInitialWorldPos() const
{
    return m_initialWorldPos;
}

bool TeamFlag::isCarried() const
{
    return m_carrier.isValid();
}

EConflictSide::TYPE TeamFlag::getTeam() const
{
    return m_team;
}
