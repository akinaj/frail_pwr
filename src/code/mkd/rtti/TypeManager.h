/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once

class IScriptingContext;

namespace rtti
{

    class TypeInfo;
    class IRTTIObject;

    class TypeManager
    {
    public:
        static TypeManager& getInstance()
        {
            static TypeManager s_instance;

            return s_instance;
        }

        TypeInfo* getTypeInfoByName(const mkString& name) const;
        void registerTypeInfo(const mkString& name, TypeInfo* type);

        void finishTypeRegistration();

        size_t getTypeInfoCount() const;
        TypeInfo* getTypeInfoByIdx(size_t idx) const;

        void registerTypesInScript(IScriptingContext* context) const;

    private:
        TypeManager();

        void validateRegisteredTypes();

        typedef std::vector< std::pair<mkString, TypeInfo*> > NamedTypeInfoVec;
        NamedTypeInfoVec m_registeredTypes;

        typedef std::vector<TypeInfo*> TypeInfoVec;
        TypeInfoVec m_scriptedTypesVec;

        bool m_sorted;
    };

}
