/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "LuaSimpleBindingImplCommon.h"

namespace lua_simple
{

    // Provides utility function to get object's field values
    // TODO: consider adding functionality to set fields values.
    // It would be trivial, but can lead to non-obvious dependencies
    // between code and scripts

    void registerRttiFieldsAccess(LuaSimpleContext* context);

}
