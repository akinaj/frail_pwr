/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "GameObject.h"

class TestGameObject : public GameObject
{
    DECLARE_RTTI(TestGameObject);

public:
    virtual void onRender();

private:
    mkVec3 m_position;
};
