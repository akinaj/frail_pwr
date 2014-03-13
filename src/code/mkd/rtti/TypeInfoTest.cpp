/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "TypeInfo.h"
#include "FieldInfo.h"
#include "TypeManager.h"
#include "serialization/SerializationJSON.h"

namespace Test
{
    struct xyz { float x, y, z; };

    class ClassWithTypeInfo : public rtti::IRTTIObject
    {    
        DECLARE_RTTI(ClassWithTypeInfo);

    public:
        ClassWithTypeInfo();
        ~ClassWithTypeInfo();

    private:
        float m_floatField;
        int m_intField;
        mkVec3 m_vec3Field;
        ClassWithTypeInfo* m_ptrField;
        xyz m_podField;
    };

    class DerivedClass : public ClassWithTypeInfo
    {
        DECLARE_RTTI(DerivedClass);

    public:
        DerivedClass();
        ~DerivedClass();

    private:
        int m_intFieldDerived;
    };

    class ClassWithPointer : public rtti::IRTTIObject
    {
        DECLARE_RTTI(ClassWithPointer);

    public:
        ClassWithPointer() : m_ptrField(NULL) { }
        ClassWithTypeInfo* m_ptrField;
    };

    IMPLEMENT_RTTI_NOSCRIPT(ClassWithPointer, rtti::IRTTIObject);

    START_RTTI_INIT(ClassWithPointer);
    {
        FIELD_PTR(m_ptrField);
    }
    END_RTTI_INIT();


////////////////////////////////////////////////////////////////////////////////////////////////
IMPLEMENT_RTTI_NOSCRIPT(Test::ClassWithTypeInfo, rtti::IRTTIObject);

START_RTTI_INIT(ClassWithTypeInfo)
{
    FIELD_FLOAT(m_floatField);
    FIELD_INT32(m_intField);
    FIELD_VEC3(m_vec3Field);
    FIELD_POD(m_ptrField);
    FIELD_POD(m_podField);
}
END_RTTI_INIT();

Test::ClassWithTypeInfo::ClassWithTypeInfo()
    : m_floatField(-1.234f)
    , m_intField(666)
    , m_vec3Field(1.f, 2.f, 3.f)
    , m_ptrField(NULL)
{
    log_trace_method();
    m_podField.x = 7.f;
    m_podField.y = 8.f;
    m_podField.z = 9.f;
}

Test::ClassWithTypeInfo::~ClassWithTypeInfo()
{
    log_trace_method();
}

IMPLEMENT_RTTI_NOSCRIPT(Test::DerivedClass, Test::ClassWithTypeInfo);

START_RTTI_INIT(DerivedClass)
{
    FIELD_INT32(m_intFieldDerived);
}
END_RTTI_INIT();

Test::DerivedClass::DerivedClass()
: m_intFieldDerived(-987)
{
    log_trace_method();
}

Test::DerivedClass::~DerivedClass()
{
    log_trace_method();
}

void RttiTest4();
void RttiTest5();
void RttiTest6();
void RttiTest7();

struct MyRttiTest
{
    MyRttiTest()
    {
        {
            Test::ClassWithTypeInfo obj1;
            obj1.getTypeInfo()->printFields();
            Test::DerivedClass obj2;
            obj2.getTypeInfo()->printFields();

            mkVec3 foo = obj1.getTypeInfo()->getField("m_vec3Field")->getValueRef<mkVec3>(obj1);
            Test::xyz bar = obj1.getTypeInfo()->getField("m_podField")->getValueRef<Test::xyz>(obj1);
            log_info("%f, %i, [%g, %g, %g], %p, [%g, %g, %g]",
                obj1.getTypeInfo()->getField("m_floatField")->getValueRef<float>(obj1),
                obj1.getTypeInfo()->getField("m_intField")->getValueRef<int>(obj1),
                foo.x, foo.y, foo.z, obj1.getTypeInfo()->getField("m_ptrField")->getValueRef<void*>(obj1),
                bar.x, bar.y, bar.z);
            log_info("%f, %i, [%g, %g, %g], %p, [%g, %g, %g], %i",
                obj2.getTypeInfo()->getField("m_floatField")->getValueRef<float>(obj2),
                obj2.getTypeInfo()->getField("m_intField")->getValueRef<int>(obj2),
                foo.x, foo.y, foo.z, obj2.getTypeInfo()->getField("m_ptrField")->getValueRef<void*>(obj2),
                bar.x, bar.y, bar.z, obj2.getTypeInfo()->getField("m_intFieldDerived")->getValueRef<int>(obj2));

            JSONDataWriterString writer;
            obj1.getTypeInfo()->write(obj1, &writer);
            obj2.getTypeInfo()->write(obj2, &writer);

            mkString result;
            writer.extractString(result);

            log_info("%s\n", result.c_str());
        }

        {
            Test::ClassWithTypeInfo* obj1 = static_cast<Test::ClassWithTypeInfo*>(Test::ClassWithTypeInfo::Type.rawCreateInstance());
            Test::DerivedClass* obj2 = static_cast<Test::DerivedClass*>(Test::DerivedClass::Type.rawCreateInstance());

            JSONDataWriterString writer;
            obj1->getTypeInfo()->write(*obj1, &writer);
            obj2->getTypeInfo()->write(*obj2, &writer);

            mkString result;
            writer.extractString(result);

            log_info("%s\n", result.c_str());

            obj1->getTypeInfo()->rawDestroyInstance(obj1);
            obj2->getTypeInfo()->rawDestroyInstance(obj2);

            obj1 = NULL;
            obj2 = NULL;
        }

        RttiTest4();
        RttiTest5();
        RttiTest6();
    }
} g_Test;

rtti::IRTTIObject* createObjectByClassName(const char* name)
{
    rtti::TypeInfo* info = rtti::TypeManager::getInstance().getTypeInfoByName(name);
    MK_ASSERT(info);

    return info->rawCreateInstance();
}

void RttiTest3()
{
    Test::ClassWithTypeInfo* obj1 = static_cast<Test::ClassWithTypeInfo*>(createObjectByClassName("Test::ClassWithTypeInfo"));
    Test::DerivedClass* obj2 = static_cast<Test::DerivedClass*>(createObjectByClassName("Test::DerivedClass"));

    JSONDataWriterString writer;
    obj1->getTypeInfo()->write(*obj1, &writer);
    obj2->getTypeInfo()->write(*obj2, &writer);

    mkString result;
    writer.extractString(result);

    log_info("%s\n", result.c_str());

    obj1->getTypeInfo()->rawDestroyInstance(obj1);
    obj2->getTypeInfo()->rawDestroyInstance(obj2);
}

struct DummyObjectProvider : public rtti::IObjectProvider
{
    rtti::IRTTIObject* findObjectById(rtti::TObjectId id) const
    {
        return NULL;
    }
};

struct SimpleObjectProvider : public rtti::IObjectProvider
{
    typedef std::map<rtti::TObjectId, rtti::IRTTIObject*> TObjectMap;
    TObjectMap m_objects;

    rtti::IRTTIObject* findObjectById(rtti::TObjectId id) const
    {
        TObjectMap::const_iterator iter = m_objects.find(id);
        if (iter != m_objects.end())
            return iter->second;
        else
            return NULL;
    }
};

void RttiTest4()
{
    Test::ClassWithTypeInfo* obj1 = static_cast<Test::ClassWithTypeInfo*>(Test::ClassWithTypeInfo::Type.rawCreateInstance());
    Test::ClassWithPointer* obj2 = static_cast<Test::ClassWithPointer*>(Test::ClassWithPointer::Type.rawCreateInstance());

    obj1->_setObjectId(1);
    obj2->_setObjectId(2);

    JSONDataWriterString writer;
    obj1->getTypeInfo()->write(*obj1, &writer);
    obj2->getTypeInfo()->write(*obj2, &writer);

    mkString result;
    writer.extractString(result);

    log_info("%s\n", result.c_str());

    JSONDataReaderString reader(result.c_str(), true);

    rtti::PointerFixups fixups;

    Test::ClassWithTypeInfo* obj3 = static_cast<Test::ClassWithTypeInfo*>(Test::ClassWithTypeInfo::Type.rawCreateInstance());
    Test::ClassWithPointer* obj4 = static_cast<Test::ClassWithPointer*>(Test::ClassWithPointer::Type.rawCreateInstance());

    obj3->getTypeInfo()->read(*obj3, &reader, &fixups, true);
    obj4->getTypeInfo()->read(*obj4, &reader, &fixups, true);

    DummyObjectProvider dummy_provider;
    fixups.fixupPointers(&dummy_provider);

    if (obj4->m_ptrField != NULL)
        log_error("RTTI ptr field: null test failed");
}

void RttiTest5()
{
    Test::ClassWithTypeInfo* obj2 = static_cast<Test::ClassWithTypeInfo*>(Test::ClassWithTypeInfo::Type.rawCreateInstance());

    obj2->_setObjectId(0xDEADF00D);

    JSONDataWriterString writer;
    obj2->getTypeInfo()->write(*obj2, &writer);

    mkString result;
    writer.extractString(result);

    JSONDataReaderString reader(result.c_str(), true);
    Test::ClassWithTypeInfo* obj4 = static_cast<Test::ClassWithTypeInfo*>(Test::ClassWithTypeInfo::Type.rawCreateInstance());
    obj4->getTypeInfo()->read(*obj4, &reader, NULL, true);

    if (obj4->getObjectId() != 0xDEADF00D)
        log_error("object id test failed");
}

void RttiTest6()
{
    Test::ClassWithTypeInfo* obj1 = static_cast<Test::ClassWithTypeInfo*>(Test::ClassWithTypeInfo::Type.rawCreateInstance());
    Test::ClassWithPointer* obj2 = static_cast<Test::ClassWithPointer*>(Test::ClassWithPointer::Type.rawCreateInstance());

    obj1->_setObjectId(0xDEADF00D);
    obj2->_setObjectId(0xE123CAFE);

    obj2->m_ptrField = obj1;

    JSONDataWriterString writer;
    obj1->getTypeInfo()->write(*obj1, &writer);
    obj2->getTypeInfo()->write(*obj2, &writer);

    mkString result;
    writer.extractString(result);

    log_info("%s\n", result.c_str());

    JSONDataReaderString reader(result.c_str(), true);

    rtti::PointerFixups fixups;

    Test::ClassWithTypeInfo* obj3 = static_cast<Test::ClassWithTypeInfo*>(Test::ClassWithTypeInfo::Type.rawCreateInstance());
    Test::ClassWithPointer* obj4 = static_cast<Test::ClassWithPointer*>(Test::ClassWithPointer::Type.rawCreateInstance());

    obj3->getTypeInfo()->read(*obj3, &reader, &fixups, true);
    obj4->getTypeInfo()->read(*obj4, &reader, &fixups, true);

    SimpleObjectProvider provider;
    provider.m_objects[obj3->getObjectId()] = obj3;
    provider.m_objects[obj4->getObjectId()] = obj4;

    fixups.fixupPointers(&provider);

    if (obj4->m_ptrField != obj3)
        log_error("RTTI ptr field: pointer fixup failed");
}

}
