#include "pch.h"
#include "TypeManager.h"
#include "TypeInfo.h"

rtti::TypeManager::TypeManager()
    : m_sorted(false)
{

}

struct TypeNameLess
    : public std::binary_function< std::pair<mkString, rtti::TypeInfo*>,
                                   std::pair<mkString, rtti::TypeInfo*>,
                                   bool>
{
    bool operator()(const std::pair<mkString, rtti::TypeInfo*>& lhs, const std::pair<mkString, rtti::TypeInfo*>& rhs) const
    {
        return lhs.first < rhs.first;
    }
};

rtti::TypeInfo* rtti::TypeManager::getTypeInfoByName( const mkString& name ) const
{
    if (!m_sorted)
    {
        log_warning("Registered type infos are not sorted!");

        for (NamedTypeInfoVec::const_iterator iter = m_registeredTypes.begin();
            iter != m_registeredTypes.end(); ++iter)
        {
            if (iter->first == name)
                return iter->second;
        }
    }
    else
    {
        std::pair<mkString, rtti::TypeInfo*> search_pair;
        search_pair.first = name;
        search_pair.second = NULL;

        NamedTypeInfoVec::const_iterator iter = std::lower_bound(m_registeredTypes.begin(),
            m_registeredTypes.end(), search_pair, TypeNameLess());

        if (iter != m_registeredTypes.end() && iter->first == name)
            return iter->second;
    }

    log_error("Could not find type info with name \'%s\'", name.c_str());
    return NULL;
}

void rtti::TypeManager::registerTypeInfo( const mkString& name, TypeInfo* type )
{
    MK_ASSERT(type);

    m_registeredTypes.push_back(std::make_pair(name, type));

    m_sorted = false;

    if (type->shouldRegisterInScripts())
        m_scriptedTypesVec.push_back(type);
}

void rtti::TypeManager::finishTypeRegistration()
{
    std::sort(m_registeredTypes.begin(), m_registeredTypes.end(), TypeNameLess());
    m_sorted = true;

    validateRegisteredTypes();
}

void rtti::TypeManager::validateRegisteredTypes()
{
    const mkString* previous_name = NULL;
    for (NamedTypeInfoVec::const_iterator iter = m_registeredTypes.begin();
        iter != m_registeredTypes.end(); ++iter)
    {
        const mkString& name = iter->first;
        TypeInfo* info_ptr = iter->second;

        if (name != info_ptr->getClassName())
            log_error("Invalid type info: registered name '%s', type info name '%s'", name.c_str(), info_ptr->getClassName());

        if (previous_name && *previous_name == name)
            log_error("Invalid type info: duplicated name '%s'", name.c_str());

        previous_name = &iter->first;
    }
}

size_t rtti::TypeManager::getTypeInfoCount() const
{
    return m_registeredTypes.size();
}

rtti::TypeInfo* rtti::TypeManager::getTypeInfoByIdx( size_t idx ) const
{
    return m_registeredTypes[idx].second;
}

void rtti::TypeManager::registerTypesInScript( IScriptingContext* context ) const
{
    for (TypeInfoVec::const_iterator iter = m_scriptedTypesVec.begin();
        iter != m_scriptedTypesVec.end(); ++iter)
    {
        TypeInfo* type = *iter;
        type->registerInScriptContext(context);
    }
}
