#include "pch.h"
#include "TestGameObject.h"
#include "contrib/DebugDrawer.h"

IMPLEMENT_RTTI_NOSCRIPT(TestGameObject, GameObject);

START_RTTI_INIT(TestGameObject);
{
    FIELD_VEC3(m_position);
}
END_RTTI_INIT();

void TestGameObject::onRender()
{
    __super::onRender();

    DebugDrawer::getSingleton().drawSphere(m_position, 5.f, Ogre::ColourValue::Blue, true);
}
