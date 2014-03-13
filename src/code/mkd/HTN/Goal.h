#pragma once
#include "HTN\Task.h"

namespace HTN {
	typedef boost::shared_ptr<Condition> pCond;

	class Goal : public Task {
	public:
		explicit Goal(std::string& name, bool isMain);
        Goal( Goal& other );
		virtual ~Goal();
        virtual Goal* clone();
		
		void addPostcond(pCond cond);
		std::vector<pCond>& getConditions() { return m_postCond; }
		bool isMainGoal() const { return m_isMain; }
	private:
		bool m_isMain;
		std::vector<pCond> m_postCond;
	};
}