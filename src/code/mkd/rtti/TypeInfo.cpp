/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "TypeInfo.h"
#include "FieldInfo.h"
#include "TypeManager.h"
#include "serialization/Serialization.h"

rtti::TypeInfo rtti::IRTTIObject::Type(NULL, "IRttiObject", &rtti::IRTTIObject::initRTTI, NULL, NULL, NULL);

void rtti::IRTTIObject::initRTTI()
{

}

rtti::TObjectId rtti::IRTTIObject::getObjectId() const
{
    return m_objectId;
}

bool rtti::IRTTIObject::_hasValidId() const
{
    return m_objectId != INVALID_OBJECT_ID;
}

void rtti::IRTTIObject::_setObjectId( TObjectId new_id )
{
    MK_ASSERT(new_id != INVALID_OBJECT_ID);

    m_objectId = new_id;
}

void rtti::genericDestroyRTTIObject( IRTTIObject* obj )
{
    delete obj;
}

////////////////////////////////////////////////////////////////////////////////////////////////
rtti::TypeInfo::TypeInfo( const TypeInfo* super_class, const char* class_name, RTTIInitFuncPtr init_func,
                         RTTICreateInstanceFuncPtr create_instance_func, RTTIDestroyInstanceFuncPtr destroy_instance_func,
                         RTTIScriptInitFuncPtr script_init_func)
    : m_superClass(super_class)
    , m_className(class_name)
    , m_createInstanceFunc(create_instance_func)
    , m_destroyInstanceFunc(destroy_instance_func)
    , m_inConstruction(true)
    , m_initInScriptFunc(script_init_func)
{
    init_func();

    TypeManager::getInstance().registerTypeInfo(class_name, this);
}

bool rtti::TypeInfo::isDerivedFrom( const TypeInfo* other ) const
{
    const TypeInfo* ancestor = m_superClass;
    while (ancestor != &IRTTIObject::Type)
    {
        if (ancestor == other)
            return true;

        ancestor = ancestor->getSuperClass();
    }

    return false;
}

bool rtti::TypeInfo::isDerivedOrExact( const TypeInfo* other ) const
{
    return this == other || isDerivedFrom(other);
}

const char* rtti::TypeInfo::getClassName() const
{
    return m_className;
}

const rtti::TypeInfo* rtti::TypeInfo::getSuperClass() const
{
    return m_superClass;
}

const rtti::FieldInfo* rtti::TypeInfo::getField( const char* name ) const
{
    MK_ASSERT(!m_inConstruction);

    for (size_t i = 0; i < m_fields.size(); ++i)
    {
        const FieldInfo& field = m_fields[i];
        if (strcmp(name, field.getName()) == 0)
            return &field;
    }

    if (getSuperClass())
        return getSuperClass()->getField(name);

    return NULL;
}

void rtti::TypeInfo::addField( const FieldInfo& field )
{
    MK_ASSERT(m_inConstruction);
    m_fields.push_back(field);
}

rtti::FieldInfo* rtti::TypeInfo::_addField_RetPtr( const FieldInfo& field )
{
    addField(field);
    return &m_fields.back();
}

void rtti::TypeInfo::finishConstruction()
{
    MK_ASSERT(m_inConstruction);
    m_inConstruction = false;
}

void rtti::TypeInfo::printFields() const
{
    log_info("RTTI fields info of type %s", getClassName());

    for (size_t i = 0; i < m_fields.size(); ++i)
        m_fields[i].printInfo();

    if (getSuperClass())
        getSuperClass()->printFields();
}

bool rtti::TypeInfo::read( IRTTIObject& obj, IDataReader* reader, PointerFixups* fixups, bool standalone ) const
{
    bool success = true;

    if (standalone)
        success &= reader->readStartBlock(m_className);

    uint32 obj_id = 0;
    const bool has_id = reader->readUInt32("ObjectID", obj_id);
    if (has_id)
        obj._setObjectId(obj_id);

    success &= readFields(obj, reader, fixups);

    if (standalone)
        success &= reader->readEndBlock(m_className);

    return success;
}

bool rtti::TypeInfo::write( const IRTTIObject& obj, IDataWriter* writer ) const
{
    bool success = writer->writeStartBlock(m_className);

    if (success)
    {
        success &= writeMetadata(obj, writer);
        success &= writeFields(obj, writer);
        success &= writer->writeEndBlock();
    }

    return success;
}

bool rtti::TypeInfo::writeMetadata( const IRTTIObject& obj, IDataWriter* writer ) const
{
    bool success = writer->writeUInt32("ObjectID", obj.getObjectId());
    return success;
}

bool rtti::TypeInfo::writeFields( const IRTTIObject& obj, IDataWriter* writer ) const
{
    bool success = true;
    if (m_superClass)
        success &= m_superClass->writeFields(obj, writer);

    for (size_t i = 0; i < m_fields.size(); ++i)
        success &= m_fields[i].writeValue(obj, writer);

    return success;
}

bool rtti::TypeInfo::readFields( IRTTIObject& obj, IDataReader* reader, PointerFixups* fixups ) const
{
    if (m_superClass)
        m_superClass->readFields(obj, reader, fixups);

    for (size_t i = 0; i < m_fields.size(); ++i)
        m_fields[i].readValue(obj, reader, fixups);

    return true;
}

rtti::IRTTIObject* rtti::TypeInfo::rawCreateInstance() const
{
    MK_ASSERT(m_createInstanceFunc);

    return m_createInstanceFunc();
}

void rtti::TypeInfo::rawDestroyInstance( IRTTIObject* obj ) const
{
    MK_ASSERT(obj);
    MK_ASSERT(m_destroyInstanceFunc);

    m_destroyInstanceFunc(obj);
}

size_t rtti::TypeInfo::getFieldCount() const
{
    return m_fields.size();
}

const rtti::FieldInfo* rtti::TypeInfo::getFieldByIdx( size_t idx ) const
{
    MK_ASSERT(idx < m_fields.size());

    return &m_fields[idx];
}

void rtti::TypeInfo::registerInScriptContext( IScriptingContext* context ) const
{
    MK_ASSERT_MSG(m_initInScriptFunc,
        "Class '%s' is requested to register in script, but has no script register function!", m_className);

    if (m_initInScriptFunc)
    {
        m_initInScriptFunc(context);
        context->registerTypeMetadata(this);
    }
}

bool rtti::TypeInfo::shouldRegisterInScripts() const
{
    return m_initInScriptFunc != NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void rtti::PointerFixups::registerPointerToFixup( IRTTIObject** pointer_address )
{
    m_registeredFixups.push_back(pointer_address);
}

void rtti::PointerFixups::fixupPointers(IObjectProvider* provider)
{
    log_info("Registered %d pointer fixups, resolving...", m_registeredFixups.size());

    uint32 null_count = 0;
    uint32 missed_count = 0;

    union ConvertUnion
    {
        IRTTIObject* ptr_val;
        TObjectId obj_id_val;
    };

    STATIC_ASSERT(sizeof(IRTTIObject*) >= sizeof(TObjectId));

    for (size_t i = 0; i < m_registeredFixups.size(); ++i)
    {
        ConvertUnion* field = reinterpret_cast<ConvertUnion*>(m_registeredFixups[i]);

        if (field->obj_id_val == 0)
        {
            field->ptr_val = NULL;
            ++null_count;
        }
        else
        {
            IRTTIObject* object_ptr = provider->findObjectById(field->obj_id_val);
            field->ptr_val = object_ptr;

            if (object_ptr == NULL)
                ++missed_count;
        }
    }

    m_registeredFixups.clear();

    log_info("Done resolving pointer fixups. %d pointers where NULL, %d pointed to non-existent objects",
        null_count, missed_count);
}
