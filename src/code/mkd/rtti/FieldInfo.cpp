/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "FieldInfo.h"
#include "TypeInfo.h"
#include "serialization/Serialization.h"

const char* rtti::EFieldDataType::toString( TYPE arg )
{
    switch (arg)
    {
#define CASE(v) case v: return #v;
        CASE(Int32)
        CASE(UInt32)
        CASE(Float)
        CASE(Bool)
        CASE(String)
        CASE(Vec3)
        CASE(Vec4)
        CASE(Quaternion)
        CASE(POD)
        CASE(RTTIObjectPtr)
        CASE(Enum)
#undef CASE
    }

    return "";
}

rtti::FieldInfo::FieldInfo( TypeInfo* from_type, const char* name, size_t offset, size_t size, EFieldDataType::TYPE data_type )
    : m_containingType(from_type)
    , m_name(name)
    , m_offset(offset)
    , m_dataType(data_type)
    , m_dataSize(size)
{
}

void* rtti::FieldInfo::getValueVoidPtr( const IRTTIObject& obj ) const
{
    // Insanely unsafe, I know ;) Will not work with multiple inheritance
    return const_cast<char*>(reinterpret_cast<const char*>(&obj) + m_offset);
}

const char* rtti::FieldInfo::getName() const
{
    return m_name;
}

void rtti::FieldInfo::printInfo() const
{
    log_info("[%2d] %s::%s", m_offset, m_containingType->getClassName(), m_name);
}

const char* rtti::FieldInfo::GENERIC_ENUM_VALUE_NAME_PREFIX = "##ENUM_VAL_";

bool rtti::FieldInfo::readValue( IRTTIObject& obj, IDataReader* reader, PointerFixups* fixups ) const
{
    switch(m_dataType)
    {
    #define READ_TYPE(type, type_name) case EFieldDataType::type_name: return reader->read ## type_name (m_name, getValueRef<type>(obj));
        READ_TYPE(int32, Int32);
        READ_TYPE(uint32, UInt32);
        READ_TYPE(float, Float);
        READ_TYPE(mkString, String);
    #undef READ_TYPE

    case EFieldDataType::Bool:
        {
            bool& out = getValueRef<bool>(obj);
            int32 val = 0;
            bool success = reader->readInt32(m_name, val);
            if (success)
                out = (val != 0);

            return success;
        }

    case EFieldDataType::Quaternion:
        {
            mkQuat& out = getValueRef<mkQuat>(obj);
            bool success = true;

            success &= reader->readStartBlock(m_name);
            if (success)
            {
                success &= reader->readFloat(NULL, out.x);
                success &= reader->readFloat(NULL, out.y);
                success &= reader->readFloat(NULL, out.z);
                success &= reader->readFloat(NULL, out.w);
                success &= reader->readEndBlock(m_name);
            }

            return success;
        }

    case EFieldDataType::Vec3:
        {
            mkVec3& out = getValueRef<mkVec3>(obj);
            bool success = true;

            success &= reader->readStartBlock(m_name);
            if (success)
            {
                success &= reader->readFloat(NULL, out.x);
                success &= reader->readFloat(NULL, out.y);
                success &= reader->readFloat(NULL, out.z);
                success &= reader->readEndBlock(m_name);
            }

            return success;
        }

    case EFieldDataType::Vec4:
        {
            mkVec4& out = getValueRef<mkVec4>(obj);
            bool success = true;

            success &= reader->readStartBlock(m_name);
            if (success)
            {
                success &= reader->readFloat(NULL, out.x);
                success &= reader->readFloat(NULL, out.y);
                success &= reader->readFloat(NULL, out.z);
                success &= reader->readFloat(NULL, out.w);
                success &= reader->readEndBlock(m_name);
            }

            return success;
        }

    case EFieldDataType::POD:
        {
            uint8* bytes = reinterpret_cast<uint8*>(getValueVoidPtr(obj));
            size_t bytes_num = m_dataSize;

            return reader->readBytes(m_name, bytes_num, bytes);
        }

    case EFieldDataType::RTTIObjectPtr:
        {
            if (fixups == NULL)
            {
                log_error("Can't read %s::%s of type RTTIObjectPtr: no pointer fixups object provided",
                    m_containingType->getClassName(), m_name);
                return false;
            }

            STATIC_ASSERT(sizeof(uint32) >= sizeof(TObjectId));

            uint32& field_ref = *reinterpret_cast<uint32*>(getValueVoidPtr(obj));
            if (!reader->readUInt32(m_name, field_ref))
                return false;

            fixups->registerPointerToFixup(&getValueRef<IRTTIObject*>(obj));

            break;
        }

    case EFieldDataType::Enum:
        {
            int32& out = getValueRef<int32>(obj);
            mkString value_name;
            bool success = reader->readString(m_name, value_name);
            if (success)
            {
                int read_val = getEnumValueByName(value_name, INT_MAX);
                if (read_val == INT_MAX && string_starts_with(value_name.c_str(), GENERIC_ENUM_VALUE_NAME_PREFIX))
                {
                    mkString val = value_name.substr(strlen(GENERIC_ENUM_VALUE_NAME_PREFIX));
                    read_val = Ogre::StringConverter::parseInt(val, out);
                }

                out = read_val;
            }

            return success;
        }
    }

    return false;
}

bool rtti::FieldInfo::writeValue( const IRTTIObject& obj, IDataWriter* writer ) const
{
    switch(m_dataType)
    {
#define WRITE_TYPE(type, type_name) case EFieldDataType::type_name: return writer->write ## type_name (m_name, getValueRef<type>(obj));
        WRITE_TYPE(int32, Int32);
        WRITE_TYPE(uint32, UInt32);
        WRITE_TYPE(float, Float);
        WRITE_TYPE(mkString, String);
#undef WRITE_TYPE

    case EFieldDataType::Bool:
        {
            bool val = getValueRef<bool>(obj);
            return writer->writeInt32(m_name, (val ? 1 : 0));
        }

    case EFieldDataType::Vec3:
        {
            mkVec3 out = getValueRef<mkVec3>(obj);
            bool success = true;

            success &= writer->writeStartBlock(m_name);
            if (success)
            {
                success &= writer->writeFloat(NULL, out.x);
                success &= writer->writeFloat(NULL, out.y);
                success &= writer->writeFloat(NULL, out.z);
                success &= writer->writeEndBlock();
            }

            return success;
        }

    case EFieldDataType::Vec4:
        {
            mkVec4 out = getValueRef<mkVec4>(obj);
            bool success = true;

            success &= writer->writeStartBlock(m_name);
            if (success)
            {
                success &= writer->writeFloat(NULL, out.x);
                success &= writer->writeFloat(NULL, out.y);
                success &= writer->writeFloat(NULL, out.z);
                success &= writer->writeFloat(NULL, out.w);
                success &= writer->writeEndBlock();
            }

            return success;
        }

    case EFieldDataType::Quaternion:
        {
            mkQuat out = getValueRef<mkQuat>(obj);
            bool success = true;

            success &= writer->writeStartBlock(m_name);
            if (success)
            {
                success &= writer->writeFloat(NULL, out.x);
                success &= writer->writeFloat(NULL, out.y);
                success &= writer->writeFloat(NULL, out.z);
                success &= writer->writeFloat(NULL, out.w);
                success &= writer->writeEndBlock();
            }

            return success;
        }

    case EFieldDataType::POD:
        {
            uint8* bytes = reinterpret_cast<uint8*>(getValueVoidPtr(obj));
            size_t bytes_num = m_dataSize;

            return writer->writeBytes(m_name, bytes_num, bytes);
        }

    case EFieldDataType::RTTIObjectPtr:
        {
            IRTTIObject* field_value = getValueRef<IRTTIObject*>(obj);

            STATIC_ASSERT(sizeof(uint32) >= sizeof(TObjectId));

            uint32 uint_value = 0;
            if (field_value != NULL)
                uint_value = field_value->getObjectId();

            return writer->writeUInt32(m_name, uint_value);
        }

    case EFieldDataType::Enum:
        {
            int32 out = getValueRef<int32>(obj);
            mkString value_to_write = getEnumNameByValue(out);
            if (value_to_write.empty())
            {
                value_to_write = GENERIC_ENUM_VALUE_NAME_PREFIX;
                value_to_write += Ogre::StringConverter::toString(out);
            }

            return writer->writeString(m_name, value_to_write);
        }
    }

    return false;
}

void rtti::FieldInfo::setValueInObjectFromString( IRTTIObject& obj, const mkString& val_str ) const
{
    switch (m_dataType)
    {
    case EFieldDataType::Enum:
        getValueRef<int32>(obj) = getEnumValueByName(val_str.c_str(), getValueRef<int32>(obj));
        break;

    case EFieldDataType::Bool:
        getValueRef<bool>(obj) = Ogre::StringConverter::parseBool(val_str);
        break;

    case EFieldDataType::String:
        getValueRef<mkString>(obj) = val_str;
        break;

    case EFieldDataType::Int32:
        getValueRef<int32>(obj) = Ogre::StringConverter::parseInt(val_str);
        break;

    case EFieldDataType::UInt32:
        getValueRef<uint32>(obj) = Ogre::StringConverter::parseUnsignedInt(val_str);
        break;

    case EFieldDataType::Float:
        getValueRef<float>(obj) = Ogre::StringConverter::parseReal(val_str);
        break;

    case EFieldDataType::Vec3:
        getValueRef<mkVec3>(obj) = Ogre::StringConverter::parseVector3(val_str);
        break;

    case EFieldDataType::Vec4:
        getValueRef<mkVec4>(obj) = Ogre::StringConverter::parseVector4(val_str);
        break;

    case EFieldDataType::Quaternion:
        getValueRef<mkQuat>(obj) = Ogre::StringConverter::parseQuaternion(val_str);
        break;

    case EFieldDataType::RTTIObjectPtr:
    case EFieldDataType::POD:
        log_error("Trying to set field '%s::%s' of type %s to value '%s' from string",
            m_containingType->getClassName(), m_name, EFieldDataType::toString(m_dataType), val_str.c_str());
        break;
    }
}

int rtti::FieldInfo::getEnumValueByName( const mkString& name, int default_val ) const
{
    MK_ASSERT(m_dataType == EFieldDataType::Enum);

    TEnumValuesMap::const_iterator iter = m_enumValues.find(name);
    if (iter != m_enumValues.end())
        return iter->second;

    log_error("Could not find value named '%s' for enum field '%s'", name.c_str(), m_name);

    return default_val;
}

void rtti::FieldInfo::registerEnumValueName( const mkString& name, int value )
{
    MK_ASSERT(m_dataType == EFieldDataType::Enum);
    m_enumValues[name] = value;
}

mkString rtti::FieldInfo::getEnumNameByValue( int val ) const
{
    MK_ASSERT(m_dataType == EFieldDataType::Enum);
    for (TEnumValuesMap::const_iterator iter = m_enumValues.begin(); iter != m_enumValues.end(); ++iter)
        if (iter->second == val)
            return iter->first;

    log_error("Could not find name of value '%d' for enum field '%s'", val, m_name);

    return NULL;
}
