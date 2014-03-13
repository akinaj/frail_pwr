/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "ModelObject.h"

class CylinderVolumeTrigger : public ModelObject
{
    DECLARE_RTTI(CylinderVolumeTrigger);

public:
    CylinderVolumeTrigger();

    virtual void onUpdate();
    virtual void onRender();

protected:
    bool isObjectTouching(ModelObject* other);
    void onTouched(GameObject* other);

    float m_radius;
    float m_height;
    bool m_enableDebugRender;
};
