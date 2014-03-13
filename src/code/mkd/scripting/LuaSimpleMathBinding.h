/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "utils.h"
#include "LuaSimpleBindingImplCommon.h"

// Do not include anywhere outside Lua binding implementation (for more info, look at the beginning of LuaSimpleBindingImplCommon.h)

namespace lua_simple
{

    // Math binding for LuaSimpleContext
    
    void registerAllMath(LuaSimpleContext* context);
    void registerVec3(LuaSimpleContext* context);
    void registerQuaternion(LuaSimpleContext* context);
    void registerMatrix(LuaSimpleContext* context);

    mkVec3* getVec3FromParam(lua_State* L, int param_num);
    mkVec3* pushNewVec3(lua_State* L);

    TYPE_METATABLE_HELPER(mkVec3);
    TYPE_METATABLE_HELPER(mkQuat);
    //TYPE_METATABLE_HELPER(mkMat3);
    TYPE_METATABLE_HELPER(mkMat4);

}
