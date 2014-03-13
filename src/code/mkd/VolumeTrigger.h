#pragma once
#include "ModelObject.h"

class VolumeTrigger : public ModelObject
{
    DECLARE_RTTI(VolumeTrigger);

public:
    VolumeTrigger();

    virtual void onUpdate();
    virtual void onTouched(GameObject* other);

protected:
    virtual bool isObjectTouching(GameObject* other);
    virtual float getMaxDistanceFromCentre() const { return 0.f; }
};

class CylinderVolumeTrigger : public VolumeTrigger
{
    DECLARE_RTTI(CylinderVolumeTrigger);

public:
    CylinderVolumeTrigger();

    virtual void onRender();

protected:
    virtual bool isObjectTouching(GameObject* other);
    virtual float getMaxDistanceFromCentre() const;

    float m_radius;
    bool m_enableDebugRender;
};
