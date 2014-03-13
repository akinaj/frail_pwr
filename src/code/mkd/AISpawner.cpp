/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "AISpawner.h"
#include "Level.h"
#include "ActorAI.h"
#include "ctf/CtfMgr.h"

IMPLEMENT_RTTI_NOSCRIPT(AISpawner, GameObject);

START_RTTI_INIT(AISpawner);
{
    FIELD_VEC3(m_spawnOrigin);
    FIELD_FLOAT(m_spawnRadius);
    FIELD_INT32(m_aiNum);
    FIELD_STRING(m_presetName);
    FIELD_INT32(m_conflictSide);
    FIELD_BOOL(m_alreadySpawned);
    FIELD_STRING(m_overrideScriptName);
}
END_RTTI_INIT();

AISpawner::AISpawner()
    : m_alreadySpawned(false)
    , m_spawnRadius(0.f)
    , m_aiNum(1)
    , m_presetName("")
    , m_conflictSide(EConflictSide::Unknown)
{

}

void AISpawner::onUpdate()
{
    __super::onUpdate();

    if (m_alreadySpawned)
        return;

    spawnAIs();
}

void AISpawner::spawnAIs()
{
    for (int i = 0; i < m_aiNum; ++i)
    {
        mkVec3 pos = m_spawnOrigin + getRandomHorizontalDir() * m_spawnRadius * randFloat(0.33f, 1.f);

        ActorAI* ai = getLevel()->createObject<ActorAI>(false, m_presetName);

        if (!m_overrideScriptName.empty())
            ai->setScriptName(m_overrideScriptName);

        ai->setWorldPosition(pos);
        ai->initStandaloneObject();

        ai->setSpeed(1.f);
        
        if (m_conflictSide != EConflictSide::Unknown)
            ai->setConflictSide(m_conflictSide);

        // TODO generic ai spawner should know nothing about CTF, create special CTF spawner
        ai->setTeamNumber(getLevel()->getCtfMgr()->getNextTeamNumber(ai->getConflictSide()));
    }

    m_alreadySpawned = true;
}