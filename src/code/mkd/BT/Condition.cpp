#include "pch.h"
#include "Condition.h"

namespace BT {

    Condition::Condition(preFunc func, std::string arg1, std::string arg2) 
        : m_func(func), m_arg1(arg1), m_arg2(arg2) {}
    Condition::~Condition() {}

    bool Condition::validateCondition(BlackBoard* bb) const {
        aiVariant v1, v2;
        v1 = bb->getStateVariant(m_arg1);
        v2 = bb->getStateVariant(m_arg2);
        bool result;
        m_func(v1,v2, result);
        return result;
    }

}