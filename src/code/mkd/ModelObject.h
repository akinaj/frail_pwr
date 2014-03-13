#pragma once
#include "GameObject.h"

struct Transform
{
    mkVec3 position;
    mkQuat orientation;

    Transform()
        : position(mkVec3::ZERO)
        , orientation(mkQuat::IDENTITY)
    {

    }
};

struct Prefab;

class ModelObject : public GameObject
{
    DECLARE_RTTI(ModelObject);

public:
    ModelObject();

    virtual void onCreate();
    virtual void onDestroy();

    virtual void onUpdate();

    // Changes initial prefab name. Note it will not be effective after
    // onCreate is called, so this should be called only on uninitialized objects
    void setPrefabName(const mkString& prefab_name);

    const Transform& getWorldTransform() const;
    const mkVec3& getWorldPosition() const;
    const mkQuat& getOrientation() const;

    void setWorldTransform(const Transform& new_transform);
    virtual void setWorldPosition(const mkVec3& new_pos);
    void setOrientation(const mkQuat& new_orientation);

    // Note: not implemented yet
    mkVec3 pointLocalToWorld(const mkVec3& local_pt) const;
    mkVec3 vecLocalToWorld(const mkVec3& local_vec) const;
    mkVec3 pointWorldToLocal(const mkVec3& world_pt) const;
    mkVec3 vecWorldToLocal(const mkVec3& world_vec) const;

    Ogre::SceneNode* getVisSceneNode() const;
    Ogre::Entity* getVisMesh() const;

protected:
    Prefab* getPrefab() const;

private:
    void createVis();
    void destroyVis();

    void updateVisSceneNodeTransform();

    // Serialized fields
private:
    Transform m_worldTransform;
    mkString m_prefabName;
    mkString m_meshName; // used only when m_prefabName is not set

protected:
    bool m_castsShadows;

    // Runtime fields
private:
    Ogre::SceneNode* m_visSceneNode;
    Ogre::Entity* m_visMesh;
};
