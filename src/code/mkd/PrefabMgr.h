/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "utils.h"
#include "Prefab.h"

class PrefabMgr
{
public:
	~PrefabMgr();

	void load(const mkString& filename, btDynamicsWorld* physics_world);
	void clear();

	Prefab* get(const mkString& name) const;

private:
	typedef hash_map<mkString, Prefab*> mkPrefabMap;
	mkPrefabMap m_prefabs;

	// Prefab blob loaded directly from disk
	uint8* m_rawPrefabs;
};
