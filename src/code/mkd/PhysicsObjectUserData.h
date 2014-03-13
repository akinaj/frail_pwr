/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once

class Character;
class EnemyGroup;
class GameObject;

struct PhysicsObjUserData
{
    enum Type
    {
        CharacterObject = 1 << 1,
        PlayerObject =    1 << 2,
        EnemyObject  =    1 << 3,
        NeutralObject =   1 << 4,
        DynGameObject   = 1 << 8
    };

    uint32 type;
    Character* character;
    GameObject* game_obj;
};
