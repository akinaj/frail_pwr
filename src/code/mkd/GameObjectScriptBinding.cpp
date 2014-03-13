#include "pch.h"
#include "GameObject.h"
#include "scripting/LuaSimpleBinding.h"
#include "Player.h"
#include "ObjectRef.h"
#include "rtti/TypeManager.h"
#include "Game.h"
#include "Level.h"

static void _script_logError(const mkString& msg)
{
    log_error("%s", msg.c_str());
}

static void _script_logInfo(const mkString& msg)
{
    log_info("%s", msg.c_str());
}

static void _script_logWarning(const mkString& msg)
{
    log_warning("%s", msg.c_str());
}

static ObjectRefBase _script_getPlayer()
{
    return ObjectRefBase(g_game->getCurrentLevel()->getPlayer());
}

static ObjectRefBase _script_createObject(GameObject* this_ptr, const mkString& class_name, const mkString& preset_name)
{
    const rtti::TypeInfo* type = rtti::TypeManager::getInstance().getTypeInfoByName(class_name);
    MK_ASSERT_MSG(type, "Script of object '%s' tried to create object of nonexistent type '%s' with preset '%s'",
        this_ptr->getName().c_str(), class_name.c_str(), preset_name.c_str());
    if (!type)
        return NULL;

    GameObject* result = this_ptr->getLevel()->createObject(type, true, preset_name);
    return ObjectRefBase(result);
}

static ObjectRefBase _script_findObject(GameObject* this_ptr, const mkString& class_name, const mkString& object_name)
{
    GameObject* result = NULL;

    if (class_name.empty())
    {
        result = this_ptr->getLevel()->findObjectByName(object_name);
    }
    else
    {
        const rtti::TypeInfo* type = rtti::TypeManager::getInstance().getTypeInfoByName(class_name);
        MK_ASSERT_MSG(type, "Script of object '%s' tried to find object '%s' of nonexistent type '%s'",
            this_ptr->getName().c_str(), object_name.c_str(), class_name.c_str());
        result = this_ptr->getLevel()->findObjectByName(object_name, type);
    }

    return ObjectRefBase(result);
}

static void _script_findObjectsInRadius(TGameObjectVec& out_vec, const mkVec3& search_origin, const float radius)
{
    g_game->getCurrentLevel()->findObjectsInRadius(out_vec, &GameObject::Type, search_origin, radius);
}

static void _script_giveDamage(GameObject* this_ptr, const ObjectRefBase& victim, int dmg_type, float dmg, const mkVec3& dir, const mkVec3& pos)
{
    SDamageInfo damage_info((EDamageType::TYPE)dmg_type, dmg, dir, pos);

    GameObject* victim_ptr = victim.fetchBasePtr();
    if (victim_ptr)
        victim_ptr->takeDamage(damage_info);
}

static bool _script_isDerivedOrExact(GameObject* this_ptr, const mkString& from_class_name)
{
    rtti::TypeInfo* from_clazz = rtti::TypeManager::getInstance().getTypeInfoByName(from_class_name);
    if (from_clazz == NULL)
        return false;

    return this_ptr->getTypeInfo()->isDerivedOrExact(from_clazz);
}

static mkVec3 _script_getCurrCameraForwardDir()
{
    ICamera* cam = g_game->getCamera();
    if (!cam)
        return mkVec3::UNIT_Z;

    return cam->getForwardVec();
}

static mkVec3 _script_getCurrCameraPos()
{
    ICamera* cam = g_game->getCamera();
    if (!cam)
        return mkVec3::ZERO;

    return cam->getPosition();
}

EXPORT_VOID_ARG_METHOD_SCRIPT(GameObject, _callScriptMethod, const mkString&);
EXPORT_NOARG_METHOD_SCRIPT(GameObject, getTimeMs, float);
EXPORT_NOARG_METHOD_SCRIPT(GameObject, getTimeDelta, float);
EXPORT_NOARG_METHOD_SCRIPT(GameObject, getName, mkString);
EXPORT_NOARG_METHOD_SCRIPT(GameObject, destroy, void);

START_SCRIPT_REGISTRATION(GameObject, ctx);
{
    ctx->registerFunc("GetTimeMs", VOID_METHOD_SCRIPT(GameObject, getTimeMs));
    ctx->registerFunc("GetTimeDelta", VOID_METHOD_SCRIPT(GameObject, getTimeDelta));
    ctx->registerFunc("GetObjectName", VOID_METHOD_SCRIPT(GameObject, getName));
    ctx->registerFunc("FindObjectOnLevelByClassAndName", _script_findObject); // wrapped for easier use in std_GameObject.lua

    ctx->registerFunc("LogError", _script_logError);
    ctx->registerFunc("LogWarning", _script_logWarning);
    ctx->registerFunc("LogInfo", _script_logInfo);

    ctx->registerFunc("CreateObject", _script_createObject);
    ctx->registerFunc("GetPlayer", _script_getPlayer);

    ctx->registerFunc("FindObjectsInRadius", _script_findObjectsInRadius);
    ctx->registerFunc("GiveDamage", _script_giveDamage);
    ctx->registerFunc("IsDerivedOrExactClass", _script_isDerivedOrExact);
    
    
    ctx->registerFunc("DestroyObject", VOID_METHOD_SCRIPT(GameObject, destroy));
    ctx->registerFunc("CallScriptMethod", VOID_METHOD_SCRIPT(GameObject, _callScriptMethod));

    ctx->registerFunc("GetCurrentCameraForwardDir", _script_getCurrCameraForwardDir);
    ctx->registerFunc("GetCurrentCameraWorldPos", _script_getCurrCameraPos);
    
}
END_SCRIPT_REGISTRATION(ctx);

