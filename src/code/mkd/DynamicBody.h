/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once

// Dynamic body is object with both physical and visual representation
class DynamicBody
{
public:
    explicit DynamicBody(const mkString& prefab_name, const mkVec3& initial_pos);
    ~DynamicBody();

private:
    Ogre::SceneNode* m_visualNode;
    btRigidBody* m_physicsBody;
};
