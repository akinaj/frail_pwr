#include "pch.h"
#include "Level.h"
#include "LevelObjectsMgr.h"
#include "Game.h"
#include "utils.h"
#include "rtti/PresetMgr.h"

LevelObjectsMgr::LevelObjectsMgr()
    : m_owner(NULL)
    , m_frameEventSubscribersDirty(false)
    , m_deserializationMode(false)
    , m_needDefrag(false)
{

}

LevelObjectsMgr::~LevelObjectsMgr()
{
    MK_ASSERT(m_childObjects.empty());
    MK_ASSERT(m_objectsToDestroy.empty());
}

void LevelObjectsMgr::init(Level* owner)
{
    m_owner = owner;
}

GameObject* LevelObjectsMgr::createObject(const rtti::TypeInfo* type, bool call_init, const mkString& preset_name)
{
    MK_ASSERT_MSG(!m_owner->isInDestruction(),
        "New object of type '%s' with preset '%s' is added when level is being destroyed! Probably createObject call in onDestroy.",
        type->getClassName(), preset_name.c_str());

    MK_ASSERT(type->isDerivedOrExact(&GameObject::Type));

    GameObject* game_obj = static_cast<GameObject*>(type->rawCreateInstance());

    if (isInNormalMode())
        insertNewGameObject(game_obj);

    game_obj->_setLevel(m_owner);

    if (!preset_name.empty())
        g_game->getPresetMgr()->applyPreset(game_obj, preset_name);

    if (call_init)
        game_obj->initStandaloneObject();

    return game_obj;
}

void LevelObjectsMgr::queueGameObjectDestruction( GameObject* obj )
{
    MK_ASSERT(obj->getLevel() == m_owner);

    // When parent level is being destructed, there's no need to queue objects for destruction - all
    // child objects of this level will be destructed soon anyway
    if (obj->getLevel()->isInDestruction())
        return;

    m_objectsToDestroy.push_back(obj);
}

void LevelObjectsMgr::destroyAllChildGameObjects()
{
    // First, call destroy handlers
    for (TGameObjectVec::iterator iter = m_childObjects.begin();
        iter != m_childObjects.end(); ++iter)
    {
        GameObject* obj = *iter;

        if (obj)
            obj->onDestroy();
    }

    // And only after that we can destroy instances
    for (TGameObjectVec::iterator iter = m_childObjects.begin();
        iter != m_childObjects.end(); ++iter)
    {
        GameObject* obj = *iter;

        if (obj)
            obj->getTypeInfo()->rawDestroyInstance(obj);
    }

    m_childObjects.clear();
    m_freeIndices.clear();

    // Clear all event subscribers
    for (int i = 0; i < EFrameEvent::_COUNT; ++i)
        m_frameEventsSubscribers[i].clear();
}

void LevelObjectsMgr::destroyQueuedGameObjects()
{
    // Consider locking m_objectsToDestroy for time of this copying
    TGameObjectVec objects_to_destroy = m_objectsToDestroy;
    m_objectsToDestroy.clear();
    // and now we can release the lock

    // Some objects can be added to m_objectsToDestroy, when other objects' onDestroy is called
    // Their destruction will be handled in next frame
    for (TGameObjectVec::iterator iter = objects_to_destroy.begin();
        iter != objects_to_destroy.end(); ++iter)
    {
        GameObject* obj = *iter;
        obj->onDestroy();
    }

    for (TGameObjectVec::iterator iter = objects_to_destroy.begin();
        iter != objects_to_destroy.end(); ++iter)
    {
        GameObject* obj = *iter;

        unsubscribeFromAllFrameEvents(obj);

        size_t removed_idx = replace_first_val_in_vec(m_childObjects, obj, (GameObject*)NULL);
        MK_ASSERT(removed_idx != m_childObjects.size());
        m_freeIndices.push_back(removed_idx);
        
        obj->getTypeInfo()->rawDestroyInstance(obj);
    }
}

rtti::IRTTIObject* LevelObjectsMgr::findObjectById( rtti::TObjectId obj_id ) const
{
    MK_ASSERT_MSG(obj_id < m_childObjects.size(), "Max ObjectID for this level is %d, tried to found one with id %d",
        m_childObjects.size() - 1, (int)obj_id);

    // TODO additional check so we don't return new object with some old ID (TObjectId should be compound of index and some magic counter value)
    if (obj_id <= m_childObjects.size())
    {
        GameObject* result = m_childObjects[obj_id];
        
        if (result)
            return result;
    }

    //log_error("Could not find GameObject with ObjectId=%x", (uint32)obj_id);
    return NULL;
}

GameObject* LevelObjectsMgr::findObjectByName( const mkString& name, const rtti::TypeInfo* exact_class ) const
{
    for (size_t i = 0; i < m_childObjects.size(); ++i)
    {
        GameObject* obj_ptr = m_childObjects[i];
        if (obj_ptr && (!exact_class || obj_ptr->getTypeInfo() == exact_class))
        {
            if (obj_ptr->getName() == name)
                return obj_ptr;
        }
    }

    return NULL;
}

void LevelObjectsMgr::findObjectsOfType( TGameObjectVec& out_objects, const rtti::TypeInfo* type, bool allow_derived )
{
    // TODO consider caching object lists by type (this method stands out when profiling when there are many game objects on level)

    for (size_t i = 0; i < m_childObjects.size(); ++i)
    {
        GameObject* game_obj = m_childObjects[i];

        if (game_obj)
        {
            const rtti::TypeInfo* obj_type = game_obj->getTypeInfo();
            if (obj_type == type || (allow_derived && obj_type->isDerivedFrom(type)))
                out_objects.push_back(game_obj);
        }
    }
}

void LevelObjectsMgr::callOnPostCreate()
{
    for (size_t i = 0; i < m_childObjects.size(); ++i)
    {
        GameObject* game_obj = m_childObjects[i];

        if (game_obj)
            game_obj->onPostCreate();
    }
}

void LevelObjectsMgr::subscribeToFrameEvent( EFrameEvent::TYPE frame_event, GameObject* object )
{
    MK_ASSERT(object);
    MK_ASSERT(object->getLevel() == m_owner);

    SubscribeData data;
    data.subscriber_ptr = object;
    data.frame_event = frame_event;

    m_subscribesToAdd.push_back(data);
    m_frameEventSubscribersDirty = true;
}

void LevelObjectsMgr::unsubscribeFromFrameEvent( EFrameEvent::TYPE frame_event, GameObject* object )
{
    MK_ASSERT(object);
    MK_ASSERT(object->getLevel() == m_owner);

    SubscribeData data;
    data.subscriber_ptr = object;
    data.frame_event = frame_event;

    m_subscribesToRemove.push_back(data);
    m_frameEventSubscribersDirty = true;
}

void LevelObjectsMgr::callFrameEventHandlers( EFrameEvent::TYPE frame_event )
{
    updateSubscribes();

    EFrameEvent::HandlerType handler_method = EFrameEvent::getHandlerMethodPtr(frame_event);

    const TGameObjectVec::iterator end_it = m_frameEventsSubscribers[frame_event].end();
    TGameObjectVec::iterator it = m_frameEventsSubscribers[frame_event].begin();

    for (; it != end_it; ++it)
    {
        GameObject* game_obj = *it;
        (game_obj->*handler_method)();
    }
}

void LevelObjectsMgr::unsubscribeFromAllFrameEvents( GameObject* object )
{
    // Please note it's only called for individually destroyed objects,
    // not when whole level is being destroyed

    for (int i = 0; i < EFrameEvent::_COUNT; ++i)
    {
        EFrameEvent::TYPE frame_event = (EFrameEvent::TYPE)i;
        unsubscribeFromFrameEvent(frame_event, object);
    }
}

void LevelObjectsMgr::updateSubscribes()
{
    if (m_frameEventSubscribersDirty)
    {
        for (TSubscribesVec::const_iterator it = m_subscribesToAdd.begin();
            it != m_subscribesToAdd.end(); ++it)
        {
            const EFrameEvent::TYPE frame_event = it->frame_event;
            GameObject* obj_ptr = it->subscriber_ptr;

            m_frameEventsSubscribers[frame_event].push_back(obj_ptr);
        }

        for (TSubscribesVec::const_iterator it = m_subscribesToRemove.begin();
            it != m_subscribesToRemove.end(); ++it)
        {
            const EFrameEvent::TYPE frame_event = it->frame_event;
            GameObject* obj_ptr = it->subscriber_ptr;

            fast_remove_val_from_vec(m_frameEventsSubscribers[frame_event], obj_ptr);
        }

        m_subscribesToAdd.clear();
        m_subscribesToRemove.clear();

        m_frameEventSubscribersDirty = false;
    }
}

void LevelObjectsMgr::insertNewGameObject(GameObject* game_obj)
{
    MK_ASSERT(isInNormalMode());

    rtti::TObjectId new_id = rtti::IRTTIObject::INVALID_OBJECT_ID;

    if (m_freeIndices.empty())
    {
        m_childObjects.push_back(game_obj);
        new_id = m_childObjects.size() - 1;
    }
    else
    {
        size_t idx = m_freeIndices.back();
        m_freeIndices.pop_back();
        m_childObjects[idx] = game_obj;

        new_id = idx;
    }

    game_obj->_setObjectId(new_id);
}

void LevelObjectsMgr::insertNewGameObjectPreservingId( GameObject* game_obj )
{
    MK_ASSERT(!isInNormalMode());
    MK_ASSERT(game_obj->getObjectId() != rtti::IRTTIObject::INVALID_OBJECT_ID);

    const rtti::TObjectId obj_id = game_obj->getObjectId();
    if (obj_id >= m_childObjects.size())
    {
        const size_t old_size = m_childObjects.size();
        const size_t new_size = obj_id + 1; // TODO: consider resizing more?

        m_childObjects.resize(new_size);
        fill(m_childObjects.begin() + old_size, m_childObjects.end(), (GameObject*)NULL);

        m_childObjects[obj_id] = game_obj;
        m_needDefrag = true;
    }
    else
    {
        MK_ASSERT(m_childObjects[obj_id] == NULL);
        m_childObjects[obj_id] = game_obj;
    }

}

bool LevelObjectsMgr::isInNormalMode() const
{
    return !m_deserializationMode;
}

void LevelObjectsMgr::enterDeserializationMode()
{
    MK_ASSERT(isInNormalMode());

    m_deserializationMode = true;
}

void LevelObjectsMgr::leaveDeserializationMode()
{
    MK_ASSERT(!isInNormalMode());

    defragmentChildObjectList();
    m_deserializationMode = false;
}

void LevelObjectsMgr::defragmentChildObjectList()
{
    // TODO: consider marking somehow serialized object ids (like always zeroing/setting last bit?)
    // so we can detect later whether someone's trying to access object with ID retrieved before defragmenting

    // Free index list has no meaning after defragmentation
    m_freeIndices.clear();

    // No point in defragmenting empty list
    if (m_childObjects.empty())
        return;

    if (!m_needDefrag)
        return;

    GameObject* null_ptr = (GameObject*)NULL;
    const TGameObjectVec::iterator begin = m_childObjects.begin();
    const TGameObjectVec::iterator end = m_childObjects.end();

    // Index we're moving through objects list. When we run into an empty slot,
    // object from last_object_idx is moved to curr_idx, its ObjectID is updated,
    // and last_object_idx is moved to last non-empty slot in list.
    // List is defragmented when last_object_idx equals curr_idx
    size_t curr_idx = std::find(begin, end, null_ptr) - begin;
    size_t last_object_idx = last_not_equal_to_idx(m_childObjects, null_ptr);

    while (curr_idx < last_object_idx)
    {
        rtti::TObjectId new_id = curr_idx;
        GameObject* game_obj = m_childObjects[last_object_idx];
        
        m_childObjects[last_object_idx] = NULL;
        m_childObjects[curr_idx] = game_obj;
        game_obj->_setObjectId(new_id);

        last_object_idx = last_not_equal_to_idx(begin, begin + last_object_idx, null_ptr);
        curr_idx = std::find(begin + curr_idx, end, null_ptr) - begin;
    }

    if (last_object_idx < m_childObjects.size() - 1)
    {
        m_childObjects.resize(last_object_idx + 1);
    }

    m_needDefrag = false;
}

void LevelObjectsMgr::serializeGameObjects( IDataWriter* writer )
{
    for (size_t i = 0; i < m_childObjects.size(); ++i)
    {
        GameObject* game_obj = m_childObjects[i];

        if (game_obj && !game_obj->m_isQueuedForDestruction)
        {
            game_obj->getTypeInfo()->write(*game_obj, writer);
        }
    }
}
