/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "PlayerSpawner.h"
#include "Player.h"
#include "Level.h"

IMPLEMENT_RTTI_NOSCRIPT(PlayerSpawner, GameObject);

START_RTTI_INIT(PlayerSpawner);
{
    FIELD_VEC3(m_worldSpawnPos);
    FIELD_BOOL(m_alreadySpawned);
    FIELD_PTR(m_spawnedPlayer);
    FIELD_STRING(m_playerPreset);
}
END_RTTI_INIT();

PlayerSpawner::PlayerSpawner()
{
    m_worldSpawnPos = mkVec3::ZERO;
    m_alreadySpawned = false;

    m_playerPreset = "Default";
}

void PlayerSpawner::onPostCreate()
{
    __super::onPostCreate();

    // Player was spawned and serialized, ignore spawner
    if (m_alreadySpawned)
        return;

    Player* player = getLevel()->createObject<Player>(false, m_playerPreset);

    player->setConflictSide(EConflictSide::RedTeam);
    player->setWorldPosition(m_worldSpawnPos);
    player->initStandaloneObject();

    //player->setIsVisibleInSightQueries(false);

    // Turn off debug visuals of player kinematic character
    btGhostObject* player_ghost_obj = player->getKinematicController()->getGhostObject();
    //player_ghost_obj->setCollisionFlags(player_ghost_obj->getCollisionFlags() | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);

    // temp!
    PhysicsObjUserData* pl_data = new PhysicsObjUserData;
    pl_data->character = player;
    pl_data->type = PhysicsObjUserData::CharacterObject | PhysicsObjUserData::PlayerObject;
    player_ghost_obj->setUserPointer(pl_data);

    m_alreadySpawned = true;
    m_spawnedPlayer = player;
}
