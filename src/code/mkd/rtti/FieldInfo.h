/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once

class IDataReader;
class IDataWriter;

namespace rtti
{
    class TypeInfo;
    class IRTTIObject;
    class PointerFixups;

    namespace EFieldDataType
    {
        enum TYPE
        {
            Int32,
            UInt32,
            Float,
            Bool,
            String,
            Vec3,
            Vec4,
            Quaternion,
            POD,
            RTTIObjectPtr,
            Enum,

            _COUNT
        };

        const char* toString(TYPE arg);
    }

    class FieldInfo
    {
    public:
        FieldInfo(TypeInfo* from_type, const char* name, size_t offset, size_t size, EFieldDataType::TYPE field_type);

        const char* getName() const;
        void* getValueVoidPtr(const IRTTIObject& obj) const;

        template <typename OutType>
        OutType& getValueRef(const IRTTIObject& obj) const
        {
            MK_ASSERT(m_dataSize == sizeof(OutType));

            return *reinterpret_cast<OutType*>(getValueVoidPtr(obj));
        }

        bool readValue(IRTTIObject& obj, IDataReader* reader, PointerFixups* fixups = NULL) const;
        bool writeValue(const IRTTIObject& obj, IDataWriter* writer) const;

        // :( for presets
        void setValueInObjectFromString(IRTTIObject& obj, const mkString& val_str) const;

        // TODO that's so not safe :( - add at least some check
        template <typename InType>
        void setValueInObject(IRTTIObject& obj, const InType& val) const
        {
            getValueRef<InType>(obj) = val;
        }

        void printInfo() const;

        EFieldDataType::TYPE getDataType() const { return m_dataType; }
        size_t getDataSize() const { return m_dataSize; }

        // Correct only for Enum fields
        // If value with such value does not exist, default_val is returned
        int getEnumValueByName(const mkString& name, int default_val = -1) const;
        mkString getEnumNameByValue(int val) const;
        void registerEnumValueName(const mkString& name, int value);

    private:
        TypeInfo* m_containingType;
        const char* m_name;
        size_t m_offset;
        size_t m_dataSize;
        EFieldDataType::TYPE m_dataType;

        typedef std::map<mkString, int> TEnumValuesMap;
        TEnumValuesMap m_enumValues;

        static const char* GENERIC_ENUM_VALUE_NAME_PREFIX;
    };
}

#define START_RTTI_INIT(clazz) \
    void clazz::initRTTI() { \
        rtti::TypeInfo* info = &clazz::Type; \
        clazz* class_ptr = (clazz*)0;

#define FIELD_NO_SIZE_CHECK(name, datatype_rtti) \
    info->addField(rtti::FieldInfo(info, #name, (size_t)&(class_ptr->name), sizeof(class_ptr->name), datatype_rtti))

#define FIELD(name, datatype_rtti, datatype) \
    STATIC_ASSERT(sizeof(class_ptr->name) == sizeof(datatype)); \
    FIELD_NO_SIZE_CHECK(name, datatype_rtti)

#define END_RTTI_INIT() Type.finishConstruction(); }

#define FIELD_FLOAT(name) FIELD(name, rtti::EFieldDataType::Float, float)
#define FIELD_INT32(name) FIELD(name, rtti::EFieldDataType::Int32, int32)
#define FIELD_UINT32(name) FIELD(name, rtti::EFieldDataType::UInt32, uint32)
#define FIELD_BOOL(name) FIELD(name, rtti::EFieldDataType::Bool, bool)
#define FIELD_STRING(name) FIELD(name, rtti::EFieldDataType::String, mkString)
#define FIELD_VEC3(name) FIELD(name, rtti::EFieldDataType::Vec3, mkVec3)
#define FIELD_VEC4(name) FIELD(name, rtti::EFieldDataType::Vec4, mkVec4)
#define FIELD_QUAT(name) FIELD(name, rtti::EFieldDataType::Quaternion, mkQuat)
#define FIELD_PTR(name) FIELD(name, rtti::EFieldDataType::RTTIObjectPtr, IRTTIObject*)

#define FIELD_POD(name) FIELD_NO_SIZE_CHECK(name, rtti::EFieldDataType::POD)

namespace rtti
{
    template <typename EnumType>
    struct ToStringFuncHelper
    {
        typedef mkString (*Type)(EnumType);
    };

    template <typename EnumType>
    void _registerEnumValues(EnumType begin_val, EnumType end_val,
        typename ToStringFuncHelper<EnumType>::Type to_string_func,
        rtti::FieldInfo* out_field)
    {
        for (int i = begin_val; i < end_val; ++i)
        {
            const EnumType val = (EnumType)i;
            const mkString val_name = to_string_func(val);

            out_field->registerEnumValueName(val_name, val);
        }
    }
}

#define FIELD_ENUM(name) \
    STATIC_ASSERT(sizeof(class_ptr->name) == sizeof(int32)); \
    {  rtti::FieldInfo* enum_field = info->_addField_RetPtr(rtti::FieldInfo(info, #name, (size_t)&(class_ptr->name), sizeof(class_ptr->name), rtti::EFieldDataType::Enum));

#define ENUM_VAL(val) enum_field->registerEnumValueName(#val, val);
#define ENUM_END() }

// Generic enum registration function. Iterates from enum_namespace::_FIRST to enum_namespace::_COUNT, calling
// enum_namespace::toString and registers those values in field info. When registering enums like this,
// there should be no holes in enum (every value from _FIRST to _COUNT should have meaning)
#define FIELD_ENUM_GEN(name, enum_namespace) \
    FIELD_ENUM(name); \
    rtti::_registerEnumValues(enum_namespace::_FIRST, enum_namespace::_COUNT, &enum_namespace::toString, enum_field); \
    ENUM_END();
