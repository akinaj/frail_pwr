#pragma once

// Based on http://cnicholson.net/2009/02/stupid-c-tricks-adventures-in-assert/

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_ASSERTS)
#define _IMPL_ENABLE_ASSERT
#endif

#define MK_UNUSED(x) (void)sizeof(x)
#define MK_HALT() __debugbreak()

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#ifndef __PRETTY_FUNCTION__
#defin __PRETTY_FUNCTION__ __FUNCTION__
#endif

namespace Assert
{
    namespace EFailBehavior
    {
        enum TYPE
        {
            Halt,
            Continue,
        };
    }

    typedef EFailBehavior::TYPE (*Handler)(const char* condition, const char* func, const char* msg, const char* file, int line);

    Handler getHandler();
    void setHandler(Handler new_handler);

    EFailBehavior::TYPE reportFailure(const char* condition, const char* func, const char* file, int line, const char* msg, ...);
}


#ifdef _IMPL_ENABLE_ASSERT

#define MK_ASSERT(cond) do {                                                                                        \
                            if (!(cond)) {                                                                          \
                                if (Assert::reportFailure(#cond, __PRETTY_FUNCTION__, __FILE__, __LINE__, NULL)     \
                                    == Assert::EFailBehavior::Halt) {                                               \
                                    MK_HALT();                                                                      \
                                }                                                                                   \
                            }                                                                                       \
                        } while (false)

#define MK_ASSERT_MSG(cond, msg, ...) do {                                                                                      \
                            if (!(cond)) {                                                                                      \
                                if (Assert::reportFailure(#cond, __PRETTY_FUNCTION__, __FILE__, __LINE__, msg, ##__VA_ARGS__)   \
                                    == Assert::EFailBehavior::Halt) {                                                           \
                                    MK_HALT();                                                                                  \
                                }                                                                                               \
                            }                                                                                                   \
                        } while (false)

#else

#define MK_ASSERT(cond) do { MK_UNUSED(cond); } while (false)
#define MK_ASSERT_MSG(cond, fmt, ...) do { MK_UNUSED(fmt); } while (false)

#endif
