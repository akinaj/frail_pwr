#include "pch.h"
#include "HTN\Operator.h"

namespace HTN {
    Operator::Operator(std::string& name, float duration, bool isInterruptible) 
        : m_duration(duration), m_isInterruptible(isInterruptible) {
            m_name = name;
    }

    Operator::Operator( Operator& other )
    {
        this->m_name = other.getName();
        this->m_parameters = other.getParameters();
        this->m_duration = other.getDuration();
        this->m_isInterruptible = other.isInterruptible();
        this->m_outcome = other.getOutcome();
        this->m_interruptVect = other.getInterruptions();
    }

    Operator::~Operator(){}

    Operator* Operator::clone()
    {
        return new Operator(*this);
    }

    void Operator::addOutcome(std::pair<std::string, std::string> outcome){
        m_outcome.push_back(outcome);
    }

    void Operator::applyOutcome(State& state){
        for(size_t i = 0; i < m_outcome.size(); ++i){
            state[m_outcome[i].first] = state[m_outcome[i].second];
        }
    }

    void Operator::addInterruption(std::string& operatorName){
        m_interruptVect.push_back(operatorName);
    }

    void Operator::replaceOutcome( std::vector<std::string>& parameters )
    {
        std::string firstValue, secondValue;
        std::vector<std::pair<std::string, std::string>> tempOutcome = m_outcome;
        m_outcome.clear();

        for(size_t i = 0; i < tempOutcome.size(); ++i){
            firstValue = tempOutcome[i].first;
            secondValue = tempOutcome[i].second;
            if(tempOutcome[i].first.at(0) == '$'){
                firstValue.erase(firstValue.begin());
                firstValue = parameters[std::atoi(firstValue.c_str())];
            }
            if(tempOutcome[i].second.at(0) == '$'){
                secondValue.erase(secondValue.begin());
                secondValue = parameters[std::atoi(secondValue.c_str())];
            }
            m_outcome.push_back(std::make_pair(firstValue,secondValue));
        }
    }

}