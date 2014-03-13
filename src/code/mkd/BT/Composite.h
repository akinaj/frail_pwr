#pragma once
#include "Behavior.h"

namespace BT {
    typedef boost::shared_ptr<Behavior> pBehavior;

    class Composite : public Behavior {
    public:
        void addChild(Behavior* child);
        void removeChild(Behavior* child);
        void clearChildren();
    protected:
        typedef std::vector<pBehavior> Behaviors;
        Behaviors m_Children;
    };

    class Sequence : public Composite {
    public:
        virtual ~Sequence();
    protected:
        Behaviors::iterator m_CurrentChild;
        virtual void onInitialize(BlackBoard* bb);
        virtual Status update();
    };

    class Selector : public Composite {
    public:
        virtual ~Selector();
    protected:
        Behaviors::iterator m_CurrentChild;
        virtual void onInitialize(BlackBoard* bb);
        virtual Status update();
    };

    class PrioritySelector : public Composite {
    public:
        virtual ~PrioritySelector();
    protected:
        Behaviors::iterator m_CurrentChild;
        virtual void onInitialize(BlackBoard* bb);
        virtual Status update();
    };

    class Parallel : public Composite {
    public:
        enum Policy{
            RequireOne,
            RequireAll,
        };

        Parallel(Policy policy);
        virtual ~Parallel();
    protected:
        Policy m_policy;

        virtual Status update();
        virtual void onTerminate(Status status);
    };
}