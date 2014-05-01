/****************************************************
 * mkdemo 2011-2013                                 *
 * author: Maciej Kurowski 'kurak' & Andrzej Cwenar *
 ****************************************************/
#include "pch.h"
#include "PrefabMgr.h"
#include "Filesystem.h"

#pragma warning(error: 4820) // warn about hidden padding
struct PrefabList
{
	uint32 fourcc;
	uint32 num_prefabs;
	Prefab prefabs[1];
};
#pragma warning(disable: 4820)

PrefabMgr::~PrefabMgr()
{
	clear();
}

void PrefabMgr::load( const mkString& filename, btDynamicsWorld* physics_world )
{
	uint32 buf_size = 0;
	bool loaded = loadFile(filename.c_str(), &m_rawPrefabs, &buf_size);

	if(!loaded)
		return;

	PrefabList* prefab_list = (PrefabList*)m_rawPrefabs;
	assert(prefab_list->fourcc == MAKEFOURCC('P', 'R', 'F', 'B'));

	char* collision_shapes_buffer = (char*)(&prefab_list->prefabs[0] + prefab_list->num_prefabs);
	int prefab_buff_len = (collision_shapes_buffer - (char*)m_rawPrefabs);
	int collision_buff_len = (int)buf_size - prefab_buff_len;
	assert(collision_buff_len > 0);

	btBulletWorldImporter bullet_importer(physics_world);
	if(!bullet_importer.loadFileFromMemory(collision_shapes_buffer, collision_buff_len))
	{
		// failed
		return;
	}

	for(uint32 i = 0; i < prefab_list->num_prefabs; ++i)
	{
		Prefab* prefab = prefab_list->prefabs + i;
		prefab->physics_shape = bullet_importer.getCollisionShapeByIndex(i);
		m_prefabs[prefab->name.c_str()] = prefab;
	}
}

void PrefabMgr::clear()
{
	delete[] m_rawPrefabs;
}

Prefab* PrefabMgr::get( const mkString& name ) const
{
	mkPrefabMap::const_iterator iter = m_prefabs.find(name);

	if(iter != m_prefabs.end())
	{
		return iter->second;
	}
	else
	{
		return 0;
	}
}
