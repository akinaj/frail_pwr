#pragma once
#include "HTN\Goal.h"
#include "HTN\Method.h"
#include "HTN\Operator.h"
#include "utils.h"

namespace HTN {
	typedef boost::shared_ptr<Task> pTask;
	typedef boost::shared_ptr<Method> pMethod;
	typedef boost::shared_ptr<Operator> pOperator;
	typedef boost::shared_ptr<Goal> pGoal;
	typedef bool(*preFunc)(aiVariant&, aiVariant&, bool&);

	class Parser {
	public:
		Parser();
		~Parser();

        bool isFileModified( const std::string& methodsPath, const std::string& operatorsPath, const std::string& goalsPath );
		bool parseGoals(const std::string& dirPath, std::vector<pTask>& goals, std::vector<pTask>& operators);
		bool parseOperators(const std::string& dirPath, std::vector<pTask>& operators);
		bool parseMethods(const std::string& dirPath, std::vector<pMethod>& methods, const std::vector<pTask>& goals, const std::vector<pTask>& operators);
        bool parseAliases(const std::string& dirPath, State& worldState);
	private:
		std::vector<std::string> splitString(std::string input);

		pugi::xml_document *m_xmlDoc;
		std::map<std::string,preFunc> m_condMap;
        time_t m_methodLastMod;
        time_t m_goalLastMod;
        time_t m_operatorLastMod;
	};
}