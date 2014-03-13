#pragma once
#include "GameObject.h"

// When object of this class is created, it changes render settings of level and destroys itself
// It's meant to be used in serialized levels to simplify level format (just list of game objects, no metadata)
class RenderSettingsSetter : public GameObject
{
    DECLARE_RTTI(RenderSettingsSetter);

public:
    RenderSettingsSetter();

    virtual void onCreate();

private:
    mkVec3 m_ambientLight;
};
