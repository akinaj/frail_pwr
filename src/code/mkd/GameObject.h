/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "rtti/TypeInfo.h"
#include "DamageType.h"

class GameObject;

namespace EFrameEvent
{
    enum TYPE
    {
        Render,
        LogicUpdate,

        _COUNT
    };

    typedef void (GameObject::*HandlerType)();

    HandlerType getHandlerMethodPtr(EFrameEvent::TYPE frame_event);
}

struct SDamageInfo
{
    EDamageType::TYPE type;
    float dmg;
    mkVec3 dir;
    mkVec3 pos;


    SDamageInfo(EDamageType::TYPE _type, float _dmg, const mkVec3& dir = mkVec3::UNIT_Z, const mkVec3& pos = mkVec3::ZERO)
        : type(_type)
        , dmg(_dmg)
        , dir(dir)
        , pos(pos)
    {

    }
};

class Level;

// TODO: consider using some sort of IScriptContext, we don't really need to know about Lua here
namespace lua_simple { class LuaSimpleContext; }

/// Base class for gameplay objects. Game objects can:
///  - be created with class name
///  - be serialized
///  - register for frame updates
///  - be deleted on demand
/// All game objects are associated with Level on which they exist.
/// They are destroyed with parent level, no need to explicitly delete them.
class GameObject : public rtti::IRTTIObject
{
    DECLARE_RTTI(GameObject);

public:
    GameObject();

    // Calls onCreate and onPostCreate for objects created after loading level
    void initStandaloneObject();

    // Note that all pointer fields in deserialized GameObjects can be invalid in onCreate (if needed, use onPostCreate)
    virtual void onCreate();

    // Called after all deserialized objects are created, pointer fields are valid
    virtual void onPostCreate();

    // Register here all functions available ONLY for scripted objects of this class and derived
    // Functions operating on this type (like exposed methods, getters etc) should be registered using RTTI
    // Note its only called in onPostCreate for objects with defined non-empty script name
    virtual void onRegisterInScripts(lua_simple::LuaSimpleContext* ctx);

    virtual void onDestroy();

    virtual void onUpdate();
    virtual void onRender() { }

    float getTimeDelta() const;
    float getTimeMs() const;

    rtti::IObjectProvider* getOwningObjectProvider() const;
    Level* getLevel() const;

    /// Queues object for destruction which can be delayed,
    /// and is guaranteed to happen no sooner then next frame
    /// onDestroy handler will be called right before destruction
    void destroy();

    const mkString& getName() const;

    void takeDamage(const SDamageInfo& dmg_info);

    virtual void onTakeDamage(const SDamageInfo& dmg_info) { }

    // Just a workaround for missing messaging system :(
    // TODO fix it after aisandbox
    void _callScriptMethod(const mkString& method_name) const;

    // Overrides object's script name. Note it will only have effect when called before
    // script context creation (which is done in onPostCreate)
    void setScriptName(const mkString& script_name);

protected:
    lua_simple::LuaSimpleContext* getScriptContext() const;

    void setName(const mkString& new_name);

    void registerFrameEventHandler(EFrameEvent::TYPE handler);
    void unregisterFrameEventHandler(EFrameEvent::TYPE handler);

private:
    // Calling script event handlers
    // TODO consider checking for existing handlers in script when it's loaded,
    // instead of trying to call handler every time
    void callScript_onUpdate();
    void callScript_onPostCreate();
    void callScript_onDestroy();

private:
    // For factory use only
    void _setLevel(Level* level);
    friend class LevelObjectsMgr;

    Level* m_parentLevel;
    bool m_isQueuedForDestruction;
    bool m_isDestroyed;

    // Name that can be used to reference this object from scripts
    // Note: it's not guaranteed to be unique.
    // Should never be used to reference other objects in code.
    // Preferred way to use in scripts is to find object by name in onPostCreate
    // and store ObjectRef to it for future usage
    mkString Name;

    // Scripting context - created in onPostCreate for objects that have non-empty ScriptName
    // When created, onRegisterInScripts is called
    lua_simple::LuaSimpleContext* m_scriptContext;

    // Filename of script file containing event handlers for specific instance of game object
    // Name does not follow naming convention, so it will not be shadowed in subclasses,
    // also stands out visually in preset files
    mkString ScriptName;

    // Runs scripts from data/scripts/std for object's class and all superclasses
    void runGameObjectCommonScript();
    void runCommonScriptForType(const char* type_name);
};

typedef std::vector<GameObject*> TGameObjectVec;
