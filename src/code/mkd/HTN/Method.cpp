#include "pch.h"
#include "HTN\Method.h"
#include "HTN\Operator.h"

namespace HTN {
	Method::Method(std::string& name, float usefulness, bool runAll) : m_name(name), m_usefulness(usefulness), m_runAll(runAll){
	}

	Method::~Method(){
	}

	bool Method::checkConditions(const State& state) const {
		for(size_t i=0; i<m_preCond.size(); ++i)
			if(!m_preCond[i]->validateCondition(const_cast<State&>(state)))
				return false;
		return true;
	}

    std::vector<pTask> Method::getVerifiedSubtasks( std::vector<std::string>& parameters )
    {
        std::vector<pTask> tasks(m_subtasks.size());

        for( size_t i = 0; i < m_subtasks.size(); ++i ){
            if(m_subtasks[i]->isParameterized()){
                pTask newTask(m_subtasks[i]->clone());
                newTask->replaceParameters(parameters);
                boost::shared_ptr<Operator> op = boost::dynamic_pointer_cast<HTN::Operator>(newTask);
                if(op)
                    op->replaceOutcome(parameters);
                tasks[i] = newTask;
            } else {
                tasks[i] = m_subtasks[i];
            }
        }

        return tasks;
    }

	void Method::addPrecond(pCond cond){
		m_preCond.push_back(cond);
	}

	void Method::addGoal(pTask task){
		m_goals.push_back(task);
	}

	void Method::addSubtask(pTask task){
		m_subtasks.push_back(task);
	}

}