#pragma once

class Level;

namespace rtti
{
    class IRTTIObject;
    class TypeInfo;
}

// Handles creation, access, finding and destroying of Game Objects on Level
class LevelObjectsMgr
{
public:
    LevelObjectsMgr();
    ~LevelObjectsMgr();

    void init(Level* owner);

    void enterDeserializationMode();
    void leaveDeserializationMode();

    // Calls onPostCreate for all objects (used in deserialization)
    void callOnPostCreate();

    void updateSubscribes();
    void subscribeToFrameEvent(EFrameEvent::TYPE frame_event, GameObject* object);
    void unsubscribeFromFrameEvent(EFrameEvent::TYPE frame_event, GameObject* object);
    void unsubscribeFromAllFrameEvents(GameObject* object);

    void callFrameEventHandlers(EFrameEvent::TYPE frame_event);

    GameObject* createObject(const rtti::TypeInfo* type, bool call_init, const mkString& preset_name);
    void queueGameObjectDestruction(GameObject* obj);

    void destroyAllChildGameObjects();
    void destroyQueuedGameObjects();

    // See Level::findObjectByName, Level::findObjectsOfType and Level::findObjectById for more info
    rtti::IRTTIObject* findObjectById   (rtti::TObjectId obj_id) const;
    GameObject*        findObjectByName (const mkString& name, const rtti::TypeInfo* exact_class) const;
    void               findObjectsOfType(TGameObjectVec& out_objects, const rtti::TypeInfo* type, bool allow_derived);

    // Inserts new game object, but does not change its ID. If needed, child objects list is resized
    // Should be used only for deserialization. Inserted object's IDs must be unique!
    void insertNewGameObjectPreservingId(GameObject* game_obj);

    void serializeGameObjects(IDataWriter* writer);

private:
    // Inserts given GameObject to child list, changing object's ID so it matches index it is inserted on
    void insertNewGameObject(GameObject* game_obj);

    void defragmentChildObjectList();
    bool isInNormalMode() const;

private:
    Level* m_owner;

    TGameObjectVec m_childObjects;
    TGameObjectVec m_objectsToDestroy;

    typedef std::vector<size_t> TIndexVec;
    TIndexVec m_freeIndices;

    bool m_needDefrag;

    // In deserialization mode, objects manager does not automatically add
    // newly created Game Objects to child objects list. This is done so 
    // such object can then be deserialized and inserted to list with ID
    // it was serialized with. After deserialization and pointer fixup, Level
    // should switch LevelObjectsMgr to normal mode. When switched to normal,
    // child objects list is defragmented, which results in changing IDs!
    bool m_deserializationMode;

    struct SubscribeData
    {
        GameObject* subscriber_ptr;
        EFrameEvent::TYPE frame_event;
    };

    typedef std::vector<SubscribeData> TSubscribesVec;

    TGameObjectVec m_frameEventsSubscribers[EFrameEvent::_COUNT];
    TSubscribesVec m_subscribesToAdd;
    TSubscribesVec m_subscribesToRemove;
    bool m_frameEventSubscribersDirty;
};
