/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "GameObject.h"
#include "Character.h"
#include "LevelObjectsMgr.h"

class ActorControllerFactory;
class ActorAI;
class CtfMgr;
class Player;

class Level : public rtti::IObjectProvider
{
public:
    Level();
    ~Level();

    void clear();
    bool load(const mkString& desc_file_name);

    void updateInput(float dt);
    void updateRendering(float dt);
    void updateLogic(float dt);

    Ogre::SceneManager* getOgreSceneMgr() const;
    btDynamicsWorld* getPhysicsWorld() const;
    ActorControllerFactory* getActorControllerFactory() const;

private:
    void loadGameObjects(const mkString& props_file_name);
    bool isKeyDown(OIS::KeyCode kc) const;

private:
    mutable int m_currFrameFindObjectsByIdQueriesNum;

    bool m_inDestruction;

    void deserializeGameObjects(json_value* val);
    void destroyAllChildGameObjects();

    void serializeGameObjects(const mkString& file_name);

    LevelObjectsMgr m_objectsMgr;

public:
    GameObject* createObject(const rtti::TypeInfo* type, bool call_init = true, const mkString& preset_name = "");

    template <typename T>
    T* createObject(bool call_init = true, const mkString& preset_name = "")
    {
        return static_cast<T*>(createObject(&(typename T::Type), call_init, preset_name));
    }

    void queueGameObjectDestruction(GameObject* obj);
    bool isInDestruction() const;

    rtti::IRTTIObject* findObjectById(rtti::TObjectId obj_id) const;

    // Finds first object with specified name. If exact_class is provided, only objects of this
    // class are considered (but not objects of subclasses!).
    // Note: this should never be used in code. Find-by-name is meant to be used in data only (scripts, quest system, etc)
    // TODO: subject to easy optimization if needed
    GameObject* findObjectByName(const mkString& name, const rtti::TypeInfo* exact_class = NULL) const;

    // Adds pointers to all existing objects of specified class (and, by default, its subclasses)
    // to out_objects. Iterates through all existing objects, so can be slow.
    // Should not be called in onCreate handler of deserialized objects, if needed, put it in onPostCreate
    void findObjectsOfType(TGameObjectVec& out_objects, const rtti::TypeInfo* type, bool allow_derived = true);

    void findObjectsInRadius(TGameObjectVec& out_objects, const rtti::TypeInfo* type, const mkVec3& search_origin, float radius, bool allow_derived = true);

    void subscribeToFrameEvent(EFrameEvent::TYPE frame_event, GameObject* object);
    void unsubscribeFromFrameEvent(EFrameEvent::TYPE frame_event, GameObject* object);

    // New gameplay stuff
public:
    CtfMgr* getCtfMgr() const;

    Player* getPlayer() const { return m_localPlayer; }
    void setLocalPlayer(Player* player);

private:
    CtfMgr* m_CtfMgr;
    Player* m_localPlayer;

    void initGameplayObjects();
    void releaseGameplayObjects();
};
