/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once

struct Prefab;

class MeshObject
{
public:
    MeshObject(const mkVec3& pos, Prefab* prefab);
    MeshObject(const mkVec3& pos, const mkString& mesh_name);
    ~MeshObject();

    void setWorldPos(const mkVec3& new_pos);
    mkVec3 getWorldPos() const { return m_pos; }

private:
    void create(const mkString& mesh_name, const mkString& material_name);
    void destroy();

    mkVec3 m_pos;

    Ogre::SceneNode* m_visualsNode;
    Ogre::Entity* m_visualsMesh;
};
