#include "pch.h"
#include "Composite.h"
#include <sstream>
#include <algorithm>

namespace BT {

    void Composite::addChild( Behavior* child )
    {
        m_Children.push_back(pBehavior(child));
    }

    void Composite::removeChild( Behavior* child )
    {
        for(Behaviors::iterator it = m_Children.begin(); it != m_Children.end(); ++it){
            Behavior* b = it->get();
            if(b == child)
                m_Children.erase(it);
        }
    }

    void Composite::clearChildren()
    {
        m_Children.clear();
    }

    //////////////////////////////////////////////////////////////////////////

    Sequence::~Sequence()
    {

    }

    void Sequence::onInitialize(BlackBoard* bb)
    {
        m_bb = bb;
        m_CurrentChild = m_Children.begin();
    }

    Status Sequence::update()
    {
        while(true){
            Status s = (*m_CurrentChild)->tick(m_bb);

            if(s != BH_SUCCESS)
                return s;

            if(++m_CurrentChild == m_Children.end())
                return BH_SUCCESS;
        }
    }

    //////////////////////////////////////////////////////////////////////////

    Selector::~Selector()
    {

    }

    void Selector::onInitialize(BlackBoard* bb)
    {
        m_bb = bb;
        m_CurrentChild = m_Children.begin();
    }

    Status Selector::update()
    {
        while(true){
            Status s = (*m_CurrentChild)->tick(m_bb);

            if(s != BH_FAILURE)
                return s;

            if(++m_CurrentChild == m_Children.end())
                return BH_FAILURE;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    bool behaviorPredicate(pBehavior &a, pBehavior &b)
    {
        return a->getUsefulness() > b->getUsefulness();
    }

    PrioritySelector::~PrioritySelector()
    {

    }

    void PrioritySelector::onInitialize(BlackBoard* bb)
    {
        std::sort(m_Children.begin(),m_Children.end(),behaviorPredicate);

        m_bb = bb;
        m_CurrentChild = m_Children.begin();
    }

    Status PrioritySelector::update()
    {
        while(true){
            Status s = (*m_CurrentChild)->tick(m_bb);

            if(s != BH_FAILURE){
                return s;
            }

            if(++m_CurrentChild == m_Children.end()){
                return BH_FAILURE;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////

    Parallel::Parallel(Policy policy) : m_policy(policy)
    {

    }

    Parallel::~Parallel()
    {

    }

    Status Parallel::update()
    {
        size_t successCount = 0;

        for(Behaviors::iterator it = m_Children.begin(); it != m_Children.end(); ++it){
            Behavior* b = it->get();

            if(!b->isTerminated())
                b->tick(m_bb);

            Status s = b->getStatus();
            if(s == BH_SUCCESS){
                ++successCount;
                if(m_policy == RequireOne)
                    return BH_SUCCESS;
            } else if(s == BH_FAILURE) {
                if(m_policy == RequireAll)
                    return BH_FAILURE;
            }
        }

        if(successCount = m_Children.size() && m_policy == RequireAll)
            return BH_SUCCESS;

        return BH_RUNNING;
    }

    void Parallel::onTerminate( Status status )
    {
        for(Behaviors::iterator it = m_Children.begin(); it != m_Children.end(); ++it){
            Behavior* b = it->get();
            if(b->isRunning())
                b->abort();
        }
    }

}