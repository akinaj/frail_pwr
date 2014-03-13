/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "mkAssert.h"
#include "utils.h"

namespace Assert
{
    EFailBehavior::TYPE DefaultHandler(const char* condition, const char* func, const char* msg, const char* file, int line)
    {
        static std::map<mkString, bool> s_IgnoredAsserts;

        char buff[2048] = {0};
        sprintf(buff, "%s:%d", file, line);

        if (s_IgnoredAsserts.find(mkString(buff)) != s_IgnoredAsserts.end())
            return EFailBehavior::Continue;

        sprintf(buff, "Assertion failed!\n\nCondition: %s\nFile: %s\nLine: %d\nFunction:\n%s\n\n%s\n", condition, file, line, func, msg ? msg : "");

        log_info(buff);
        int result = MessageBoxA(NULL, buff, "Assertion failed!", MB_ICONERROR | MB_ABORTRETRYIGNORE | MB_DEFBUTTON3);
        switch (result)
        {
        case IDABORT:
            ExitProcess(-1);
            break;

        case IDRETRY:
            return EFailBehavior::Halt;

        case IDIGNORE:
            if (MessageBoxA(NULL, "Ignore always?", "Ignore always?", MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                sprintf(buff, "%s:%d", file, line);
                s_IgnoredAsserts[mkString(buff)] = true;
            }

            return EFailBehavior::Continue;
        }

        return EFailBehavior::Continue;
    }
}

static Assert::Handler g_AssertHandler = &Assert::DefaultHandler;

Assert::Handler Assert::getHandler()
{
    return g_AssertHandler;
}

void Assert::setHandler( Handler new_handler )
{
    g_AssertHandler = new_handler;
}

Assert::EFailBehavior::TYPE Assert::reportFailure( const char* condition, const char* func, const char* file, int line, const char* msg, ... )
{
    char buff[2048];
    const char* message = NULL;
    if (msg != NULL)
    {        
        va_list args;
        va_start(args, msg);
        vsnprintf(buff, 2048, msg, args);
        va_end(args);

        message = buff;
    }

    return getHandler()(condition, func, message, file, line);
}
