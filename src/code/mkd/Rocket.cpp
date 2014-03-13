/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "Rocket.h"
#include "Game.h"
#include "Character.h"
#include "Level.h"
#include "PhysicsObjectUserData.h"
#include "contrib/DebugDrawer.h"

IMPLEMENT_RTTI_NOSCRIPT(Rocket, GameObject);

START_RTTI_INIT(Rocket);
{
    FIELD_VEC3(m_worldPos);
    FIELD_VEC3(m_moveDir);
    FIELD_FLOAT(m_moveSpeed);
    FIELD_FLOAT(m_collisionRadius);
    FIELD_FLOAT(m_explosionDmg);
    FIELD_FLOAT(m_explosionRadius);
    FIELD_BOOL(m_exploded);
}
END_RTTI_INIT();

Rocket::Rocket()
    : m_worldPos(mkVec3::ZERO)
    , m_moveDir(mkVec3::UNIT_Z)
    , m_moveSpeed(1.f)
    , m_collisionRadius(1.f)
    , m_explosionDmg(1.f)
    , m_explosionRadius(1.f)
    , m_exploded(false)
    , m_explosionTime(-1.f)
{

}

void Rocket::onUpdate()
{
    __super::onUpdate();

    if (m_exploded)
    {
        static float EXPLOSION_TIME = 400.f;
        if ((getTimeMs() - m_explosionTime) > EXPLOSION_TIME)
            destroy();
    }
    else
    {
        const mkVec3 displacement = m_moveDir * m_moveSpeed * getTimeDelta();

        const mkVec3 old_pos = m_worldPos;
        const mkVec3 new_pos = old_pos + displacement;

        const btVector3 ray_start = ogre_to_bullet(old_pos);
        const btVector3 ray_end = ogre_to_bullet(new_pos);

        btCollisionWorld::AllHitsRayResultCallback ray_callback(ray_start, ray_end);
        g_game->getPhysicsWorld()->rayTest(ray_start, ray_end, ray_callback);

        bool should_explode = false;

        if (ray_callback.hasHit())
        {
            should_explode = true;

            // If only collision is with ignored object, don't explode
            if (m_ignoredObject.isValid() && ray_callback.m_collisionObjects.size() == 1)
            {
                btCollisionObject* object = ray_callback.m_collisionObjects[0];
                PhysicsObjUserData* user_data = (PhysicsObjUserData*)object->getUserPointer();

                if (user_data && (user_data->type & (PhysicsObjUserData::CharacterObject | PhysicsObjUserData::PlayerObject))
                    && user_data->character == m_ignoredObject.fetchPtr())
                {
                    should_explode = false;
                }
            }
        }

        if (should_explode)
            explode();
        else
            m_worldPos += displacement;
    }
}

static float EXPL_VIS_TIME = 150.f;
static float EXPL_POST_VIS_TIME = 350.f;

void Rocket::onRender()
{
    __super::onRender();

    if (m_exploded)
    {
        float time_since_explosion = getTimeMs() - m_explosionTime;

        if (time_since_explosion < EXPL_POST_VIS_TIME)
        {
            time_since_explosion += 100.f;

            float expl_vis_radius_mul = 1.f;

            if (time_since_explosion < EXPL_VIS_TIME)
                expl_vis_radius_mul = time_since_explosion / EXPL_VIS_TIME;

            DebugDrawer::getSingleton().drawSphere(m_worldPos, m_explosionRadius * expl_vis_radius_mul, Ogre::ColourValue::Red);
        }
    }
    else
    {
        DebugDrawer::getSingleton().drawSphere(m_worldPos, m_collisionRadius, Ogre::ColourValue::Red, true);
    }
}

void Rocket::setDirection( const mkVec3& new_dir )
{
    m_moveDir = new_dir.normalisedCopy();
}

void Rocket::setWorldPosition( const mkVec3& new_world_pos )
{
    m_worldPos = new_world_pos;
}

void Rocket::setIgnoredObject( GameObject* obj )
{
    m_ignoredObject = obj;
}

void Rocket::explode()
{
    m_exploded = true;
    m_explosionTime = getTimeMs();

    TGameObjectVec objects;
    getLevel()->findObjectsInRadius(objects, &GameObject::Type, m_worldPos, m_explosionRadius);

    for (size_t i = 0; i < objects.size(); ++i)
    {
        if (objects[i] != this)
        {
            SDamageInfo dmg_info(EDamageType::Fire, m_explosionDmg, m_moveDir, m_worldPos);

            objects[i]->takeDamage(dmg_info);
        }
    }
}
