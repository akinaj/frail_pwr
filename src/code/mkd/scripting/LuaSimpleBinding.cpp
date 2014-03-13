#include "pch.h"
#include "LuaSimpleBinding.h"
#include "utils.h"
#include "Filesystem.h"

#include <lua.hpp>

#include "LuaSimpleMathBinding.h"
#include "LuaSimpleObjectRefBinding.h"
#include "LuaSimpleRttiBinding.h"

#if _DEBUG
#pragma comment(lib, "lua51d.lib")
#else
#pragma comment(lib, "lua51.lib")
#endif
// TODO: all calls to lua functions are unsafe! surround them with pcall or smth...

namespace lua_simple
{
    ////////////////////////////////////////////////////////////////////////////////////////////////

    typedef std::map<lua_State*, LuaSimpleContext*> TDebugLuaStateToContextMap;
    static TDebugLuaStateToContextMap s_DebugLuaStateToContext;

    static void debugRegisterLuaState(lua_State* state, LuaSimpleContext* context)
    {
        s_DebugLuaStateToContext[state] = context;
    }

    static void debugUnregisterLuaState(lua_State* state)
    {
        s_DebugLuaStateToContext.erase(state);
    }

    static LuaSimpleContext* debugGetContextForLuaState(lua_State* state)
    {
        return s_DebugLuaStateToContext[state];
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////

    struct LuaSimpleContextPimpl
    {
        LuaSimpleContextPimpl()
            : L(NULL)
            , validation_idx(1)
        {

        }

        lua_State* L;
        int validation_idx;
        void increaseValidationIdx();
    };

    void LuaSimpleContextPimpl::increaseValidationIdx()
    {
        ++validation_idx;
    }

    int panic(lua_State* L)
    {
        const char* error_msg = lua_tostring(L, -1);
        mkString context_dbg_name = "(n/a)";

        LuaSimpleContext* context = debugGetContextForLuaState(L);
        if (context)
            context_dbg_name = context->getName();

        if (error_msg && strstr(error_msg, "bad FPU precision"))
        {
            log_error("Lua panic (context %s): '%s'\n\n"
                "Hint: try setting 'Floating-point mode' to 'Consistent' in OGRE settings window on startup",
                context_dbg_name.c_str(), error_msg);
        }
        else
        {
            log_error("Lua panic (context %s): %s",
                context_dbg_name.c_str(), error_msg);
        }

        return 0;
    }

    LuaSimpleContext::LuaSimpleContext(const mkString& name)
        : m_pimpl(new LuaSimpleContextPimpl)
        , m_name(name)
    {
        m_pimpl->L = luaL_newstate();
        debugRegisterLuaState(m_pimpl->L, this);

        lua_atpanic(m_pimpl->L, panic);
        luaL_openlibs(m_pimpl->L);
    }

    LuaSimpleContext::~LuaSimpleContext()
    {
        // TODO check for any hanging variable references?

        debugUnregisterLuaState(m_pimpl->L);

        lua_close(m_pimpl->L);
        delete m_pimpl;
    }

    bool LuaSimpleContext::runString( const char* code )
    {
        bool success = true;

        int status = luaL_dostring(m_pimpl->L, code);
        if (status != 0)
        {
            log_error("Error running lua string: %s", lua_tostring(m_pimpl->L, -1));
            success = false;
        }

        m_pimpl->increaseValidationIdx();

        return success;
    }

    bool LuaSimpleContext::runFile( const mkString& filename )
    {
        uint8* text = NULL;
        uint32 size = 0;

        if (!loadFile(filename.c_str(), &text, &size, true))
        {
            log_error("Could not read Lua script file '%s'", filename.c_str());
            return false;
        }

        return runString((const char*)text);
    }

    void LuaSimpleContext::tryCallFunc( const char* name )
    {
        callFunc(name, true);
    }

    bool LuaSimpleContext::callFunc( const char* name, const FunctionCallArgs& args, bool ignore_non_existent )
    {
        const int stack_size_before = lua_gettop(m_pimpl->L);

        lua_getglobal(m_pimpl->L, name);
        if (lua_isfunction(m_pimpl->L, -1) != 1)
        {
            if (!ignore_non_existent)
            {
                log_error("Could not call function '%s' with %d args: no such function exist in context '%s'",
                    name, args.args.size(), getName().c_str());
            }

            lua_pop(m_pimpl->L, 1);
            return false;
        }

        for (size_t i = 0; i < args.args.size(); ++i)
        {
            const FunctionCallArg& arg = args.args[i];

            if (arg.is_number)
                lua_pushnumber(m_pimpl->L, arg.number_value);
            else if (arg.object_ref.isSet())
                pushNewObjectRef(m_pimpl->L) = arg.object_ref;
            else
                lua_pushstring(m_pimpl->L, arg.str_value.c_str());
        }

        // TODO: pcall error handler (last argument of lua_pcall) with more debug info like callstack
        const int result = lua_pcall(m_pimpl->L, args.args.size(), 0, 0);
        if (result != 0)
        {
            const char* error_str = lua_tostring(m_pimpl->L, -1);
            log_error("Error when executing function '%s' with %d args in context '%s': %s",
                name, args.args.size(), getName().c_str(), error_str);
            lua_pop(m_pimpl->L, 1);

            MK_ASSERT(lua_gettop(m_pimpl->L) == stack_size_before);
            return false;
        }

        m_pimpl->increaseValidationIdx();

        MK_ASSERT(lua_gettop(m_pimpl->L) == stack_size_before);

        return true;
    }

    bool LuaSimpleContext::callFunc( const char* name, bool ignore_non_existent )
    {
        FunctionCallArgs args;
        return callFunc(name, args, ignore_non_existent);
    }

    lua_simple::VariableRef LuaSimpleContext::getGlobalVariable( const char* name )
    {
        return VariableRef(name, this);
    }

    void LuaSimpleContext::setGlobalUserPointer( const char* name, void* ptr )
    {
        lua_pushlightuserdata(m_pimpl->L, ptr);
        lua_setglobal(m_pimpl->L, name);
    }

    LuaSimpleContextPimpl* LuaSimpleContext::getPimpl() const
    {
        return m_pimpl;
    }

    mkString LuaSimpleContext::getName() const
    {
        return m_name;
    }

    VariableRef::VariableRef( const mkString& var_name, LuaSimpleContext* context, VariableRef* parent_var )
        : m_varName(var_name)
        , m_validationIdx(context->getPimpl()->validation_idx)
        , m_ownerContext(context)
        , m_parentVar(parent_var)
    {
    }

#define VALIDATE() do { MK_ASSERT(validate()); if (!validate()) return false; } while (false)

    bool VariableRef::readInt32( int32& out ) const
    {
        VALIDATE();

        pushValue();
        out = (int32)lua_tonumber(m_ownerContext->getPimpl()->L, -1);
        popValue();

        return true;
    }

    bool VariableRef::readUInt32( uint32& out ) const
    {
        VALIDATE();

        pushValue();
        out = (uint32)lua_tonumber(m_ownerContext->getPimpl()->L, -1);
        popValue();

        return true;
    }

    bool VariableRef::readFloat( float& out ) const
    {
        VALIDATE();

        pushValue();
        out = (float)lua_tonumber(m_ownerContext->getPimpl()->L, -1);
        popValue();

        return true;
    }

    bool VariableRef::readString( mkString& out ) const
    {
        VALIDATE();

        bool success = false;

        pushValue();
        const char* ptr = lua_tolstring(m_ownerContext->getPimpl()->L, -1, NULL);
        if (ptr)
        {
            success = true;
            out = ptr;
        }
        popValue();

        return success;
    }

    bool VariableRef::readBytes( size_t num, uint8* out_buf ) const
    {
        VALIDATE();

        lua_State* L = m_ownerContext->getPimpl()->L;

        bool success = false;

        pushValue();
        if (lua_istable(L, -1) == 1)
        {
            success = true;

            lua_pushnil(L);

            for (size_t i = 0; i < num; ++i)
            {
                const int next_ret = lua_next(L, -2);
                if (next_ret == 0)
                {
                    log_error("Not enough values in table (%d required, has %d)", num, i);
                    success = false;
                    break;
                }

                if (lua_isnumber(L, -1) == 0)
                {
                    log_error("Non-number value in table when reading bytes");
                    success = false;
                    break;
                }

                // Checks for fractional parts, too high values etc...

                const uint8 val = (uint8)lua_tonumber(L, -1);
                out_buf[i] = val;

                lua_pop(L, 1);
            }

            lua_pop(L, 1);
        }
        popValue();

        return true;
    }

#undef VALIDATE

    bool VariableRef::validate() const
    {
        return m_ownerContext->getPimpl()->validation_idx == m_validationIdx;
    }

    void VariableRef::pushValue() const
    {
        if (m_parentVar == NULL)
            lua_getglobal(m_ownerContext->getPimpl()->L, m_varName.c_str());
        else
        {
            m_parentVar->pushValue();
            lua_getfield(m_ownerContext->getPimpl()->L, -1, m_varName.c_str());
        }
    }

    void VariableRef::popValue() const
    {
        lua_pop(m_ownerContext->getPimpl()->L, 1);

        if (m_parentVar)
            m_parentVar->popValue();
    }

    lua_simple::VariableRef VariableRef::getVariableByKey( const mkString& key )
    {
        return VariableRef(key, m_ownerContext, this);
    }


    void FunctionCallArgs::add( const mkString& val )
    {
        FunctionCallArg arg;
        arg.str_value = val;
        arg.is_number = false;

        args.push_back(arg);
    }

    void FunctionCallArgs::add( int val )
    {
        FunctionCallArg arg;
        arg.number_value = (float)val;
        arg.is_number = true;

        args.push_back(arg);
    }

    void FunctionCallArgs::add( float val )
    {
        FunctionCallArg arg;
        arg.number_value = val;
        arg.is_number = true;

        args.push_back(arg);
    }    

    void FunctionCallArgs::add( const ObjectRefBase& val )
    {
        FunctionCallArg arg;
        arg.object_ref = val;
        arg.is_number = false;

        args.push_back(arg);
    }

    void* LuaSimpleContext::createUserData( size_t size )
    {
        return lua_newuserdata(m_pimpl->L, size);
    }

    void LuaSimpleContext::registerFuncImpl( const char* name, lua_CFunction proxy_fun_ptr )
    {
        int index = LUA_GLOBALSINDEX;
        if (isRegisteringInNamespace())
        {
            // Lua stack after this lua_getglobal:
            // [-1] table to register in
            // [-2] upvalue for c closure
            // we need to swap them, so lua_pushcclosure can proceed:
            // [-1] upvalue for c closure
            // [-2] table to register in
            // After lua_pushcclosure, stack will look like this:
            // [-1] c closure
            // [-2] table to register in
            lua_getglobal(_getLuaState(), m_currentRegistrationNamespace.c_str());
            lua_insert(_getLuaState(), -2);

            index = -2;
        }

        lua_pushcclosure(_getLuaState(), proxy_fun_ptr, 1);

        MK_ASSERT(lua_istable(_getLuaState(), index) || index == LUA_GLOBALSINDEX);
        lua_setfield(_getLuaState(), index, name);

        if (isRegisteringInNamespace())
            lua_pop(_getLuaState(), 1);
    }

    // TODO: checking value types before lua_to*

    int LuaSimpleContext::CallFunction( lua_State* L, int (*ptr)(int) )
    {
        int arg1 = lua_tointeger(L, 1);
        int ret_val = ptr(arg1);
        lua_pushinteger(L, ret_val);

        // return number of output values
        return 1;
    }

    int LuaSimpleContext::CallFunction( lua_State* L, int (*ptr)(int, int) )
    {
        int arg1 = lua_tointeger(L, 1);
        int arg2 = lua_tointeger(L, 2);
        int ret_val = ptr(arg1, arg2);
        lua_pushinteger(L, ret_val);

        // return number of output values
        return 1;
    }

    int LuaSimpleContext::CallFunction( lua_State* L, void (*ptr)(int) )
    {
        ptr(lua_tointeger(L, 1));

        return 0;
    }

    int LuaSimpleContext::CallFunction( lua_State* L, float (*ptr)() )
    {
        float ret_val = ptr();
        lua_pushnumber(L, ret_val);

        return 1;
    }

    int LuaSimpleContext::CallFunction( lua_State* L, void (*ptr)(const mkString&) )
    {
        const char* str = lua_tostring(L, 1);
        ptr(mkString(str));

        return 0;
    }

    int LuaSimpleContext::CallFunction( lua_State* L, ObjectRefBase (*ptr)() )
    {
        ObjectRefBase* obj = pushNewLuaObject<ObjectRefBase>(L);
        *obj = ptr();

        return 1;
    }

    int LuaSimpleContext::CallFunction( lua_State* L, void (*ptr)(TGameObjectVec&, const mkVec3&, float) )
    {
        TGameObjectVec out_vec;

        mkVec3* arg1 = getVec3FromParam(L, 1);
        float arg2 = (float)lua_tonumber(L, 2);

        ptr(out_vec, *arg1, arg2);

        lua_newtable(L);

        for (size_t i = 0; i < out_vec.size(); ++i)
        {
            lua_pushnumber(L, i+1);
            ObjectRefBase* obj = pushNewLuaObject<ObjectRefBase>(L);
            *obj = ObjectRefBase(out_vec[i]);
            lua_rawset(L, -3);
        }

        return 1;
    }

    int LuaSimpleContext::CallFunction( lua_State* L, void (*ptr)(GameObject*, const ObjectRefBase&, int, float, const mkVec3&, const mkVec3&) )
    {
        GameObject* arg1 = (GameObject*)(lua_topointer(L, 1));
        ObjectRefBase* arg2 = tryGetLuaObject<ObjectRefBase>(L, 2);
        int arg3 = (int)lua_tonumber(L, 3);
        float arg4 = (float)lua_tonumber(L, 4);
        mkVec3* arg5 = getVec3FromParam(L, 5);
        mkVec3* arg6 = getVec3FromParam(L, 6);

        ptr(arg1, *arg2, arg3, arg4, *arg5, *arg6);

        return 0;
    }

    int LuaSimpleContext::CallFunction( lua_State* L, mkVec3 (*ptr)() )
    {
        mkVec3 result = ptr();

        *pushNewVec3(L) = result;

        return 1;
    }

    int LuaSimpleContext::CallFunctionVoidPtr( lua_State* L, void* (*ptr)() )
    {
        void* result = ptr();
        lua_pushlightuserdata(L, result);

        return 1;
    }

    int LuaSimpleContext::CallFunctionVoidPtr( lua_State* L, void (*ptr)(void*) )
    {
        void* arg1 = (void*)lua_topointer(L, 1);
        ptr(arg1);

        return 0;
    }

    int LuaSimpleContext::CallFunctionVoidPtr( lua_State* L, mkString (*ptr)(void*) )
    {
        void* arg1 = (void*)lua_topointer(L, 1);
        mkString result = ptr(arg1);
        lua_pushstring(L, result.c_str());

        return 1;
    }

    int LuaSimpleContext::CallFunctionVoidPtr( lua_State* L, float (*ptr)(void*) )
    {
        void* arg1 = (void*)lua_topointer(L, 1);
        float result = ptr(arg1);
        lua_pushnumber(L, result);

        return 1;
    }

    int LuaSimpleContext::CallFunctionVoidPtr( lua_State* L, int32 (*ptr)(void*) )
    {
        void* arg1 = (void*)lua_topointer(L, 1);
        int32 result = ptr(arg1);
        lua_pushinteger(L, result);

        return 1;
    }

    int LuaSimpleContext::CallFunctionVoidPtr( lua_State* L, ObjectRefBase (*ptr)(void*, const mkString&) )
    {
        void* arg1 = (void*)lua_topointer(L, 1);
        mkString arg2 = lua_tostring(L, 2);

        ObjectRefBase* result = pushNewLuaObject<ObjectRefBase>(L);
        *result = ptr(arg1, arg2);

        return 1;
    }

    int LuaSimpleContext::CallFunctionVoidPtr( lua_State* L, ObjectRefBase (*ptr)(void*, const mkString&, const mkString&) )
    {
        void* arg1 = (void*)lua_topointer(L, 1);
        mkString arg2 = lua_tostring(L, 2);
        mkString arg3 = lua_tostring(L, 3);

        ObjectRefBase* result = pushNewLuaObject<ObjectRefBase>(L);
        *result = ptr(arg1, arg2, arg3);

        return 1;
    }

    int LuaSimpleContext::CallFunctionVoidPtr( lua_State* L, void (*ptr)(void*, const mkVec3&) )
    {
        void* arg1 = (void*)lua_topointer(L, 1);
        mkVec3* arg2 = getVec3FromParam(L, 2);
        ptr(arg1, *arg2);

        return 0;
    }

    int LuaSimpleContext::CallFunctionVoidPtr( lua_State* L, mkVec3 (*ptr)(void*) )
    {
        void* arg1 = (void*)lua_topointer(L, 1);
        mkVec3* result = pushNewVec3(L);
        *result = ptr(arg1);

        return 1;
    }

    int LuaSimpleContext::CallFunctionVoidPtr( lua_State* L, bool (*ptr)(void*, const mkString&) )
    {
        void* arg1 = (void*)lua_topointer(L, 1);
        mkString arg2 = lua_tostring(L, 2);

        bool result = ptr(arg1, arg2);

        lua_pushboolean(L, result);

        return 1;
    }

    int LuaSimpleContext::CallFunctionVoidPtr( lua_State* L, void (*ptr)(void*, float) )
    {
        void* arg1 = (void*)lua_topointer(L, 1);
        float arg2 = (float)lua_tonumber(L, 2);

        ptr(arg1, arg2);

        return 0;
    }

    int LuaSimpleContext::CallFunctionVoidPtr( lua_State* L, void (*ptr)(void*, bool) )
    {
        void* arg1 = (void*)lua_topointer(L, 1);
        bool arg2 = lua_toboolean(L, 2) != 0;

        ptr(arg1, arg2);

        return 0;
    }

    int LuaSimpleContext::CallFunctionVoidPtr( lua_State* L, void (*ptr)(void*, const mkString&) )
    {
        void* arg1 = (void*)lua_topointer(L, 1);
        const char* arg2 = lua_tostring(L, 2);

        ptr(arg1, arg2);

        return 0;
    }

    int LuaSimpleContext::CallFunctionVoidPtr( lua_State* L, void (*ptr)(void*, const mkQuat&) )
    {
        void* arg1 = (void*)lua_topointer(L, 1);
        mkQuat* arg2 = tryGetLuaObject<mkQuat>(L, 2);

        ptr(arg1, *arg2);

        return 0;
    }

    int LuaSimpleContext::CallFunctionVoidPtr( lua_State* L, mkQuat (*ptr)(void*) )
    {
        void* arg1 = (void*)lua_topointer(L, 1);
        mkQuat* result = pushNewLuaObject<mkQuat>(L);
        *result = ptr(arg1);

        return 1;
    }

    void* LuaSimpleContext::getUpValueUserData( lua_State* L, int idx )
    {
        int val_idx = lua_upvalueindex(idx);
        return lua_touserdata(L, val_idx);
    }

    lua_State* LuaSimpleContext::_getLuaState() const
    {
        return m_pimpl->L;
    }

    void LuaSimpleContext::registerType_Math()
    {
        registerAllMath(this);
    }

    void LuaSimpleContext::registerType_GameObjectReference()
    {
        registerObjectRef(this);
    }

    void LuaSimpleContext::registerRttiBinding()
    {
        registerRttiFieldsAccess(this);
    }

    void LuaSimpleContext::setFunctionRegistrationNamespace( const char* name )
    {
        m_currentRegistrationNamespace = name;

        if (!m_currentRegistrationNamespace.empty())
            ensureTableExists(m_currentRegistrationNamespace.c_str());
    }

    bool LuaSimpleContext::isRegisteringInNamespace() const
    {
        return !m_currentRegistrationNamespace.empty();
    }

    void LuaSimpleContext::registerTypeMetadata( const rtti::TypeInfo* type )
    {
        ensureTableExists(type->getClassName());

        lua_getglobal(_getLuaState(), type->getClassName());

        lua_pushstring(_getLuaState(), type->getClassName());
        lua_setfield(_getLuaState(), -2, "ClassName");

        lua_pushlightuserdata(_getLuaState(), (void*)type);
        lua_setfield(_getLuaState(), -2, "ClassInfo");

        lua_pop(_getLuaState(), 1);
    }

    void LuaSimpleContext::ensureTableExists( const char* name )
    {
        lua_getglobal(_getLuaState(), name);
        if (lua_isnil(_getLuaState(), -1))
        {
            lua_pop(_getLuaState(), 1);
            lua_newtable(_getLuaState());
            lua_setglobal(_getLuaState(), name);
        }
        else
        {
            lua_pop(_getLuaState(), 1);
        }
    }
}
