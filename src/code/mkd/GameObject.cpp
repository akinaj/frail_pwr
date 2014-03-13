#include "pch.h"
#include "GameObject.h"
#include "Game.h"
#include "Level.h"
#include "scripting/LuaSimpleBinding.h"
#include "rtti/TypeManager.h"
#include "Filesystem.h"

EFrameEvent::HandlerType EFrameEvent::getHandlerMethodPtr( TYPE frame_event )
{
    switch (frame_event)
    {
    case EFrameEvent::Render:       return &GameObject::onRender;
    case EFrameEvent::LogicUpdate:  return &GameObject::onUpdate;
    }

    MK_ASSERT_MSG(false, "Undefined frame event handler method for event '%d'!", (int)frame_event);
    return EFrameEvent::HandlerType();
}

IMPLEMENT_RTTI_SCRIPTED(GameObject, rtti::IRTTIObject);

START_RTTI_INIT(GameObject);
{
    FIELD_STRING(Name);
    FIELD_STRING(ScriptName);
}
END_RTTI_INIT();

// Script binding initialization is done in GameObjectScriptBinding.cpp

GameObject::GameObject()
    : m_parentLevel(NULL)
    , m_isQueuedForDestruction(false)
    , m_isDestroyed(false)
    , m_scriptContext(NULL)
{

}

float GameObject::getTimeDelta() const
{
    return g_game->getTimeDelta();
}

float GameObject::getTimeMs() const
{
    return g_game->getTimeMs();
}

Level* GameObject::getLevel() const
{
    MK_ASSERT(m_parentLevel);

    return m_parentLevel;
}

void GameObject::destroy()
{
    MK_ASSERT_MSG(!m_isQueuedForDestruction, "Attempting to destroy object '%s' of class '%s' more than once", getName().c_str(), getTypeInfo()->getClassName());

    getLevel()->queueGameObjectDestruction(this);
    m_isQueuedForDestruction = true;
}

void GameObject::onDestroy()
{
    // Clearing event handlers is done in LevelObjectsMgr

    callScript_onDestroy();

    if (m_scriptContext)
    {
        delete m_scriptContext;
        m_scriptContext = NULL;
    }

    MK_ASSERT_MSG(!m_isDestroyed, "onDestroy called twice - object probably queued for destruction multiple times");
    MK_ASSERT_MSG(getLevel()->isInDestruction() || m_isQueuedForDestruction, "onDestroy called without being marked as queued for destruction!");

    m_isDestroyed = true;
}

void GameObject::_setLevel(Level* level)
{
    MK_ASSERT(m_parentLevel == NULL);

    m_parentLevel = level;
}

void GameObject::initStandaloneObject()
{
    onCreate();
    onPostCreate();
}

rtti::IObjectProvider* GameObject::getOwningObjectProvider() const
{
    return m_parentLevel;
}

void GameObject::onCreate()
{
    if (Name.empty())
    {
        Name = getTypeInfo()->getClassName();
        Name += "_";
        Name += int2str(getObjectId());
    }

    // For now, just subscribe all GameObjects to both render and logic update frame events
    // Consider leaving that to specific classes
    registerFrameEventHandler(EFrameEvent::Render);
    registerFrameEventHandler(EFrameEvent::LogicUpdate);
}

void GameObject::onPostCreate()
{
    if (!ScriptName.empty())
    {
        m_scriptContext = new lua_simple::LuaSimpleContext(getName() + "_LUA");
        rtti::TypeManager::getInstance().registerTypesInScript(m_scriptContext);
        onRegisterInScripts(m_scriptContext);

        runGameObjectCommonScript();

        if (!m_scriptContext->runFile("data/scripts/" + ScriptName))
        {
            log_error("Error when loading script '%s' of object '%s' of class '%s', scripted event handlers for this object will not be executed",
                ScriptName.c_str(), getName().c_str(), getTypeInfo()->getClassName());
            delete m_scriptContext;
            m_scriptContext = NULL;
            ScriptName.clear();
        }
    }

    callScript_onPostCreate();
}

void GameObject::onUpdate()
{
    callScript_onUpdate();
}

const mkString& GameObject::getName() const
{
    return Name;
}

void GameObject::onRegisterInScripts( lua_simple::LuaSimpleContext* ctx )
{
    ctx->registerType_Math();
    ctx->registerType_GameObjectReference();
    ctx->registerRttiBinding();

    ctx->setGlobalUserPointer("this", this);
}

void GameObject::callScript_onPostCreate()
{
    if (!getScriptContext())
        return;

    getScriptContext()->tryCallFunc("onPostCreate");
}

void GameObject::callScript_onDestroy()
{
    if (!getScriptContext())
        return;

    // Special case - we don't call script onDestroy when whole level is destroyed
    // Rationale: some logic in scripted onDestroy (mostly creating other objects)
    // will fail when called during level destruction. When level is destroyed,
    // all game objects will be deleted anyway, so not calling scripted onDestroy
    // should not do much harm.
    // TODO: try to think of something less dirty
    if (getLevel() && getLevel()->isInDestruction())
        return;

    getScriptContext()->tryCallFunc("onDestroy");
}

void GameObject::callScript_onUpdate()
{
    if (!getScriptContext())
        return;

    getScriptContext()->tryCallFunc("onUpdate");
}

lua_simple::LuaSimpleContext* GameObject::getScriptContext() const
{
    return m_scriptContext;
}

void GameObject::runGameObjectCommonScript()
{
    const rtti::TypeInfo* obj_type = getTypeInfo();
    const rtti::TypeInfo* end_type = GameObject::Type.getSuperClass();

    const rtti::TypeInfo* type_it = obj_type;
    while (type_it != end_type)
    {
        runCommonScriptForType(type_it->getClassName());
        type_it = type_it->getSuperClass();
    }
}

void GameObject::runCommonScriptForType( const char* type_name )
{
    mkString script_filename = "data/scripts/std/std_";
    script_filename += type_name;
    script_filename += ".lua";

    if (fileExists(script_filename.c_str()))
        m_scriptContext->runFile(script_filename);
}

void GameObject::setName( const mkString& new_name )
{
    Name = new_name;
}

void GameObject::registerFrameEventHandler( EFrameEvent::TYPE handler )
{
    getLevel()->subscribeToFrameEvent(handler, this);
}

void GameObject::unregisterFrameEventHandler( EFrameEvent::TYPE handler )
{
    getLevel()->unsubscribeFromFrameEvent(handler, this);
}

void GameObject::takeDamage( const SDamageInfo& dmg_info )
{
    onTakeDamage(dmg_info);
}

void GameObject::_callScriptMethod( const mkString& method_name ) const
{
    if (getScriptContext())
        getScriptContext()->tryCallFunc(method_name.c_str());
}
