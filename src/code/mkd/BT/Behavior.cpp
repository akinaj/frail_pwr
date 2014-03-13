#include "pch.h"
#include "Behavior.h"

namespace BT {

    Behavior::Behavior() 
        : m_Status(BH_INVALID), m_usefulness(-1.f)
    {

    }

    Behavior::~Behavior()
    {

    }

    Status Behavior::tick(BlackBoard* bb)
    {
        if(m_Status != BH_RUNNING)
            onInitialize(bb);

        m_Status = update();

        if(m_Status != BH_RUNNING)
            onTerminate(m_Status);

        return m_Status;
    }

    void Behavior::reset()
    {
        m_Status = BH_INVALID;
    }

    void Behavior::abort()
    {
        onTerminate(BH_ABORTED);
        m_Status = BH_ABORTED;
    }

    bool Behavior::isTerminated() const
    {
        return m_Status == BH_SUCCESS || m_Status == BH_FAILURE;
    }

    bool Behavior::isRunning() const
    {
        return m_Status == BH_RUNNING;
    }

    Status Behavior::getStatus() const
    {
        return m_Status;
    }
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////

    ConditionNode::~ConditionNode()
    {
        
    }

    Status ConditionNode::update()
    {
        if(validateConditions(m_bb))
            return BH_SUCCESS;
        else
            return BH_FAILURE;
    }

    void ConditionNode::onInitialize( BlackBoard* bb )
    {
        m_bb = bb;
    }

    void ConditionNode::onTerminate( Status )
    {

    }

    void ConditionNode::addCondition(Condition* condition)
    {
        m_conditions.push_back(condition);
    }

    bool ConditionNode::validateConditions(BlackBoard* bb)
    {
        for(size_t i=0; i<m_conditions.size(); ++i){
            if(!m_conditions[i].validateCondition(bb))
                return false;
        }
        return true;
    }
}