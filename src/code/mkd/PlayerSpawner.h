#pragma once
#include "GameObject.h"

class Player;

class PlayerSpawner : public GameObject
{
    DECLARE_RTTI(PlayerSpawner);

public:
    PlayerSpawner();

    virtual void onPostCreate();

private:
    mkVec3 m_worldSpawnPos;
    bool m_alreadySpawned;

    Player* m_spawnedPlayer;

    mkString m_playerPrefab;
};
