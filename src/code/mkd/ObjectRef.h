#pragma once
#include "TypeInfo.h"
#include "GameObject.h"

// ObjectRef is kind of weak reference that should be used to point to other game objects, especially to those created dynamically (like AIs, players etc)
// ObjectRef assures to never return invalid pointer. It is achieved by storing object ID, and fetching pointer from object provider
// each time a pointer is needed. It is safe, but slow - it would be possible to use typical weak pointer where stored pointer value would
// just be zeroed on object's destruction, but that would cause problems in multithreaded environment (objects destruction should be done in point
// where no one uses weak pointers).

class IObjectProvider;
class GameObject;

class ObjectRefBase
{
public:
    ObjectRefBase(GameObject* obj = NULL)
    {
        _setPtr(obj);
    }

    // Fetches a pointer when validating, slow!
    bool isValid() const
    {
        return _isValid(_fetchPtr());
    }

    // Just checks for clear values, no pointer fetching. Should never be used to check reference validity
    bool isSet() const
    {
        return m_objId != rtti::IRTTIObject::INVALID_OBJECT_ID && m_provider != NULL;
    }

    // Finds a pointer to object and returns it. Slow!
    GameObject* fetchBasePtr() const
    {
        GameObject* ptr = _fetchPtr();
        MK_ASSERT(!isSet() || _isValid(ptr));

        return ptr;
    }

    void setBasePtr(GameObject* ptr)
    {
        _setPtr(ptr);
    }

protected:
    // Finds a pointer to object and returns it. Slow!
    GameObject* _fetchPtr() const
    {
        if (m_provider == NULL)
            return NULL;

        return static_cast<GameObject*>(m_provider->findObjectById(m_objId));
    }

    bool _isValid(GameObject* fetched_obj) const
    {
        if (fetched_obj == NULL)
            return false;

        if (fetched_obj->getObjectId() != m_objId)
            return false;

        return true;
    }

    void _setPtr(GameObject* obj)
    {
        if (obj == NULL)
        {
            m_objId = rtti::IRTTIObject::INVALID_OBJECT_ID;
            m_provider = NULL;
        }
        else
        {
            m_objId = obj->getObjectId();
            m_provider = obj->getOwningObjectProvider();
        }
    }

private:
    rtti::TObjectId m_objId;

    // TODO: consider keeping pointer to LevelObjectsMgr instead,
    // so fetching ptr wouldn't require virtual function call
    rtti::IObjectProvider* m_provider;
};

template <typename T>
class ObjectRef : public ObjectRefBase
{
public:
    ObjectRef()
        : ObjectRefBase(NULL)
    {

    }

    ObjectRef(T* ptr)
        : ObjectRefBase(ptr)
    {

    }

    // Finds a pointer to object and returns it. Slow!
    T* fetchPtr() const
    {
        return static_cast<T*>(fetchBasePtr());
    }

    void setPtr(T* ptr)
    {
        _setPtr(ptr);
    }
};
