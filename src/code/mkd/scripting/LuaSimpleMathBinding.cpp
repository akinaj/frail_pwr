/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"

#include "LuaSimpleBinding.h"

#include <lua.hpp>

#define IMPLEMENT_METATABLE_NAMES
#include "LuaSimpleMathBinding.h"

namespace lua_simple
{

    void registerAllMath( LuaSimpleContext* context )
    {
        registerVec3(context);
        registerQuaternion(context);
        registerMatrix(context);
    }

    static int mkVec3_new(lua_State* L)
    {
        const float x = (float)luaL_optnumber(L, 1, 0);
        const float y = (float)luaL_optnumber(L, 2, 0);
        const float z = (float)luaL_optnumber(L, 3, 0);

        mkVec3* obj = pushNewLuaObject<mkVec3>(L);
        obj->x = x;
        obj->y = y;
        obj->z = z;

        return 1;
    }

    static int mkQuat_new(lua_State* L)
    {
        mkVec3 axis = mkVec3::ZERO;
        float angle = 0.f;

        // mkQuat(axis, angle_deg)
        if (lua_isuserdata(L, 1) != 0)
        {
            axis = *tryGetLuaObject<mkVec3>(L, 1);
            angle = (float)lua_tonumber(L, 2);
        }

        mkQuat* obj = pushNewLuaObject<mkQuat>(L);
        *obj = mkQuat(Ogre::Degree(angle), axis);

        return 1;
    }

#define BINARY_OP(clazz, name, op)                  \
    static int clazz##_##name(lua_State* L) {       \
        clazz* a = tryGetLuaObject<clazz>(L, 1);    \
        clazz* b = tryGetLuaObject<clazz>(L, 2);    \
        clazz* result = pushNewLuaObject<clazz>(L); \
        op;                                         \
        return 1;                                   \
    }

#define BINARY_OP_RET_FLOAT(clazz, name, op)        \
    static int clazz##_##name(lua_State* L) {       \
        clazz* a = tryGetLuaObject<clazz>(L, 1);    \
        clazz* b = tryGetLuaObject<clazz>(L, 2);    \
        float result; op;                           \
        lua_pushnumber(L, result);                  \
        return 1;                                   \
    }

#define BINARY_OP_RET_OTHER(clazz, ret_clazz, name, op) \
    static int clazz##_##name(lua_State* L) {           \
        clazz* a = tryGetLuaObject<clazz>(L, 1);        \
        clazz* b = tryGetLuaObject<clazz>(L, 2);        \
        clazz* result = pushNewLuaObject<ret_clazz>(L); \
        op;                                             \
        return 1;                                       \
    }

BINARY_OP(mkVec3, add, *result = *a + *b);
BINARY_OP(mkVec3, subtract, *result = *a - *b);
BINARY_OP(mkVec3, cross, *result = a->crossProduct(*b));
BINARY_OP_RET_FLOAT(mkVec3, dot, result = a->dotProduct(*b));
BINARY_OP_RET_FLOAT(mkVec3, distance, result = a->distance(*b));
BINARY_OP_RET_FLOAT(mkVec3, distance_sq, result = a->squaredDistance(*b));

    static int mkVec3_normalized(lua_State* L)
    {
        mkVec3* arg = tryGetLuaObject<mkVec3>(L, 1);
        mkVec3* result = pushNewLuaObject<mkVec3>(L);
        *result = arg->normalisedCopy();

        return 1;
    }

    static int mkVec3_length(lua_State* L)
    {
        mkVec3* arg = tryGetLuaObject<mkVec3>(L, 1);
        lua_pushnumber(L, arg->length());

        return 1;
    }

    static int mkVec3_length_sq(lua_State* L)
    {
        mkVec3* arg = tryGetLuaObject<mkVec3>(L, 1);
        lua_pushnumber(L, arg->squaredLength());

        return 1;
    }

    static int mkVec3_scale(lua_State* L)
    {
        int vec_idx = lua_isnumber(L, 1) ? 2 : 1;
        int scale_idx = vec_idx == 2 ? 1 : 2;

        mkVec3* lhs = tryGetLuaObject<mkVec3>(L, vec_idx);
        float rhs = (float)lua_tonumber(L, scale_idx);

        mkVec3* result = pushNewLuaObject<mkVec3>(L);
        *result = *lhs * rhs;

        return 1;
    }

    static int mkVec3_rotation_to(lua_State* L)
    {
        mkVec3* arg1 = tryGetLuaObject<mkVec3>(L, 1);
        mkVec3* arg2 = tryGetLuaObject<mkVec3>(L, 2);

        mkQuat* result = pushNewLuaObject<mkQuat>(L);
        *result = arg1->getRotationTo(*arg2);

        return 1;
    }

#define VEC3_GET_COMPONENT_FUN(component)               \
    static int mkVec3_##component (lua_State* L)        \
    {                                                   \
        mkVec3* arg = tryGetLuaObject<mkVec3>(L, 1);    \
        lua_pushnumber(L, arg->component);              \
        return 1;                                       \
    }

    VEC3_GET_COMPONENT_FUN(x);
    VEC3_GET_COMPONENT_FUN(y);
    VEC3_GET_COMPONENT_FUN(z);


    static const luaL_reg s_Vec3_Methods[] =
    {
        { "new", mkVec3_new },
        { "add", mkVec3_add },
        { "subtract", mkVec3_subtract},
        { "dot", mkVec3_dot },
        { "cross", mkVec3_cross },
        { "dist", mkVec3_distance },
        { "dist_sq", mkVec3_distance_sq },
        { "normalized", mkVec3_normalized },
        { "length", mkVec3_length },
        { "length_sq", mkVec3_length_sq },
        { "scale", mkVec3_scale },
        { "rotation_to", mkVec3_rotation_to },
        { "x", mkVec3_x },
        { "y", mkVec3_y },
        { "z", mkVec3_z },
        { 0, 0 }
    };

    static int mkVec3_toString(lua_State* L)
    {
        mkVec3* arg = tryGetLuaObject<mkVec3>(L, 1);
        mkString result = vec32str(*arg);
        
        lua_pushstring(L, result.c_str());

        return 1;
    }

    static const luaL_reg s_Vec3_Metatable[] =
    {
        { "__tostring", mkVec3_toString },
        { "__add", mkVec3_add },
        { "__sub", mkVec3_subtract },
        { "__mul", mkVec3_scale },
        { "__call", mkVec3_new },
        { 0, 0 }
    };

    void registerVec3( LuaSimpleContext* context )
    {
        _registerType<mkVec3>(context->_getLuaState(), s_Vec3_Methods, s_Vec3_Metatable);
    }

    static int mkQuat_transformVector(lua_State* L)
    {
        mkQuat quat = *tryGetLuaObject<mkQuat>(L, 1);
        mkVec3 vec = *tryGetLuaObject<mkVec3>(L, 2);

        mkVec3* result = pushNewVec3(L);
        *result = quat * vec;

        return 1;
    }

    static int mkQuat_getYawDeg(lua_State* L)
    {
        const float result = tryGetLuaObject<mkQuat>(L, 1)->getYaw().valueDegrees();
        lua_pushnumber(L, result);

        return 1;
    }

    static int mkQuat_getPitchDeg(lua_State* L)
    {
        const float result = tryGetLuaObject<mkQuat>(L, 1)->getPitch().valueDegrees();
        lua_pushnumber(L, result);

        return 1;
    }

    static int mkQuat_getRollDeg(lua_State* L)
    {
        const float result = tryGetLuaObject<mkQuat>(L, 1)->getRoll().valueDegrees();
        lua_pushnumber(L, result);

        return 1;
    }

    static int mkQuat_getAxis(lua_State* L)
    {
        mkQuat quat = *tryGetLuaObject<mkQuat>(L, 1);
        mkVec3* result = pushNewVec3(L);
        
        Ogre::Radian dummy;
        quat.ToAngleAxis(dummy, *result);

        return 1;
    }

    static int mkQuat_getAngleDeg(lua_State* L)
    {
        mkQuat quat = *tryGetLuaObject<mkQuat>(L, 1);
        mkVec3 axis;
        Ogre::Degree angle;

        quat.ToAngleAxis(angle, axis);
        lua_pushnumber(L, angle.valueDegrees());

        return 1;
    }

    static int mkQuat_toAxisAngleDeg(lua_State* L)
    {
        mkQuat quat = *tryGetLuaObject<mkQuat>(L, 1);
        mkVec3* axis = pushNewVec3(L);
        Ogre::Degree angle;

        quat.ToAngleAxis(angle, *axis);
        lua_pushnumber(L, angle.valueDegrees());

        return 1;
    }

    static int mkQuat_toString(lua_State* L)
    {
        mkQuat quat = *tryGetLuaObject<mkQuat>(L, 1);
        mkString result = quat2str(quat);

        lua_pushstring(L, result.c_str());

        return 1;
    }

    static luaL_reg g_mkQuat_Methods[] =
    {
        { "new", mkQuat_new },
        { "transformVector", mkQuat_transformVector },
        { "yaw", mkQuat_getYawDeg },
        { "pitch", mkQuat_getPitchDeg },
        { "roll", mkQuat_getRollDeg },
        { "getAxis", mkQuat_getAxis },
        { "getAngle", mkQuat_getAngleDeg },
        { "getAxisAngle", mkQuat_toAxisAngleDeg },
        { 0, 0 }
    };

    static luaL_reg g_mkQuat_Metatable[] =
    {
        { "__tostring", mkQuat_toString },
        { "__mul", mkQuat_transformVector },
        { "__call", mkQuat_new },
        { 0, 0 }
    };

    void registerQuaternion( LuaSimpleContext* context )
    {
        _registerType<mkQuat>(context->_getLuaState(), g_mkQuat_Methods, g_mkQuat_Metatable);
    }

    static int mkMat4_new(lua_State* L)
    {
        mkMat4 initial_value;
        if (lua_isuserdata(L, 1) != 0)
            initial_value = *tryGetLuaObject<mkMat4>(L, 1);
        else
            initial_value = mkMat4::IDENTITY; // always clear matrices created in script

        mkMat4* result = pushNewLuaObject<mkMat4>(L);
        *result = initial_value;

        return 1;
    }

    static int mkMat4_tostring(lua_State* L)
    {
        mkMat4* val = tryGetLuaObject<mkMat4>(L, 1);
        mkString result = mat42str(*val);

        lua_pushstring(L, result.c_str());
        return 1;
    }

    static int mkMat4_getRotation(lua_State* L)
    {
        mkMat4* val = tryGetLuaObject<mkMat4>(L, 1);
        mkQuat* result = pushNewLuaObject<mkQuat>(L);
        mkVec3 dummy1, dummy2;

        val->decomposition(dummy1, dummy2, *result);
        return 1;
    }

    static int mkMat4_getTranslation(lua_State* L)
    {
        mkMat4* val = tryGetLuaObject<mkMat4>(L, 1);
        mkVec3* result = pushNewLuaObject<mkVec3>(L);
        mkVec3 dummy1;
        mkQuat dummy2;

        val->decomposition(*result, dummy1, dummy2);
        return 1;
    }

    static int mkMat4_getScale(lua_State* L)
    {
        mkMat4* val = tryGetLuaObject<mkMat4>(L, 1);
        mkVec3* result = pushNewLuaObject<mkVec3>(L);
        mkVec3 dummy1;
        mkQuat dummy2;

        val->decomposition(dummy1, *result, dummy2);
        return 1;
    }

    static int mkMat4_fromPosScaleRot(lua_State* L)
    {
        mkVec3 pos = *tryGetLuaObject<mkVec3>(L, 1);
        mkVec3 scale = *tryGetLuaObject<mkVec3>(L, 2);
        mkQuat rot = *tryGetLuaObject<mkQuat>(L, 3);

        mkMat4* result = pushNewLuaObject<mkMat4>(L);
        result->makeTransform(pos, scale, rot);

        return 1;
    }

    static int mkMat4_inv(lua_State* L)
    {
        mkMat4* val = tryGetLuaObject<mkMat4>(L, 1);
        mkMat4* result = pushNewLuaObject<mkMat4>(L);
        *result = val->inverse();

        return 1;
    }

    static int mkMat4_mul(lua_State* L)
    {
        mkMat4* lhs = tryGetLuaObject<mkMat4>(L, 1);

        if (isArgumentOfType<mkMat4>(L, 2))
        {
            mkMat4* rhs = tryGetLuaObject<mkMat4>(L, 2);
            mkMat4* result = pushNewLuaObject<mkMat4>(L);
            *result = *lhs * *rhs;
        }
        else if (isArgumentOfType<mkVec3>(L, 2))
        {
            mkVec3* rhs = tryGetLuaObject<mkVec3>(L, 2);
            mkVec3* result = pushNewLuaObject<mkVec3>(L);

            // Default behavior is to treat vector as point (not direction). To transform direction, use mul_dir
            *result = transform_point(*lhs, *rhs);
        }

        return 1;
    }

    static int mkMat4_mul_dir(lua_State* L)
    {
        mkMat4* lhs = tryGetLuaObject<mkMat4>(L, 1);
        mkVec3* rhs = tryGetLuaObject<mkVec3>(L, 2);
        
        mkVec3* result = pushNewLuaObject<mkVec3>(L);
        *result = transform_dir(*lhs, *rhs);

        return 1;
    }

    static luaL_reg g_mkMat4_Methods[] =
    {
        { "new", mkMat4_new },
        { "tostring", mkMat4_tostring },
        { "getRotation", mkMat4_getRotation },
        { "getScale", mkMat4_getScale },
        { "getTranslation", mkMat4_getTranslation },
        { "fromPosScaleRot", mkMat4_fromPosScaleRot },
        { "getInverse", mkMat4_inv },
        { "mul", mkMat4_mul },
        { "mul_dir", mkMat4_mul_dir },
        { 0, 0 }
    };

    static luaL_reg g_mkMat4_Metatable[] =
    {
        { "__mul", mkMat4_mul },
        { "__tostring", mkMat4_tostring },
        { "__call", mkMat4_new },
        { 0, 0 }
    };

    void registerMatrix( LuaSimpleContext* context )
    {
        _registerType<mkMat4>(context->_getLuaState(), g_mkMat4_Methods, g_mkMat4_Metatable);
    }

    mkVec3* getVec3FromParam( lua_State* L, int param_num )
    {
        return tryGetLuaObject<mkVec3>(L, param_num);
    }

    mkVec3* pushNewVec3( lua_State* L )
    {
        return pushNewLuaObject<mkVec3>(L);
    }

}
