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
