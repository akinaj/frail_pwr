#include "pch.h"
#include "HTN\Goal.h"

namespace HTN {
	Goal::Goal(std::string& name, bool isMain) : m_isMain(isMain) {
		m_name = name;
	}

    Goal::Goal( Goal& other )
    {
        this->m_name = other.getName();
        this->m_parameters = other.getParameters();
        this->m_isMain = other.isMainGoal();
        this->m_postCond = other.getConditions();
    }

    Goal::~Goal(){}

    Goal* Goal::clone()
    {
        return new Goal(*this);
    }

	void Goal::addPostcond(pCond cond){
		m_postCond.push_back(cond);
	}

}