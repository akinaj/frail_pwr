/****************************************************
 * mkdemo 2011-2013                                 *
 * author: Maciej Kurowski 'kurak' & Andrzej Cwenar *
 ****************************************************/
#include "pch.h"
#include "utils.h"
#include "MeshObject.h"
#include "Game.h"
#include "PrefabMgr.h"

MeshObject::MeshObject( const mkVec3& pos, const mkString& mesh_name )
	: m_pos(pos)
	, m_visualsNode(NULL)
	, m_visualsMesh(NULL)
{
	create(mesh_name, "");
}

MeshObject::MeshObject( const mkVec3& pos, Prefab* prefab )
	: m_pos(pos)
	, m_visualsNode(NULL)
	, m_visualsMesh(NULL)
{
	create(prefab->mesh_name.c_str(), prefab->material_name.c_str());
}

MeshObject::~MeshObject()
{
	destroy();
}

void MeshObject::setWorldPos( const mkVec3& new_pos )
{
	m_pos = new_pos;
	m_visualsNode->setPosition(new_pos);
}

void MeshObject::create(const mkString& mesh_name, const mkString& material_name)
{
	m_visualsMesh = g_game->getOgreSceneMgr()->createEntity(mesh_name);
	
	if (!material_name.empty())
		m_visualsMesh->setMaterialName(material_name);

	m_visualsNode = g_game->getOgreSceneMgr()->getRootSceneNode()->createChildSceneNode(m_pos);
	if (m_visualsMesh)
		m_visualsNode->attachObject(m_visualsMesh);
}

void MeshObject::destroy()
{
	g_game->getOgreSceneMgr()->getRootSceneNode()->removeChild(m_visualsNode);
	g_game->getOgreSceneMgr()->destroySceneNode(m_visualsNode);

	m_visualsNode = NULL;
	m_visualsMesh = NULL;
}
