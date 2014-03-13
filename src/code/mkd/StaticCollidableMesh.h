/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "ModelObject.h"

class StaticCollidableMesh : public ModelObject
{
    DECLARE_RTTI(StaticCollidableMesh);

public:
    StaticCollidableMesh();

    virtual void onCreate();
    virtual void onDestroy();

private:
    btRigidBody* m_physicsBody;

    btBvhTriangleMeshShape* createCollisionFromRenderingMesh();
};
