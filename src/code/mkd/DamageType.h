#pragma once

namespace EDamageType
{
    enum TYPE
    {
        Unknown = 0,
        Bullet,
        Fire,
        Poison,
        Punch,
        Collision,

        _COUNT,
        _FIRST = Unknown
    };

    static mkString toString(TYPE arg)
    {
#define MAKE_CASE(x) case x: return #x;
        switch(arg)
        {
            MAKE_CASE(Unknown);
            MAKE_CASE(Bullet);
            MAKE_CASE(Fire);
            MAKE_CASE(Poison);
            MAKE_CASE(Punch);
            MAKE_CASE(Collision);
        }
#undef MAKE_CASE

        return "";
    }
}
