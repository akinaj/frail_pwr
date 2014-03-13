#pragma once
#include "ModelObject.h"
#include "ObjectRef.h"

class Rocket : public GameObject
{
    DECLARE_RTTI(Rocket);

public:
    Rocket();

    virtual void onUpdate();
    virtual void onRender();

    void setDirection(const mkVec3& new_dir);
    void setWorldPosition(const mkVec3& new_world_pos);

    void setIgnoredObject(GameObject* obj);

protected:
    void explode();

private:
    mkVec3 m_worldPos;

    mkVec3 m_moveDir;
    float m_moveSpeed;

    float m_collisionRadius;

    float m_explosionDmg;
    float m_explosionRadius;

    bool m_exploded;
    float m_explosionTime;

    ObjectRef<GameObject> m_ignoredObject;
};
