/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "GameObject.h"

class DynamicLight : public GameObject
{
    DECLARE_RTTI(DynamicLight);

public:
    DynamicLight();

    virtual void onPostCreate();
    virtual void onDestroy();

protected:
    mkVec3 m_worldPos;
    Ogre::Light* m_light;

    mkVec4 m_diffuse;
    mkVec4 m_specular;
};

class PlayerFollowingDynamicLight : public DynamicLight
{
    DECLARE_RTTI(PlayerFollowingDynamicLight);

public:
    PlayerFollowingDynamicLight();

    virtual void onUpdate();
};
