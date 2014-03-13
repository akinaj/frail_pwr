/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "utils.h"
#include "FieldInfo.h"

class IDataReader;
class IDataWriter;

namespace rtti { class TypeInfo; }

class IScriptingContext
{
public:
    virtual ~IScriptingContext() { }

    // For stuff like instanceof queries etc. Registering functions is done elsewhere
    virtual void registerTypeMetadata(const rtti::TypeInfo* type) = 0;
};

namespace rtti
{
    class TypeInfo;
    class IRTTIObject;

    typedef uint32 TObjectId;

    class IObjectProvider
    {
    public:
        virtual IRTTIObject* findObjectById(TObjectId id) const = 0;
    };

    class PointerFixups
    {
    public:
        void registerPointerToFixup(IRTTIObject** pointer_address);
        void fixupPointers(IObjectProvider* object_provider);

    private:
        typedef std::vector<IRTTIObject**> TPointerAddressVec;
        TPointerAddressVec m_registeredFixups;
    };

    typedef void (*RTTIInitFuncPtr)();
    typedef IRTTIObject* (*RTTICreateInstanceFuncPtr)();
    typedef void (*RTTIDestroyInstanceFuncPtr)(IRTTIObject*);
    typedef void (*RTTIScriptInitFuncPtr)(IScriptingContext*);

    class TypeInfo
    {
    public:
        TypeInfo(const TypeInfo* super_class, const char* class_name, RTTIInitFuncPtr init_func,
            RTTICreateInstanceFuncPtr create_instance_func, RTTIDestroyInstanceFuncPtr destroy_instance_func,
            RTTIScriptInitFuncPtr script_init_func);

        void finishConstruction();

        bool isDerivedFrom(const TypeInfo* other) const;
        bool isDerivedOrExact(const TypeInfo* other) const;
        const char* getClassName() const;
        const TypeInfo* getSuperClass() const;
        const FieldInfo* getField(const char* name) const;

        size_t getFieldCount() const;
        const FieldInfo* getFieldByIdx(size_t idx) const;
        
        void addField(const FieldInfo& field);

        FieldInfo* _addField_RetPtr(const FieldInfo& field); // used for enum registration, do not use it by hand

        void printFields() const;

        typedef std::vector<FieldInfo> TFieldsVec;

        bool read(IRTTIObject& obj, IDataReader* reader, PointerFixups* fixups = NULL, bool standalone = false) const;
        bool write(const IRTTIObject& obj, IDataWriter* writer) const;

        bool writeMetadata(const IRTTIObject& obj, IDataWriter* writer) const;
        bool writeFields(const IRTTIObject& obj, IDataWriter* writer) const;

        bool readFields(IRTTIObject& obj, IDataReader* reader, PointerFixups* fixups) const;

        // Creates and destroys object, but does not call init, does not register
        // object on level or anything. To create game object, use GameObjectFactory
        IRTTIObject* rawCreateInstance() const;
        void rawDestroyInstance(IRTTIObject* obj) const;

        void registerInScriptContext(IScriptingContext*) const;
        bool shouldRegisterInScripts() const;

    private:
        const TypeInfo* m_superClass;
        const char* m_className;

        bool m_inConstruction;
        TFieldsVec m_fields;

        RTTICreateInstanceFuncPtr m_createInstanceFunc;
        RTTIDestroyInstanceFuncPtr m_destroyInstanceFunc;
        
        RTTIScriptInitFuncPtr m_initInScriptFunc;
    };

    class IRTTIObject
    {
    public:
        static const TObjectId INVALID_OBJECT_ID = ~0;

        IRTTIObject() : m_objectId(INVALID_OBJECT_ID) { }
        virtual ~IRTTIObject() { }
        virtual const TypeInfo* getTypeInfo() const = 0;

        static rtti::TypeInfo Type;
        static void initRTTI();

        TObjectId getObjectId() const;

        bool _hasValidId() const;
        void _setObjectId(TObjectId new_id);

    private:
        TObjectId m_objectId;
    };

    template <typename T>
    IRTTIObject* genericCreateRTTIObject()
    {
        return new T;
    }

    void genericDestroyRTTIObject(IRTTIObject* obj);

#define DECLARE_RTTI(type) public:                                                          \
                            static rtti::TypeInfo Type;                                     \
                            static void initRTTI();                                         \
                            static void registerTypeInScriptContext(IScriptingContext*);    \
                            virtual const rtti::TypeInfo* getTypeInfo() const;              \
                           private:

#define IMPLEMENT_RTTI_NOSCRIPT(type, superclass)                                   \
    rtti::TypeInfo type::Type(&superclass::Type, #type, &type::initRTTI,            \
    &rtti::genericCreateRTTIObject<type>, &rtti::genericDestroyRTTIObject, NULL);   \
    const rtti::TypeInfo* type::getTypeInfo() const { return &Type; }

#define IMPLEMENT_RTTI_SCRIPTED(type, superclass)                           \
    rtti::TypeInfo type::Type(&superclass::Type, #type, &type::initRTTI,    \
    &rtti::genericCreateRTTIObject<type>, &rtti::genericDestroyRTTIObject,  \
    &type::registerTypeInScriptContext);                                    \
    const rtti::TypeInfo* type::getTypeInfo() const { return &Type; }

}
