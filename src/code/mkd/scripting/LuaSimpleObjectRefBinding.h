/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "LuaSimpleBindingImplCommon.h"
#include "ObjectRef.h"

// Do not include anywhere outside Lua binding implementation (for more info, look at the beginning of LuaSimpleBindingImplCommon.h)

namespace lua_simple
{

    void registerObjectRef(LuaSimpleContext* context);

    const ObjectRefBase& getObjectRefFromParam(lua_State* L, int param_num);
    ObjectRefBase& pushNewObjectRef(lua_State* L);

    TYPE_METATABLE_HELPER(ObjectRefBase);

}
