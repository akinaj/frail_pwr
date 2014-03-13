#pragma once
#include "GameObject.h"
#include "ActorAI.h"

class AISpawner : public GameObject
{
    DECLARE_RTTI(GameObject);

public:
    AISpawner();

    virtual void onUpdate();

private:
    bool m_alreadySpawned;

    mkVec3 m_spawnOrigin;
    float m_spawnRadius;
    int m_aiNum;

    mkString m_presetName;

    EConflictSide::TYPE m_conflictSide;

    void spawnAIs();
};
