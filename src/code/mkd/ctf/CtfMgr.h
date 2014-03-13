/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "GameObject.h"
#include "Character.h"
#include "ObjectRef.h"

class TeamFlag;

class CtfMgr : public GameObject
{
    DECLARE_RTTI(CtfMgr);

public:
    CtfMgr();

    virtual void onPostCreate();
    virtual void onUpdate();

    TeamFlag* getTeamFlag(EConflictSide::TYPE team) const;
    const mkVec3& getTeamBasePos(EConflictSide::TYPE team) const;
    int getTeamSize(EConflictSide::TYPE team) const;
    bool isTeamFlagAlreadyCarried(EConflictSide::TYPE team) const;

    int getNextTeamNumber(EConflictSide::TYPE team);

    bool isInCtfMode() const;

private:
    ObjectRef<TeamFlag> m_flags[EConflictSide::_COUNT];
    int m_teamSize[EConflictSide::_COUNT];
};
