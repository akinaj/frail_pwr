#include "pch.h"

#include <lua.hpp>

#include "LuaSimpleRttiBinding.h"
#include "LuaSimpleMathBinding.h"
#include "LuaSimpleObjectRefBinding.h"
#include "LuaSimpleBinding.h"

#include "rtti/TypeInfo.h"
#include "rtti/FieldInfo.h"
#include "ObjectRef.h"

namespace lua_simple
{

    // Parameters:
    //  - IRttiObject* object_ptr (lightuserdata)
    //  - string field_name
    // Returns single value depending on type of field type:
    //  - number for int32, uint32, float and enum fields
    //  - boolean for bool fields
    //  - string for string fields
    //  - mkVec3 for vector fields
    //  - mkQuat for quaternion fields
    //  - ObjectRefBase for RTTIObjectPtr fields (only if is NULL or points to GameObject or derived; otherwise nothing is returned)
    //  - nothing for POD fields (for now there would be no way to use those in script anyway)
    static int getFieldValue(lua_State* L)
    {
        rtti::IRTTIObject* arg_obj = (rtti::IRTTIObject*)lua_topointer(L, 1);
        const char* field_name = lua_tostring(L, 2);

        const rtti::TypeInfo* type_info = arg_obj->getTypeInfo();
        const rtti::FieldInfo* field_info = type_info->getField(field_name);

        MK_ASSERT_MSG(field_info, "Lua script requested value of nonexistent field '%s' of class '%s'",
            field_name, type_info->getClassName());
        if (!field_info)
            return 0;

        switch (field_info->getDataType())
        {
        case rtti::EFieldDataType::Int32:
            lua_pushnumber(L, (lua_Number)field_info->getValueRef<int32>(*arg_obj));
            break;

        case rtti::EFieldDataType::UInt32:
            lua_pushnumber(L, (lua_Number)field_info->getValueRef<uint32>(*arg_obj));
            break;

        case rtti::EFieldDataType::Float:
            lua_pushnumber(L, field_info->getValueRef<float>(*arg_obj));
            break;

        case rtti::EFieldDataType::Enum:
            lua_pushnumber(L, (lua_Number)field_info->getValueRef<int32>(*arg_obj));
            break;

        case rtti::EFieldDataType::Bool:
            lua_pushboolean(L, (int)field_info->getValueRef<bool>(*arg_obj));
            break;

        case rtti::EFieldDataType::String:
            lua_pushstring(L, field_info->getValueRef<mkString>(*arg_obj).c_str());
            break;

        case rtti::EFieldDataType::Vec3:
            *pushNewVec3(L) = field_info->getValueRef<mkVec3>(*arg_obj);
            break;

        case rtti::EFieldDataType::Quaternion:
            *pushNewLuaObject<mkQuat>(L) = field_info->getValueRef<mkQuat>(*arg_obj);
            break;

        case rtti::EFieldDataType::RTTIObjectPtr:
            {
                ObjectRefBase ret_obj_ref(NULL);
                rtti::IRTTIObject* ret_ptr = field_info->getValueRef<rtti::IRTTIObject*>(*arg_obj);
                if (ret_ptr != NULL)
                {
                    if (!ret_ptr->getTypeInfo()->isDerivedOrExact(&GameObject::Type))
                    {
                        log_error("Lua script tried to get value of object pointer field %s::%s "
                            "which does not point to GameObject or descendant. Lua scripts can "
                            "get references only to GameObjects or descendants.",
                            type_info->getClassName(), field_name);
                        return 0;
                    }
                    else
                    {
                        ret_obj_ref.setBasePtr(static_cast<GameObject*>(ret_ptr));
                    }
                }

                pushNewObjectRef(L) = ret_obj_ref;

            }
            break;

        case rtti::EFieldDataType::POD:
            log_warning("Lua script tried to get value of POD field %s::%s",
                type_info->getClassName(), field_name);
            return 0;

        default:
            MK_ASSERT_MSG(false, "Lua object field retrieving: not implemented type %s (%d) - field %s::%s",
                rtti::EFieldDataType::toString(field_info->getDataType()), (int)field_info->getDataType(),
                type_info->getClassName(), field_name);
            return 0;
        }

        return 1;
    }

    static int setFieldValue(lua_State* L)
    {
        rtti::IRTTIObject* arg_obj = (rtti::IRTTIObject*)lua_topointer(L, 1);
        const char* field_name = lua_tostring(L, 2);

        const rtti::TypeInfo* type_info = arg_obj->getTypeInfo();
        const rtti::FieldInfo* field_info = type_info->getField(field_name);

        MK_ASSERT_MSG(field_info, "Lua script requested changing value of nonexistent field '%s' of class '%s'",
            field_name, type_info->getClassName());
        if (!field_info)
            return 0;

        const int arg_idx = 3;

        switch (field_info->getDataType())
        {
        case rtti::EFieldDataType::Int32:
            field_info->setValueInObject<int32>(*arg_obj, (int)lua_tonumber(L, arg_idx));
            break;

        case rtti::EFieldDataType::UInt32:
            field_info->setValueInObject<uint32>(*arg_obj, (uint32)lua_tonumber(L, arg_idx));
            break;

        case rtti::EFieldDataType::Float:
            field_info->setValueInObject<float>(*arg_obj, (float)lua_tonumber(L, arg_idx));
            break;

        case rtti::EFieldDataType::Enum:
            field_info->setValueInObjectFromString(*arg_obj, lua_tostring(L, arg_idx));
            break;

        case rtti::EFieldDataType::Bool:
            field_info->setValueInObject<bool>(*arg_obj, lua_toboolean(L, arg_idx) != 0);
            break;

        case rtti::EFieldDataType::String:
            field_info->setValueInObject<mkString>(*arg_obj, lua_tostring(L, arg_idx));
            break;

        case rtti::EFieldDataType::Vec3:
            field_info->setValueInObject<mkVec3>(*arg_obj, *tryGetLuaObject<mkVec3>(L, arg_idx));
            break;

        case rtti::EFieldDataType::Quaternion:
            field_info->setValueInObject<mkQuat>(*arg_obj, *tryGetLuaObject<mkQuat>(L, arg_idx));
            break;

        case rtti::EFieldDataType::RTTIObjectPtr:
            {
                ObjectRefBase* obj_ref = tryGetLuaObject<ObjectRefBase>(L, arg_idx);
                rtti::IRTTIObject* ptr_to_set = NULL;
                if (obj_ref->isSet())
                    ptr_to_set = obj_ref->fetchBasePtr();

                field_info->setValueInObject<rtti::IRTTIObject*>(*arg_obj, ptr_to_set);
            }
            break;

        case rtti::EFieldDataType::POD:
            log_error("Lua script tried to set value of POD field %s::%s",
                type_info->getClassName(), field_name);
            return 0;

        default:
            MK_ASSERT_MSG(false, "Lua object field setting: not implemented type %s (%d) - field %s::%s",
                rtti::EFieldDataType::toString(field_info->getDataType()), (int)field_info->getDataType(),
                type_info->getClassName(), field_name);
            return 0;
        }

        return 0;
    }

void registerRttiFieldsAccess( LuaSimpleContext* context )
{
    lua_pushcfunction(context->_getLuaState(), getFieldValue);
    lua_setglobal(context->_getLuaState(), "GetFieldValue");

    lua_pushcfunction(context->_getLuaState(), setFieldValue);
    lua_setglobal(context->_getLuaState(), "SetFieldValue");
}

}
