/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "utils.h"
#include "DynamicBody.h"
#include "PrefabMgr.h"
#include "Game.h"

struct DynamicBodyMotionState : public btDefaultMotionState
{
    DynamicBodyMotionState(Ogre::SceneNode* _node, const btTransform& transform)
        : btDefaultMotionState(transform), node(_node) { }

    Ogre::SceneNode* node;

    virtual void    setWorldTransform(const btTransform& worldTrans)
    {
        node->setPosition(bullet_to_ogre(worldTrans.getOrigin()));
        node->setOrientation(bullet_to_ogre(worldTrans.getRotation()));
    }
};

DynamicBody::DynamicBody( const mkString& prefab_name, const mkVec3& initial_pos )
{
    Prefab* prfb = g_game->getPrefabMgr()->get(prefab_name);

    Ogre::Entity* entity = g_game->getOgreSceneMgr()->createEntity(prfb->mesh_name.c_str());
    m_visualNode = g_game->getOgreSceneMgr()->getRootSceneNode()->createChildSceneNode(initial_pos);
    m_visualNode->attachObject(entity);


    btTransform transform;
    transform.setOrigin(ogre_to_bullet(initial_pos));

    btVector3 local_inertia(0, 0, 0);
    prfb->physics_shape->calculateLocalInertia(prfb->mass, local_inertia);

    btDefaultMotionState* motion_state = new DynamicBodyMotionState(m_visualNode, transform);
    btRigidBody::btRigidBodyConstructionInfo rb_info(prfb->mass, motion_state, prfb->physics_shape, local_inertia);
    m_physicsBody = new btRigidBody(rb_info);

    g_game->getPhysicsWorld()->addRigidBody(m_physicsBody);
}
