/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
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

    mkString m_playerPreset;
};
