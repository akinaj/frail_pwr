/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "PlayerSpawner.h"
#include "Player.h"
#include "Level.h"

#include "Game.h"

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
	Ogre::LogManager::getSingletonPtr()->logMessage("PlayerSpawner::onPostCreate()");
    // Player was spawned and serialized, ignore spawner
    if (m_alreadySpawned)
        return;

    Player* player = getLevel()->createObject<Player>(false, m_playerPreset);

	Ogre::LogManager::getSingletonPtr()->logMessage("PlayerSpawner::onPostCreate()");


	// TODO delete
	std::string x = boost::lexical_cast<std::string>(g_game->getCamera()->getPosition().x);
	std::string y = boost::lexical_cast<std::string>(g_game->getCamera()->getPosition().y);
	std::string z = boost::lexical_cast<std::string>(g_game->getCamera()->getPosition().z);

	Ogre::LogManager::getSingletonPtr()->logMessage("Camera position before player->setConflictSide : " + x + "," + y +"," + z);
	

    player->setConflictSide(EConflictSide::RedTeam);

	// TODO delete
	x = boost::lexical_cast<std::string>(g_game->getCamera()->getPosition().x);
	y = boost::lexical_cast<std::string>(g_game->getCamera()->getPosition().y);
	z = boost::lexical_cast<std::string>(g_game->getCamera()->getPosition().z);
	Ogre::LogManager::getSingletonPtr()->logMessage("Camera position after setConflictSide() : " + x + "," + y +"," + z);

    player->setWorldPosition(m_worldSpawnPos);

	// TODO delete
	x = boost::lexical_cast<std::string>(g_game->getCamera()->getPosition().x);
	y = boost::lexical_cast<std::string>(g_game->getCamera()->getPosition().y);
	z = boost::lexical_cast<std::string>(g_game->getCamera()->getPosition().z);
	Ogre::LogManager::getSingletonPtr()->logMessage("Camera position after setWorldPosition() : " + x + "," + y +"," + z);

    player->initStandaloneObject();

	// TODO delete
	x = boost::lexical_cast<std::string>(g_game->getCamera()->getPosition().x);
	y = boost::lexical_cast<std::string>(g_game->getCamera()->getPosition().y);
	z = boost::lexical_cast<std::string>(g_game->getCamera()->getPosition().z);
	Ogre::LogManager::getSingletonPtr()->logMessage("Camera position after initStandaloneObject() : " + x + "," + y +"," + z);

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
