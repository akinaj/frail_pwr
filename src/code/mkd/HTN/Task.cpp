#include "pch.h"
#include "HTN\Task.h"

namespace HTN {
	Task::Task(){ m_last = false; }
	Task::~Task(){}

    void Task::setTempValues(State& state)
    {
        std::string key = "";
        for(size_t i=0; i<m_parameters.size(); ++i){
            key = "$" + boost::lexical_cast<std::string>(i);
            state[key] = state[m_parameters[i]];
        }
    }

    void Task::addParameter( std::string parameter )
    {
        this->m_parameters.push_back(parameter);
    }

    bool Task::isParameterized()
    {
        bool isParameterized = false;
        std::vector<std::string> params = getParameters();
        for( size_t j = 0; j < params.size(); ++j ){
            if(params[j].size() > 0 && params[j].at(0) == '$' ){
                isParameterized = true;
                break;
            }
        }

        return isParameterized;
    }

    void Task::replaceParameters( std::vector<std::string>& parameters )
    {
        std::vector<std::string> params = getParameters();
        std::string parameter;
        m_parameters.clear();
        for( size_t j = 0; j < params.size(); ++j ){
            if(params[j].size() > 0){
                if(params[j].at(0) == '$' ){
                    parameter = params[j];
                    parameter.erase(parameter.begin());
                    m_parameters.push_back(parameters[std::atoi(parameter.c_str())]);
                } else {
                    m_parameters.push_back(params[j]);
                }
            }
        }
    }

    void Task::clearParameters()
    {
        m_parameters.clear();
    }

	Condition::Condition(preFunc func, std::string& arg1, std::string& arg2) : m_func(func), m_arg1(arg1), m_arg2(arg2) {}
	Condition::~Condition() {}

	bool Condition::validateCondition(State& state) const {
		aiVariant v1, v2;
		v1 = state[m_arg1];
		v2 = state[m_arg2];
		bool result;
        m_func(v1,v2, result);
		return result;
	}
}