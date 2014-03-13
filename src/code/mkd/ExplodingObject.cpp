/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "ExplodingObject.h"
#include "scripting/LuaSimpleBinding.h"
#include "Game.h"
#include "Level.h"
#include "contrib/DebugDrawer.h"

struct DynamicModelObjectMotionState : public btDefaultMotionState
{
    DynamicModelObjectMotionState(ModelObject* _obj, const btTransform& transform, const mkVec3& origin_offset)
        : btDefaultMotionState(transform), mo(_obj), origin_offset(origin_offset) { }

    ModelObject* mo;
    mkVec3 origin_offset;

    virtual void setWorldTransform(const btTransform& worldTrans)
    {
        mo->setOrientation(bullet_to_ogre(worldTrans.getRotation()));

        mkVec3 new_pos = bullet_to_ogre(worldTrans.getOrigin()) + mo->vecLocalToWorld(origin_offset);
        if (mo->getTypeInfo()->isDerivedOrExact(&ExplodingObject::Type))
            static_cast<ExplodingObject*>(mo)->setWorldPositionFromPhysics(new_pos);
        else
            mo->setWorldPosition(new_pos);
    }
};

IMPLEMENT_RTTI_NOSCRIPT(ExplodingObject, ModelObject);

START_RTTI_INIT(ExplodingObject);
{
    FIELD_VEC3(m_originOffset);
    FIELD_VEC3(m_size);
    FIELD_FLOAT(m_mass);
    FIELD_FLOAT(m_damageRadius);
    FIELD_FLOAT(m_damage);
    FIELD_ENUM_GEN(m_damageType, EDamageType);
}
END_RTTI_INIT();

ExplodingObject::ExplodingObject()
    : m_damageRadius(4.f)
    , m_originOffset(mkVec3::ZERO)
    , m_mass(1.f)
    , m_exploded(false)
{
    m_castsShadows = true;
}

void ExplodingObject::onTakeDamage( const SDamageInfo& dmg_info )
{
    __super::onTakeDamage(dmg_info);

    if (getScriptContext())
    {
        lua_simple::FunctionCallArgs args;
        args.add(dmg_info.type);
        args.add(dmg_info.dmg);
        getScriptContext()->callFunc("onTakeDamage", args, true);
    }
}

void ExplodingObject::onExploded()
{
    if (getScriptContext())
        getScriptContext()->tryCallFunc("onExploded");
}

void ExplodingObject::onCreate()
{
    __super::onCreate();

    btTransform transform;
    transform.setOrigin(ogre_to_bullet(getWorldPosition()));
    transform.setRotation(ogre_to_bullet(getOrientation()));

    m_physicsShape = new btCylinderShape(ogre_to_bullet(m_size));

    btVector3 local_inertia(0, 0, 0);
    m_physicsShape->calculateLocalInertia(m_mass, local_inertia);

    btDefaultMotionState* motion_state = new DynamicModelObjectMotionState(this, transform, m_originOffset);
    btRigidBody::btRigidBodyConstructionInfo rb_info(m_mass, motion_state, m_physicsShape, local_inertia);
    m_physicsBody = new btRigidBody(rb_info);

    g_game->getPhysicsWorld()->addRigidBody(m_physicsBody);

    m_physicsUserData.type = PhysicsObjUserData::DynGameObject;
    m_physicsUserData.game_obj = this;

    m_physicsBody->setUserPointer(&m_physicsUserData);
}

void ExplodingObject::onDestroy()
{
    __super::onDestroy();

    g_game->getPhysicsWorld()->removeRigidBody(m_physicsBody);

    delete m_physicsBody;
}

static float EXPL_VIS_TIME = 250.f;
static float EXPL_POST_VIS_TIME = 1000.f;

void ExplodingObject::onRender()
{
    __super::onRender();

    if (m_exploded)
    {
        float time_since_explosion = getTimeSinceExplosion();

        if (time_since_explosion < EXPL_POST_VIS_TIME)
        {
            float expl_vis_radius_mul = 1.f;

            if (time_since_explosion < EXPL_VIS_TIME)
                expl_vis_radius_mul = time_since_explosion / EXPL_VIS_TIME;

            DebugDrawer::getSingleton().drawSphere(getWorldPosition(), m_damageRadius * expl_vis_radius_mul, Ogre::ColourValue::Red);
        }
    }
}

void ExplodingObject::onUpdate()
{
    __super::onUpdate();

    if (m_exploded)
    {
        if (getTimeSinceExplosion() > EXPL_POST_VIS_TIME)
            destroy();
        else
            getVisSceneNode()->setVisible(false);
    }
}

float ExplodingObject::getTimeSinceExplosion() const
{
    return getTimeMs() - m_explosionTime;
}

EXPORT_NOARG_METHOD_SCRIPT(ExplodingObject, explode, void);

void ExplodingObject::onRegisterInScripts( lua_simple::LuaSimpleContext* ctx )
{
    __super::onRegisterInScripts(ctx);

    ctx->registerFunc("Explode", VOID_METHOD_SCRIPT(ExplodingObject, explode));
}

void ExplodingObject::explode()
{
    if (!m_exploded)
    {
        m_exploded = true;
        m_explosionTime = getTimeMs();

        onExploded();
    }
}

void ExplodingObject::setWorldPosition( const mkVec3& new_pos )
{
    __super::setWorldPosition(new_pos);

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(ogre_to_bullet(new_pos));
    m_physicsBody->setWorldTransform(transform);
}

void ExplodingObject::setWorldPositionFromPhysics( const mkVec3& new_pos )
{
    __super::setWorldPosition(new_pos);
}