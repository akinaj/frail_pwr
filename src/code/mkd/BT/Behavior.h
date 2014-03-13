#pragma once
#include "BT/Condition.h"
#include "utils.h"
#include "BT/BlackBoard.h"

class ActorAI;

namespace BT {

    enum Status {
        BH_INVALID,
        BH_SUCCESS,
        BH_FAILURE,
        BH_RUNNING,
        BH_ABORTED,
    };

    enum NodeType {
        BH_Action,
        BH_Selector,
        BH_PSelector,
        BH_Sequence,
        BH_Parallel,
        BH_Condition,
    };

    class Behavior {
    public:
        Behavior();
        virtual ~Behavior();

        virtual Status update() = 0;
        virtual void onInitialize(BlackBoard* bb) {};
        virtual void onTerminate(Status) {};

        Status tick(BlackBoard* bb);
        void reset();
        void abort();

        bool isTerminated() const;
        bool isRunning() const;
        Status getStatus() const;

        NodeType getType() const { return m_type; }
        void setType(NodeType val) { m_type = val; }
        float getUsefulness() const { return m_usefulness; }
        void setUsefulness(float val) { m_usefulness = val; }
    protected:
        BlackBoard* m_bb;
    private:
        Status m_Status;
        NodeType m_type;
        float m_usefulness;
    };

    class ConditionNode : public Behavior {
    public:
        ~ConditionNode();

        virtual Status update();
        virtual void onInitialize( BlackBoard* bb );
        virtual void onTerminate( Status );

        void addCondition(Condition* condition);
        bool validateConditions(BlackBoard* bb);

    private:
        boost::ptr_vector<Condition> m_conditions;
    };
}