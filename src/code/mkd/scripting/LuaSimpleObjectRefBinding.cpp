#include "pch.h"

#include <lua.hpp>

#define IMPLEMENT_METATABLE_NAMES
#include "LuaSimpleObjectRefBinding.h"
#include "LuaSimpleBinding.h"
#include "ObjectRef.h"

namespace lua_simple
{

static int _objectRef_isValid(lua_State* L)
{
    ObjectRefBase* ref_ptr = tryGetLuaObject<ObjectRefBase>(L, 1);
    const bool result = ref_ptr->isValid();
    lua_pushboolean(L, (int)result);

    return 1;
}

static int _objectRef_getObject(lua_State* L)
{
    ObjectRefBase* ref_ptr = tryGetLuaObject<ObjectRefBase>(L, 1);
    GameObject* result = ref_ptr->fetchBasePtr();
    lua_pushlightuserdata(L, (void*)result);

    return 1;
}

static const luaL_reg g_ObjectRef_Methods[] =
{
    { "IsValid",  _objectRef_isValid },
    { "GetObject", _objectRef_getObject },
    { 0, 0 }
};

void lua_simple::registerObjectRef( LuaSimpleContext* context )
{
    lua_State* L = context->_getLuaState();

    _registerType<ObjectRefBase>(context->_getLuaState(), g_ObjectRef_Methods, g_ObjectRef_Methods);
}

const ObjectRefBase& lua_simple::getObjectRefFromParam( lua_State* L, int param_num )
{
    ObjectRefBase* ref_ptr = (ObjectRefBase*)lua_touserdata(L, param_num);
    return *ref_ptr;
}

ObjectRefBase& lua_simple::pushNewObjectRef(lua_State* L)
{
    ObjectRefBase* result = (ObjectRefBase*)lua_newuserdata(L, sizeof(ObjectRefBase));
    luaL_getmetatable(L, MetatableHelper<ObjectRefBase>::Name);
    lua_setmetatable(L, -2);

    return *result;
}

}