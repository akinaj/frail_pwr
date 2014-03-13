#pragma once
#include "HTN\Parser.h"

namespace HTN {
	typedef std::map<std::string, aiVariant> State;

	class Planner{
	public:
		Planner();
		~Planner();

		void init(const mkString& methodsPath, const mkString& operatorsPath, const mkString& goalsPath);
		std::vector<pTask> getPlan(const std::string& methodsPath,const std::string& operatorsPath,const std::string& goalsPath);
		State& getWorldState() { return m_worldState; }

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

		bool seekPlan(pTask task, std::vector<pTask>& plan, State& state);
		pTask getMainGoal() const;
		bool isOperator(pTask task);
		std::vector<pMethod> findMethodsForGoal(pTask goal);
		bool isGoalSatisfied(State& state, pGoal goal);
	};
}