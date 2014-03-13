#pragma once
#include "HTN\Task.h"

namespace HTN {
	class Operator : public Task {
	public:
		Operator(std::string& name, float duration, bool isInterruptible, bool isAnim);
        Operator( Operator& other );
        virtual ~Operator();
        virtual Operator* clone();

		void addOutcome(std::pair<std::string, std::string> outcome);
		std::vector<std::pair<std::string, std::string>>& getOutcome() { return m_outcome; }
		void applyOutcome(State& state);
        void replaceOutcome(std::vector<std::string>& parameters);

		float getDuration() const { return m_duration; }
		bool isInterruptible() const { return m_isInterruptible; }
        std::vector<std::string>& getInterruptions() { return m_interruptVect; }
        void addInterruption(std::string& operatorName);
        bool isAnim() const { return m_isAnim; }
	private:
        bool m_isAnim;
        float m_duration;
		bool m_isInterruptible;
		std::vector<std::pair<std::string, std::string>> m_outcome;
        std::vector<std::string> m_interruptVect;
	};
}