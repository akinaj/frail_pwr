/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once

// This file should be included only in Lua binding implementation. To ensure this, it does not include lua.hpp,
// so including it anywhere else will cause compiler error without explicitly including lua.hpp before

namespace lua_simple
{
    class LuaSimpleContext;

    // Helpers for generic registration functions (ugly macro & templates marriage, but allows simpler user code)

    template <typename T>
    struct MetatableHelper
    {

    };

    // In .cpp files with binding implementation define IMPLEMENT_METATABLE_NAMES before any include

#ifdef IMPLEMENT_METATABLE_NAMES
#define METATABLE_NAME(type) const char* MetatableHelper<type>::Name = #type;
#else
#define METATABLE_NAME(type)
#endif

#define TYPE_METATABLE_HELPER(type)                \
    template <> struct MetatableHelper<type> {    \
        static const char* Name;                \
    };                                            \
    METATABLE_NAME(type);
    // End TYPE_METATABLE_HELPER def

    // Creates userdata object and pushes it on Lua stack
    template <typename T>
    static T* pushNewLuaObject(lua_State* L)
    {
        T* result = (T*)lua_newuserdata(L, sizeof(T));
        luaL_getmetatable(L, MetatableHelper<T>::Name);
        lua_setmetatable(L, -2);

        return result;
    }

    // Returns userdata object from specified index on stack
    // Performs additional checks so mismatched types will cause error
    template <typename T>
    static T* tryGetLuaObject(lua_State* L, int index)
    {
        T* obj = NULL;

        luaL_checktype(L, index, LUA_TUSERDATA);

        obj = (T*)luaL_checkudata(L, index, MetatableHelper<T>::Name);

        if (obj == NULL)
            luaL_typerror(L, index, MetatableHelper<T>::Name);

        return obj;
    }

    template <typename T>
    void _registerType(lua_State* L, const luaL_reg methods[], const luaL_reg metatable[])
    {
        luaL_openlib(L, MetatableHelper<T>::Name, methods, 0);
        luaL_newmetatable(L, MetatableHelper<T>::Name);
        luaL_openlib(L, 0, metatable, 0);
        lua_pushliteral(L, "__index");
        lua_pushvalue(L, -3);
        lua_rawset(L, -3);
        lua_pushliteral(L, "__metatable");
        lua_pushvalue(L, -3);
        lua_rawset(L, -3);
        lua_pop(L, 1);
    }

    template <typename T>
    bool isArgumentOfType(lua_State* L, int param_idx)
    {
        lua_getmetatable(L, param_idx);
        luaL_getmetatable(L, MetatableHelper<T>::Name);

        return (lua_equal(L, -1, -2) == 1);
    }

}
