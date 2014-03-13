#pragma once
#include "HTN\Task.h"

namespace HTN {
	typedef boost::shared_ptr<Condition> pCond;
	typedef boost::shared_ptr<Task> pTask;

	class Method {
	public:
		Method(std::string& name, float usefulness, bool runAll);
		virtual ~Method();

		bool checkConditions(const State& state) const;
        std::vector<pTask> getVerifiedSubtasks(std::vector<std::string>& parameters);

		float getUsefulness() const { return m_usefulness; }
		void addPrecond(pCond cond);
		std::vector<pCond>& getConditions() { return m_preCond; }
		void addGoal(pTask task);
		std::vector<pTask>& getGoals() { return m_goals; }
		void addSubtask(pTask task);
		std::vector<pTask>& getSubtasks() { return m_subtasks; }
		std::string getName() const { return m_name; }
		bool getRunAll() const { return m_runAll; }
	private:
		std::string m_name;
		float m_usefulness;
		bool m_runAll;
		std::vector<pTask> m_goals;
		std::vector<pTask> m_subtasks;
		std::vector<pCond> m_preCond;
	};
}