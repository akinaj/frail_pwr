#pragma once
#include "utils.h"

namespace HTN {
	typedef bool(*preFunc)(aiVariant&, aiVariant&, bool&);
	typedef std::map<std::string, aiVariant> State;

	class Task {
	public:
		Task();
		virtual ~Task();
        virtual Task* clone() = 0;

        bool isParameterized();
        void clearParameters();
        void replaceParameters(std::vector<std::string>& parameters);
        void addParameter(std::string parameter);
		std::string getName() const { return m_name; }
        std::vector<std::string>& getParameters() { return m_parameters; }
        void setTempValues(State& state);
	protected:
		std::string m_name;
        std::vector<std::string> m_parameters;
    };

	class Condition {
	public:
		Condition(preFunc func, std::string& arg1, std::string& arg2);
		~Condition();

		bool validateCondition(State& state) const;
		std::string getArguments() { return m_arg1+";"+m_arg2; }
	private:
		preFunc m_func;
		std::string m_arg1, m_arg2;
	};
}