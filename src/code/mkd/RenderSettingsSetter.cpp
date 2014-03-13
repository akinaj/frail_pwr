#include "pch.h"
#include "RenderSettingsSetter.h"
#include "Game.h"

IMPLEMENT_RTTI_NOSCRIPT(RenderSettingsSetter, GameObject);

START_RTTI_INIT(RenderSettingsSetter);
{
    FIELD_VEC3(m_ambientLight);
}
END_RTTI_INIT();

RenderSettingsSetter::RenderSettingsSetter()
{
    m_ambientLight = mkVec3::UNIT_SCALE;
}

void RenderSettingsSetter::onCreate()
{
    __super::onCreate();

    g_game->getOgreSceneMgr()->setAmbientLight(Ogre::ColourValue(m_ambientLight.x, m_ambientLight.y, m_ambientLight.z));

    destroy();
}
