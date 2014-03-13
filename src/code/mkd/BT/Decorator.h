#pragma once
#include "Behavior.h"

namespace BT {

    class Decorator : public Behavior {
    public:
        Decorator(Behavior* child);
    protected:
        Behavior* m_Child;
    };

}