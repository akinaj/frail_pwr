#pragma once
#include "HTN\Parser.h"

class HTNActorController;

namespace HTN {
    enum PlanResult {
        PLAN_INTERRUPTED,
        PLAN_EMPTYPLAN,
        PLAN_RUNNING,
        PLAN_NEW,
    };

    typedef std::map<std::string, aiVariant> State;

    class Planner{
    public:
        Planner();
        ~Planner();

        void init(const mkString& methodsPath, const mkString& operatorsPath, const mkString& goalsPath);
        std::vector<pTask> getPlan(const std::string& methodsPath,const std::string& operatorsPath,const std::string& goalsPath);
        State& getWorldState() { return m_worldState; }
        PlanResult resolvePlan(std::vector<HTN::pTask>& plan, float dt, HTN::pOperator& newTask);

        bool contains( std::string key );
        void setStateVec3( std::string key, mkVec3 vec3 );
        mkVec3 getStateVec3( std::string key, bool& isValid );
        void setStateBool( std::string key, bool val );
        bool getStateBool( std::string key, bool& isValid );
        void setStateFloat( std::string key, float val );
        float getStateFloat( std::string key, bool& isValid );
        void setStateVariant( std::string key, aiVariant val );
        aiVariant getStateVariant( std::string key );
    private:
        Parser *m_parser;
        std::vector<pTask> m_goals;
        std::vector<pMethod> m_methods;
        std::vector<pTask> m_operators;
        State m_worldState;

        int m_currentIdx;
        float m_taskDuration;
        bool m_isTaskExecuted;
        HTN::pOperator m_currentTask;

        bool seekPlan(pTask task, std::vector<pTask>& plan, State& state);
        pTask getMainGoal() const;
        bool isOperator(pTask task);
        std::vector<pMethod> findMethodsForGoal(pTask goal);
        bool isGoalSatisfied(State& state, pGoal goal);

        bool outcomeValidation(HTN::pOperator op);
        bool isOperatorInterrupted(HTN::pOperator current, HTN::pOperator next);
    };
}