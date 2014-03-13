#include "pch.h"
#include "LuaSimpleBinding.h"
#include "filesystem.h"
#include "utils.h"

int test(int a)
{
    return a * 10;
}

int test2(int a, int b)
{
    return a * b;
}

int test3(int a, int b, int c)
{
    return a * b + c;
}

void test4()
{
    log_info("Udalo sie!");
}

void test5(int a)
{
    log_info("Udalo sie na %d!", a);
}

struct Foo
{
    mkString bar;
};

Foo* getFoo()
{
    static Foo foo = { "dupa" };
    return &foo;
}

void printFoo(Foo* foo)
{
    log_info("%s", foo->bar.c_str());
}

struct LuaSimpleBindingTest
{
    LuaSimpleBindingTest()
    {
        uint8* text = NULL;
        uint32 size = 0;

        if (!loadFile("lua_test.lua", &text, &size, true))
            return;

        lua_simple::LuaSimpleContext context;

        context.runString((const char*)text);

        int32 test_i = 0;
        uint32 test_u = 0;
        float test_f = 0;
        mkString test_s;
        uint8 test_bytes[4] = {0};

        context.registerFunc("getFoo", getFoo);
        context.registerFunc("printFoo", printFoo);

        bool success = true;

        success &= context.callFunc("onInit");

        {
            lua_simple::FunctionCallArgs args;
            args.add(1.23f);
            args.add(2);
            success &= context.callFunc("onUpdate", args);
        }

        success &= context.getGlobalVariable("test_int32").readInt32(test_i);
        success &= context.getGlobalVariable("test_uint32").readUInt32(test_u);
        success &= context.getGlobalVariable("test_float").readFloat(test_f);
        success &= context.getGlobalVariable("test_string").readString(test_s);
        success &= context.getGlobalVariable("test_bytes").readBytes(4, test_bytes);

        {
            lua_simple::FunctionCallArgs args;
            args.add("dupa slonia");
            success &= context.callFunc("onTakeDamage", args);
        }

        context.registerFunc("testCFunc", test);
        context.registerFunc("testCFunc2", test2);
        //context.registerFunc("testCFunc3", test3);
        context.registerFunc("testCFunc4", test4);
        context.registerFunc("testCFunc5", test5);

        success &= context.callFunc("testMyTest");

        success &= context.getGlobalVariable("dupa").getVariableByKey("jeden").readInt32(test_i);

        success &= context.getGlobalVariable("test_int32").readInt32(test_i);
        success &= context.getGlobalVariable("test_uint32").readUInt32(test_u);
        success &= context.getGlobalVariable("test_float").readFloat(test_f);
        success &= context.getGlobalVariable("test_string").readString(test_s);
        success &= context.getGlobalVariable("test_bytes").readBytes(4, test_bytes);

        //success &= context.getGlobalVariable("test_int32").writeInt32(987);
        //success &= context.getGlobalVariable("test_int32").readInt32(test_i);

        MK_ASSERT(success);

        delete[] text;
    }
} g_LuaTest;
