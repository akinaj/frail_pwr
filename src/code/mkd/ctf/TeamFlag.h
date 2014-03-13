#pragma once
#include "ModelObject.h"
#include "Character.h"
#include "ObjectRef.h"

class ActorAI;

class TeamFlag : public ModelObject
{
    DECLARE_RTTI(TeamFlag);

public:
    TeamFlag();

    virtual void onCreate();
    virtual void onUpdate();

    void setCarrier(ActorAI* new_carrier);
    void resetCarrier();

    const mkVec3& getInitialWorldPos() const;

    bool isCarried() const;

    EConflictSide::TYPE getTeam() const;

private:
    EConflictSide::TYPE m_team;
    mkVec3 m_initialWorldPos;

    ObjectRef<ActorAI> m_carrier;
};
