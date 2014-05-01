/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "utils.h"
#include "Level.h"
#include "Game.h"
#include "ActorAI.h"
#include "ActorControllerFactory.h"
#include "Filesystem.h"
#include "serialization/SerializationJSON.h"
#include "rtti/TypeManager.h"
#include "rtti/PresetMgr.h"
#include "ctf/CtfMgr.h"
#include "Player.h"
#include "MeshObject.h"
#include "Contrib/OgreRecast/include/OgreRecast.h"
#include "Contrib/OgreRecast/include/OgreDetourTileCache.h"

Level::Level()
    : m_localPlayer(NULL)
    , m_currFrameFindObjectsByIdQueriesNum(0)
    , m_inDestruction(false)
	, m_meshName("2nomansland.mesh")
{
    m_objectsMgr.init(this);
	//m_playerStartPos =  mkVec3(114.3516f, 10.8807f, 98.7614f);
}

Level::~Level()
{
}

bool Level::load( const mkString& desc_file_name)
{
    loadGameObjects(desc_file_name);
    initGameplayObjects();

    if (getPlayer() == NULL)
    {
        log_error("No player was created. Please place PlayerSpawner in your level file");
        return false;
    }

	initLevelNavMesh();

    return true;
}

void Level::initLevelNavMesh()
{
	Ogre::LogManager::getSingletonPtr()->logMessage("initLevelNavMesh() called");

	Ogre::LogManager::getSingletonPtr()->logMessage("createEntity() called");
	Ogre::Entity* level_mesh = g_game->getOgreSceneMgr()->createEntity("LevelMesh", m_meshName);

	Ogre::LogManager::getSingletonPtr()->logMessage("createChildSceneNode() called");
	Ogre::SceneNode* level_node = g_game->getOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();

	Ogre::LogManager::getSingletonPtr()->logMessage("attachObject() called");
	level_node->attachObject(level_mesh);

	level_mesh->setCastShadows(false);

	this->ogreRecast = new OgreRecast(getOgreSceneMgr());
	std::vector<Ogre::Entity* > mNavmeshEnts;
	mNavmeshEnts.push_back(getOgreSceneMgr()->getEntity("LevelMesh"));
	
	
	ogreRecast->NavMeshBuild(mNavmeshEnts);
	ogreRecast->drawNavMesh();
	
	
	OgreDetourTileCache *detourTileCache = new OgreDetourTileCache(ogreRecast);
	detourTileCache->TileCacheBuild(mNavmeshEnts);
	detourTileCache->drawNavMesh();
	
	Ogre::LogManager::getSingletonPtr()->logMessage("initLevelNavMesh() done");
}

void Level::updateRendering( float dt )
{
    char buff[512] = {0};
    sprintf(buff, "Find queries: %d", m_currFrameFindObjectsByIdQueriesNum);
    Ogre::OverlayManager::getSingleton().getOverlayElement("Game/GunOrientation")->setCaption(buff);

    m_objectsMgr.callFrameEventHandlers(EFrameEvent::Render);
}

void Level::updateLogic( float dt )
{
    m_currFrameFindObjectsByIdQueriesNum = 0;

    m_objectsMgr.callFrameEventHandlers(EFrameEvent::LogicUpdate);

    m_objectsMgr.destroyQueuedGameObjects();
}

void Level::clear()
{
    releaseGameplayObjects();
    destroyAllChildGameObjects();

    m_localPlayer = NULL;
}

void Level::updateInput( float dt )
{
    m_localPlayer->resetDirection();
    m_localPlayer->setSpeed(0.0f);

    if (isKeyDown(OIS::KC_SPACE))
        m_localPlayer->jump();

    OIS::KeyCode codes_for_dbg_ai[] = 
    {
        OIS::KC_DOWN,
        OIS::KC_UP,
        OIS::KC_LEFT,
        OIS::KC_RIGHT,
        OIS::KC_SPACE,
        OIS::KC_LSHIFT,
        OIS::KC_RSHIFT,
        OIS::KC_TAB,
        OIS::KC_LCONTROL,
        OIS::KC_RCONTROL
    };

    if (isKeyDown(OIS::KC_S) && isKeyDown(OIS::KC_LCONTROL))
        serializeGameObjects("data/levels/serialized.json");

    // temp plz
    for (int i = 0; i < _countof(codes_for_dbg_ai); ++i)
    {
        if (isKeyDown(codes_for_dbg_ai[i]))
        {
            TGameObjectVec objects;
            findObjectsOfType(objects, &ActorAI::Type);

            for (size_t ai_idx = 0; ai_idx < objects.size(); ++ai_idx)
            {
                static_cast<ActorAI*>(objects[ai_idx])->onDbgKeyDown(codes_for_dbg_ai[i]);
            }
        }
    }
}

bool Level::isKeyDown( OIS::KeyCode kc ) const
{
    return g_game->isKeyDown(kc);
}

Ogre::SceneManager* Level::getOgreSceneMgr() const
{
    return g_game->getOgreSceneMgr();
}

btDynamicsWorld* Level::getPhysicsWorld() const
{
    return g_game->getPhysicsWorld();
}

ActorControllerFactory* Level::getActorControllerFactory() const
{
    return g_game->getActorControllerFactory();
}

void Level::loadGameObjects( const mkString& objects_file_name )
{
    uint8* out_buf_ptr = NULL;
    uint32 out_buf_size = 0;
    if (loadFile(objects_file_name.c_str(), &out_buf_ptr, &out_buf_size))
    {
        char* error_pos = 0;
        char* error_desc = 0;
        int error_line = 0;

        block_allocator allocator(1 << 10);
        json_value* root = json_parse((char*)out_buf_ptr, &error_pos, &error_desc, &error_line, &allocator);
        if (!root)
        {
            log_error("Error at line %d: %s\n%s\n\n", error_line, error_desc, error_pos);
            return;
        }

        deserializeGameObjects(root);

        delete[] out_buf_ptr;
    }
}

GameObject* Level::createObject( const rtti::TypeInfo* type, bool call_init, const mkString& preset_name )
{
    return m_objectsMgr.createObject(type, call_init, preset_name);
}

void Level::destroyAllChildGameObjects()
{
    m_inDestruction = true;

    m_objectsMgr.destroyAllChildGameObjects();

    m_inDestruction = false;
}

void Level::queueGameObjectDestruction(GameObject* obj)
{
    m_objectsMgr.queueGameObjectDestruction(obj);
}

bool Level::isInDestruction() const
{
    return m_inDestruction;
}

void Level::serializeGameObjects( const mkString& file_name )
{
    JSONDataWriterString writer;

    writer.writeInt32("SerializedWithIds", 1);
    m_objectsMgr.serializeGameObjects(&writer);

    mkString result;
    writer.extractString(result);

    writeTextFile(file_name.c_str(), result.c_str(), result.size());
}

void Level::deserializeGameObjects(json_value* val)
{
    rtti::PointerFixups fixups;

    JSONDataReader reader;
    reader.init(val, true);

    int32 dummy;
    const bool deserializing_with_ids = reader.readInt32("SerializedWithIds", dummy);

    if (deserializing_with_ids)
        m_objectsMgr.enterDeserializationMode();

    const char* class_name = NULL;
    while ((class_name = reader.getNextBlockName())) // redundant parentheses to silence compiler warning
    {
        reader.readStartBlock(class_name);

        JSONDataReader object_reader;
        object_reader.init(reader.getCurrentBlockValue(), false);

        GameObject* created_object = NULL;

        const rtti::TypeInfo* type_info = rtti::TypeManager::getInstance().getTypeInfoByName(class_name);

        if (type_info)
        {
            mkString preset_name;
            reader.readString("Preset", preset_name);

            created_object = createObject(type_info, false, preset_name);

            type_info->read(*created_object, &object_reader, &fixups);

            if (deserializing_with_ids)
                m_objectsMgr.insertNewGameObjectPreservingId(created_object);
            
            // Consider calling onCreates after pointer fixups and switching objects manager to normal mode
            created_object->onCreate();
        }

        reader.readEndBlock(class_name);
    }

    fixups.fixupPointers(this);

    if (deserializing_with_ids)
        m_objectsMgr.leaveDeserializationMode();

    m_objectsMgr.callOnPostCreate();
}

rtti::IRTTIObject* Level::findObjectById(rtti::TObjectId obj_id) const
{
    ++m_currFrameFindObjectsByIdQueriesNum;

    return m_objectsMgr.findObjectById(obj_id);
}

void Level::findObjectsOfType( TGameObjectVec& out_objects, const rtti::TypeInfo* type, bool allow_derived /*= true*/ )
{
    return m_objectsMgr.findObjectsOfType(out_objects, type, allow_derived);
}

CtfMgr* Level::getCtfMgr() const
{
    return m_CtfMgr;
}

void Level::initGameplayObjects()
{
    // Create gameplay objects not deserialized from level

    m_CtfMgr = createObject<CtfMgr>();
}

void Level::releaseGameplayObjects()
{
    // All GameObjects are deleted when level is destroyed, put here any other deininit code
    // concerning objects created in initGameplayObjects
}

void Level::setLocalPlayer( Player* player )
{
    m_localPlayer = player;

	if(!g_game->isFreelookCamera()) 
	{
		    g_game->getCamera()->setTarget(m_localPlayer);
	}
}

GameObject* Level::findObjectByName( const mkString& name, const rtti::TypeInfo* exact_class /*= NULL*/ ) const
{
    return m_objectsMgr.findObjectByName(name, exact_class);
}

void Level::subscribeToFrameEvent( EFrameEvent::TYPE frame_event, GameObject* object )
{
    m_objectsMgr.subscribeToFrameEvent(frame_event, object);
}

void Level::unsubscribeFromFrameEvent( EFrameEvent::TYPE frame_event, GameObject* object )
{
    m_objectsMgr.unsubscribeFromFrameEvent(frame_event, object);
}

void Level::findObjectsInRadius( TGameObjectVec& out_objects, const rtti::TypeInfo* type, const mkVec3& search_origin, float radius, bool allow_derived /*= true*/ )
{
    MK_ASSERT(out_objects.empty());

    m_objectsMgr.findObjectsOfType(out_objects, type, allow_derived);

    for (size_t i = 0; i < out_objects.size(); ++i)
    {
        bool accept = false;

        GameObject* go = out_objects[i];
        if (go->getTypeInfo()->isDerivedOrExact(&ModelObject::Type))
        {
            ModelObject* mo = static_cast<ModelObject*>(go);
            float dist_sq = (mo->getWorldPosition() - search_origin).length();

            if (dist_sq <= radius)
                accept = true;
        }

        if (!accept)
            out_objects[i] = NULL;
    }

    fast_remove_val_from_vec(out_objects, (GameObject*)NULL);
}
