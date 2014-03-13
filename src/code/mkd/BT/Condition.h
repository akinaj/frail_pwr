#pragma once
#include "utils.h"
#include "BT/BlackBoard.h"

namespace BT {

    typedef bool(*preFunc)(aiVariant&, aiVariant&, bool&);
    class Condition {
    public:
        Condition(preFunc func, std::string arg1, std::string arg2);
        ~Condition();

        bool validateCondition(BlackBoard* bb) const;
    private:
        preFunc m_func;
        std::string m_arg1, m_arg2;
    };
}