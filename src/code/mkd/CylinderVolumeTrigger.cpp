/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "CylinderVolumeTrigger.h"
#include "contrib/DebugDrawer.h"
#include "Level.h"
#include "scripting/LuaSimpleBinding.h"

////////////////////////////////////////////////////////////////////////////////
IMPLEMENT_RTTI_NOSCRIPT(CylinderVolumeTrigger, ModelObject);

START_RTTI_INIT(CylinderVolumeTrigger);
{
    FIELD_FLOAT(m_radius);
    FIELD_FLOAT(m_height);
    FIELD_BOOL(m_enableDebugRender);
}
END_RTTI_INIT();

CylinderVolumeTrigger::CylinderVolumeTrigger()
    : m_radius(1.f)
    , m_height(1.f)
    , m_enableDebugRender(false)
{

}

void CylinderVolumeTrigger::onRender()
{
    __super::onRender();

    if (m_enableDebugRender)
        DebugDrawer::getSingleton().drawCylinder(getWorldPosition(), m_radius, 10, m_height, Ogre::ColourValue::Blue);
}

void CylinderVolumeTrigger::onUpdate()
{
    __super::onUpdate();

    TGameObjectVec objects;
    getLevel()->findObjectsInRadius(objects, &ModelObject::Type, getWorldPosition(), m_radius * 2);

    for (size_t i = 0; i < objects.size(); ++i)
    {
        if (objects[i] != this && isObjectTouching(static_cast<ModelObject*>(objects[i])))
            onTouched(objects[i]);
    }
}

void CylinderVolumeTrigger::onTouched( GameObject* other )
{
    if (getScriptContext())
        getScriptContext()->callFuncArgs("onTouched", ObjectRefBase(other));
}

bool CylinderVolumeTrigger::isObjectTouching(ModelObject* other)
{
    // TODO: for model objects, test their OBB instead of just pivot
    mkVec3 other_world_pos = other->getWorldPosition();
    mkVec3 my_world_pos = getWorldPosition();

    if (fabsf(my_world_pos.y - other_world_pos.y) > m_height * 0.5f)
        return false;

    other_world_pos.y = my_world_pos.y;
    const float dist_sq_from_center = my_world_pos.squaredDistance(other_world_pos);

    if (dist_sq_from_center > m_radius * m_radius)
        return false;

    return true;
}
