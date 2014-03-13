#pragma once
#include "GameObject.h"
#include "ObjectRef.h"
#include "rtti/TypeInfo.h"

extern "C" {
    struct lua_State;
    typedef int (*lua_CFunction)(lua_State*);
}

class ObjectRefBase;

namespace lua_simple
{

    class LuaSimpleContext;

    // Reference for variable in Lua, can be read and written.
    // Invalidates quickly - every Lua function call in owner context,
    // or any write to ancestor references changes validation counter,
    // which is checked before any method call. It is best to read/write value
    // after getting the reference, as opposed to caching
    class VariableRef
    {
    public:
        VariableRef(const mkString& var_name, LuaSimpleContext* context, VariableRef* parent_var = NULL);

        bool readInt32(int32& out) const;
        bool readUInt32(uint32& out) const;
        bool readFloat(float& out) const;
        bool readString(mkString& out) const;
        bool readBytes(size_t num, uint8* out_buf) const;

        // TODO: writing! not implemented yet
        bool writeInt32(int32 val);
        bool writeUInt32(uint32 val);
        bool writeFloat(float val);
        bool writeString(const mkString& val);
        bool writeBytes(size_t num, uint8* val_buf);

        VariableRef getVariableByKey(const mkString& key);

    private:
        mkString m_varName;
        const int m_validationIdx;
        LuaSimpleContext* m_ownerContext;
        VariableRef* m_parentVar;

        void pushValue() const;
        void popValue() const;

        bool validate() const;
    };

    struct LuaSimpleContextPimpl;

    struct FunctionCallArg
    {
        mkString str_value;
        float number_value;
        ObjectRefBase object_ref;

        bool is_number;
    };

    struct FunctionCallArgs
    {
        typedef std::vector<FunctionCallArg> TArgsVec;
        TArgsVec args;

        void add(const mkString& val);
        void add(int val);
        void add(float val);
        void add(const ObjectRefBase& val);
    };

// Context for running simple Lua scripts. Access from C++ is restricted to calling functions
// and reading global variables. Script can do anything, but canonical form would be:
//  1. Defining global values (can be used as configuration/data definition files)
//  2. Defining global functions (can be used as event handlers)
// This is enough to do some basic scripting and does not require using heavy lua-C++ binding libraries
class LuaSimpleContext : public IScriptingContext
{
private:
    // no copying of contexts
    LuaSimpleContext(const LuaSimpleContext&);
    LuaSimpleContext& operator=(const LuaSimpleContext&);

public:
    explicit LuaSimpleContext(const mkString& name = "UNKNOWN");
    ~LuaSimpleContext();

    bool runString(const char* code);
    bool runFile(const mkString& filename);

    void tryCallFunc(const char* name);

    template <typename T1>
    bool callFuncArgs(const char* name, T1 arg1)
    {
        FunctionCallArgs args;
        args.add(arg1);

        return callFunc(name, args, true);
    }

    bool callFunc(const char* name, bool ignore_non_existent = false);
    bool callFunc(const char* name, const FunctionCallArgs& args, bool ignore_non_existent = false);

    VariableRef getGlobalVariable(const char* name);

    // Mainly for setting "this" pointer in GameObject scripts
    // Overrides any previous user pointer with the same name in this context
    // Sets pointer as Lua light userdata, so no GC is involved
    void setGlobalUserPointer(const char* name, void* ptr);
    
    LuaSimpleContextPimpl* getPimpl() const;

    mkString getName() const;

    void registerType_Math();
    void registerType_GameObjectReference();

    void registerRttiBinding();

    // All future calls to registerFunction will place them in given namespace (table)
    // If empty or NULL (as is by default), registered functions will be placed in global table
    void setFunctionRegistrationNamespace(const char* name);
    bool isRegisteringInNamespace() const;

    virtual void registerTypeMetadata(const rtti::TypeInfo* type);

private:
    LuaSimpleContextPimpl* m_pimpl;
    mkString m_name;

private:
    mkString m_currentRegistrationNamespace;

    void ensureTableExists(const char* name);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Function registering
    // 
    // To register a function in this context, use "registerFunc" method (template method generated with macros :o)
    // like this:
    //   int myFunc(int arg1, float arg2);
    //   myLuaContext.registerFunc("my_func", myFunc);
    // so it can be used in Lua code:
    //   function onUpdate(dt)
    //       my_func(2, 3.14)
    //   end
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:

    // Add new function callers here if necessary
    // Caller returns number of return values, usually 1 (0 for functions with return type void)

    static int CallFunction(lua_State* L, int (*ptr)(int));
    static int CallFunction(lua_State* L, int (*ptr)(int, int));
    static int CallFunction(lua_State* L, void (*ptr)(int));
    static int CallFunction(lua_State* L, float (*ptr)());
    static int CallFunction(lua_State* L, void (*ptr)(const mkString&));
    static int CallFunction(lua_State* L, ObjectRefBase (*ptr)());
    static int CallFunction(lua_State* L, void (*ptr)(TGameObjectVec&, const mkVec3&, float));
    static int CallFunction(lua_State* L, void (*ptr)(GameObject*, const ObjectRefBase&, int, float, const mkVec3&, const mkVec3&));
    static int CallFunction(lua_State* L, mkVec3 (*ptr)());
   
    template <typename T> static int CallFunction(lua_State* L, T* (*ptr)())
    {
        return CallFunctionVoidPtr(L, (void* (*)())ptr);
    }

    template <typename RetType, typename ArgType>
    static int CallFunction(lua_State* L, RetType (*ptr)(ArgType*))
    {
        return CallFunctionVoidPtr(L, (RetType (*)(void*))ptr);
    }

    template <typename RetType, typename ArgType, typename T1>
    static int CallFunction(lua_State* L, RetType (*ptr)(ArgType*, T1))
    {
        return CallFunctionVoidPtr(L, (RetType (*)(void*, T1))ptr);
    }

    template <typename RetType, typename ArgType, typename T1, typename T2>
    static int CallFunction(lua_State* L, RetType (*ptr)(ArgType*, T1, T2))
    {
        return CallFunctionVoidPtr(L, (RetType (*)(void*, T1, T2))ptr);
    }

    template <typename RetType, typename ArgType, typename T1>
    static int CallFunction(lua_State* L, RetType* (*ptr)(ArgType*, T1))
    {
        return CallFunctionVoidPtr(L, (void* (*)(void*, T1))ptr);
    }

    template <typename RetType, typename ArgType, typename T1, typename T2>
    static int CallFunction(lua_State* L, RetType* (*ptr)(ArgType*, T1, T2))
    {
        return CallFunctionVoidPtr(L, (void* (*)(void*, T1, T2))ptr);
    }

    static int CallFunctionVoidPtr(lua_State* L, void* (*ptr)());
    static int CallFunctionVoidPtr(lua_State* L, void (*ptr)(void*));
    static int CallFunctionVoidPtr(lua_State* L, float (*ptr)(void*));
    static int CallFunctionVoidPtr(lua_State* L, int32 (*ptr)(void*));
    static int CallFunctionVoidPtr(lua_State* L, mkString (*ptr)(void*));
    static int CallFunctionVoidPtr(lua_State* L, mkVec3 (*ptr)(void*));
    static int CallFunctionVoidPtr(lua_State* L, void (*ptr)(void*, const mkVec3&));
    static int CallFunctionVoidPtr(lua_State* L, void (*ptr)(void*, const mkString&));
    static int CallFunctionVoidPtr(lua_State* L, ObjectRefBase (*ptr)(void*, const mkString&));
    static int CallFunctionVoidPtr(lua_State* L, ObjectRefBase (*ptr)(void*, const mkString&, const mkString&));
    static int CallFunctionVoidPtr(lua_State* L, bool (*ptr)(void*, const mkString&));
    static int CallFunctionVoidPtr(lua_State* L, void (*ptr)(void*, float));
    static int CallFunctionVoidPtr(lua_State* L, void (*ptr)(void*, bool));
    static int CallFunctionVoidPtr(lua_State* L, void (*ptr)(void*, const mkQuat&));
    static int CallFunctionVoidPtr(lua_State* L, mkQuat (*ptr)(void*));

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Low-level implementation part
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Special case for void (*)(void)
    static int RegisteredFunctionVoid(lua_State* L)
    {
        typedef void (*FunPtr)();
        FunPtr* fun_ptr_ptr = (FunPtr*)getUpValueUserData(L, 1);
        FunPtr function = *fun_ptr_ptr;
        function();
        return 0;
    }

    // Special case for T (*)(void)
    template <typename RT> static int RegisteredFunctionNoArgs(lua_State* L)
    {
        typedef RT (*FunPtr)();
        FunPtr* fun_ptr_ptr = (FunPtr*)getUpValueUserData(L, 1);
        FunPtr function = *fun_ptr_ptr;
        return CallFunction(L, function);
    }

#define REG_FUN_IMPL(...) \
    template <typename RT, ##__VA_ARGS__> static int RegisteredFunction(lua_State* L) { \
        typedef RT (*FunPtr)(##__VA_ARGS__); \
        FunPtr* fun_ptr_ptr = (FunPtr*)getUpValueUserData(L, 1); \
        FunPtr function = *fun_ptr_ptr; \
        return CallFunction(L, function); \
    }

#define REGISTER_FUN(...) \
    REG_FUN_IMPL(__VA_ARGS__); \
    template <typename RT, ##__VA_ARGS__> \
    void registerFunc(const char* name, RT (*ptr)(##__VA_ARGS__)) { \
        typedef typename RT (*FunPtr)(##__VA_ARGS__); \
        FunPtr* fun_ptr = (FunPtr*)createUserData(sizeof(FunPtr)); \
        *fun_ptr = ptr; \
        registerFuncImpl(name, RegisteredFunction<RT, ##__VA_ARGS__>); \
    }

public:
    // Special case for void (*)(void)
    void registerFunc(const char* name, void (*ptr)())
    {
        typedef void (*FunPtr)();
        FunPtr* fun_ptr = (FunPtr*)createUserData(sizeof(FunPtr));
        *fun_ptr = ptr;
        registerFuncImpl(name, RegisteredFunctionVoid);
    }

    // Special case for T (*)(void)
    template <typename T>
    void registerFunc(const char* name, T (*ptr)())
    {
        typedef T (*FunPtr)();
        FunPtr* fun_ptr = (FunPtr*)createUserData(sizeof(FunPtr));
        *fun_ptr = ptr;
        registerFuncImpl(name, RegisteredFunctionNoArgs<T>);
    }

    REGISTER_FUN(typename T1);
    REGISTER_FUN(typename T1, typename T2);
    REGISTER_FUN(typename T1, typename T2, typename T3);
    REGISTER_FUN(typename T1, typename T2, typename T3, typename T4);
    REGISTER_FUN(typename T1, typename T2, typename T3, typename T4, typename T5);
    REGISTER_FUN(typename T1, typename T2, typename T3, typename T4, typename T5, typename T6);

private:
    void* createUserData(size_t size);
    void registerFuncImpl(const char* name, lua_CFunction proxy_fun_ptr);

    static void* getUpValueUserData(lua_State* L, int idx);

public:
    // Only for registering types in scripts, forbidden outside scripting framework impl
    lua_State* _getLuaState() const;
};

}

#define EXPORT_VOID_METHOD_SCRIPT(clazz, name) static void _script_##clazz##name(clazz* ptr) { ptr->name(); }
#define EXPORT_NOARG_METHOD_SCRIPT(clazz, name, ret_type) static ret_type _script_##clazz##name(clazz* ptr) { return ptr->name(); }
#define EXPORT_VOID_ARG_METHOD_SCRIPT(clazz, name, arg_type) static void _script_##clazz##name(clazz* ptr, arg_type a) { ptr->name(a); }

#define VOID_METHOD_SCRIPT(clazz, name) (&_script_##clazz##name)

#define START_SCRIPT_REGISTRATION(clazz, ctx)                                       \
    void clazz::registerTypeInScriptContext(IScriptingContext* _context_param) {    \
        MK_ASSERT(dynamic_cast<lua_simple::LuaSimpleContext*>(_context_param)       \
                == static_cast<lua_simple::LuaSimpleContext*>(_context_param));     \
        lua_simple::LuaSimpleContext* ctx = static_cast<lua_simple::LuaSimpleContext*>(_context_param); \
        ctx->setFunctionRegistrationNamespace(#clazz);

#define END_SCRIPT_REGISTRATION(ctx) ctx->setFunctionRegistrationNamespace(""); }
