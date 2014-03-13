/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "DynamicLight.h"
#include "Level.h"
#include "Player.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IMPLEMENT_RTTI_NOSCRIPT(DynamicLight, GameObject);

START_RTTI_INIT(DynamicLight);
{
    FIELD_VEC3(m_worldPos);
    FIELD_VEC4(m_diffuse);
    FIELD_VEC4(m_specular);
}
END_RTTI_INIT();

DynamicLight::DynamicLight()
    : m_worldPos(mkVec3::ZERO)
    , m_light(NULL)
    , m_diffuse(1, 1, 1, 1)
    , m_specular(0, 0, 0, 1)
{

}

void DynamicLight::onPostCreate()
{
    __super::onPostCreate();

    m_light = getLevel()->getOgreSceneMgr()->createLight("MainLight");
    m_light->setType(Ogre::Light::LT_POINT);
    m_light->setPosition(m_worldPos);

    m_light->setDiffuseColour(Ogre::ColourValue(m_diffuse.x, m_diffuse.y, m_diffuse.z, m_diffuse.w));
    m_light->setSpecularColour(Ogre::ColourValue(m_specular.x, m_specular.y, m_specular.z, m_specular.w));
}

void DynamicLight::onDestroy()
{
    __super::onDestroy();

    getLevel()->getOgreSceneMgr()->destroyLight(m_light);
    m_light = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IMPLEMENT_RTTI_NOSCRIPT(PlayerFollowingDynamicLight, DynamicLight);

START_RTTI_INIT(PlayerFollowingDynamicLight);
{

}
END_RTTI_INIT();

PlayerFollowingDynamicLight::PlayerFollowingDynamicLight()
{

}

void PlayerFollowingDynamicLight::onUpdate()
{
    __super::onUpdate();

    m_light->setPosition(getLevel()->getPlayer()->getWorldPosition());
}
