#pragma once
#include "ModelObject.h"
#include "PhysicsObjectUserData.h"

class ExplodingObject : public ModelObject
{
    DECLARE_RTTI(ExplodingObject);

public:
    ExplodingObject();

    virtual void onCreate();
    virtual void onDestroy();

    virtual void onUpdate();
    virtual void onRender();

    virtual void onTakeDamage(const SDamageInfo& dmg_info);
    virtual void onExploded();

    float getTimeSinceExplosion() const;

    void explode();

    void setWorldPosition(const mkVec3& new_pos);
    void setWorldPositionFromPhysics(const mkVec3& new_pos);

private:
    float m_damage;
    bool m_exploded;
    float m_mass;
    mkVec3 m_size;
    mkVec3 m_originOffset;
    float m_damageRadius;
    EDamageType::TYPE m_damageType;

    float m_explosionTime;

    btConvexShape* m_physicsShape;
    btRigidBody* m_physicsBody;
    PhysicsObjUserData m_physicsUserData;

    virtual void onRegisterInScripts(lua_simple::LuaSimpleContext* ctx);
};
