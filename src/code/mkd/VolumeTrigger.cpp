#include "pch.h"
#include "VolumeTrigger.h"
#include "contrib/DebugDrawer.h"
#include "Level.h"
#include "scripting/LuaSimpleBinding.h"

IMPLEMENT_RTTI_NOSCRIPT(VolumeTrigger, ModelObject)

START_RTTI_INIT(VolumeTrigger);
{

}
END_RTTI_INIT();

VolumeTrigger::VolumeTrigger()
{

}

void VolumeTrigger::onUpdate()
{
    __super::onUpdate();

    TGameObjectVec objects;
    getLevel()->findObjectsInRadius(objects, &GameObject::Type, getWorldPosition(), getMaxDistanceFromCentre());

    for (size_t i = 0; i < objects.size(); ++i)
    {
        if (objects[i] != this && isObjectTouching(objects[i]))
            onTouched(objects[i]);
    }
}

void VolumeTrigger::onTouched( GameObject* other )
{
    if (getScriptContext())
        getScriptContext()->callFuncArgs("onTouched", ObjectRefBase(other));
}

bool VolumeTrigger::isObjectTouching( GameObject* other )
{
    return false;
}

////////////////////////////////////////////////////////////////////////////////
IMPLEMENT_RTTI_NOSCRIPT(CylinderVolumeTrigger, VolumeTrigger);

START_RTTI_INIT(CylinderVolumeTrigger);
{
    FIELD_FLOAT(m_radius);
    FIELD_BOOL(m_enableDebugRender);
}
END_RTTI_INIT();

CylinderVolumeTrigger::CylinderVolumeTrigger()
    : m_radius(1.f)
    , m_enableDebugRender(false)
{

}

void CylinderVolumeTrigger::onRender()
{
    __super::onRender();

    if (m_enableDebugRender)
        DebugDrawer::getSingleton().drawCylinder(getWorldPosition(), m_radius, 10, 1.f, Ogre::ColourValue::Blue);
}

bool CylinderVolumeTrigger::isObjectTouching( GameObject* other )
{
    if (other->getTypeInfo()->isDerivedOrExact(&ModelObject::Type))
    {
        ModelObject* mo = static_cast<ModelObject*>(other);
        const mkVec3& world_pos = mo->getWorldPosition();

        float dist_sq_from_center = getWorldPosition().squaredDistance(world_pos);

        if (dist_sq_from_center < m_radius * m_radius)
            return true;
    }

    return false;
}

float CylinderVolumeTrigger::getMaxDistanceFromCentre() const
{
    return m_radius;
}