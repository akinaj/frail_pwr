/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "FixedString.h"

class btCollisionShape;

struct xyz { float x, y, z; };

#pragma warning(error: 4820) // warn about hidden padding
struct Prefab
{
    mkFixedString name;
    mkFixedString mesh_name;
    mkFixedString material_name;

    btCollisionShape* physics_shape;
    float mass;
    float vis_scale;
    xyz vis_forward_vec;
    xyz vis_offset;
};
#pragma warning(disable: 4820)
