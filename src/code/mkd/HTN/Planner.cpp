#include "pch.h"
#include "HTN\Planner.h"

namespace HTN {
    Planner::Planner(){
        m_parser = new Parser();
    }
    Planner::~Planner(){
        delete m_parser;
    }

    void Planner::init(const mkString& methodsPath, const mkString& operatorsPath, const mkString& goalsPath){
        m_parser->parseOperators(operatorsPath, m_operators);
        m_parser->parseGoals(goalsPath, m_goals, m_operators);
        m_parser->parseMethods(methodsPath, m_methods, m_goals, m_operators);

        //const////////////////////////////////////////////////////////////////////////
        m_worldState["True"] = true;
        m_worldState["False"] = false;
        m_worldState["Zero"] = 0.f;

        m_parser->parseAliases(methodsPath,m_worldState);
    }

    std::vector<pTask> Planner::getPlan(const std::string& methodsPath,const std::string& operatorsPath,const std::string& goalsPath){
        if(m_parser->isFileModified(methodsPath,operatorsPath,goalsPath)){
            m_operators.clear(); m_methods.clear(); m_goals.clear();
            m_parser->parseOperators(operatorsPath, m_operators);
            m_parser->parseGoals(goalsPath, m_goals, m_operators);
            m_parser->parseMethods(methodsPath, m_methods, m_goals, m_operators);
            m_parser->parseAliases(methodsPath,m_worldState);
        }

        std::vector<pTask> plan, smallPlan;
        State state = m_worldState;
        pTask mainGoal = getMainGoal();
        if(mainGoal){
            while(!isGoalSatisfied(state, boost::dynamic_pointer_cast<Goal>(mainGoal))){
                smallPlan.clear();
                if(seekPlan(mainGoal, smallPlan, state))
                    plan.insert(plan.end(), smallPlan.begin(), smallPlan.end());

                if(plan.empty()){
                    Ogre::LogManager::getSingleton().logMessage("Cannot create HTN plan!");
                    break;
                }
            }

            std::stringstream ss;
            for(int i=0; i<plan.size(); ++i){
                ss << plan[i]->getName() << " ";
                for(int j=0; j<plan[i]->getParameters().size(); ++j){
                    ss << plan[i]->getParameters()[j] << " ";
                }
                ss << ";";
            }
            Ogre::LogManager::getSingleton().logMessage(ss.str());
        }

        return plan;
    }

    bool Planner::seekPlan(pTask task, std::vector<pTask>& plan, State& state){
        if(isOperator(task)){
            plan.push_back(task);
            boost::dynamic_pointer_cast<HTN::Operator>(task)->applyOutcome(state);
            return true;
        }
        std::vector<pMethod> methods = findMethodsForGoal(task);
        task->setTempValues(state);

        if(methods.empty())
            return false;

        State stateBackup;
        float bestUsefulness = -1.f;
        std::vector<pTask> operators;
        std::vector<pTask> subOps;
        std::vector<pTask> subtasks;
        bool possibility = false;
        for(size_t i=0; i<methods.size(); ++i){
            if(methods[i]->getUsefulness() < bestUsefulness)
                break;

            if(!methods[i]->checkConditions(state))
                continue;

            subtasks = methods[i]->getVerifiedSubtasks(task->getParameters());
            //subtasks = methods[i]->getSubtasks();

            stateBackup = state;

            if(!methods[i]->getRunAll()){
                possibility = false;
                subOps.clear();
                for(size_t j=0; j<subtasks.size(); j++){
                    if(seekPlan(subtasks[j],subOps,state)){
                        possibility = true;
                    }
                }
            } else {
                possibility = true;
                subOps.clear();
                for(size_t j=0; j<subtasks.size(); j++){
                    if(!seekPlan(subtasks[j],subOps,state)){
                        possibility = false;
                        break;
                    }
                }
            }

            if(possibility){
                operators.insert(operators.end(), subOps.begin(), subOps.end());
                bestUsefulness = methods[i]->getUsefulness();
            }
            else
                state = stateBackup;
        }

        plan.insert(plan.end(), operators.begin(), operators.end());

        return bestUsefulness >= 0.f;
    }

    bool Planner::isOperator(pTask task){
        for(size_t i=0; i<m_operators.size(); ++i){
            if(task->getName() == m_operators[i]->getName())
                return true;
        }
        return false;
    }

    bool methodComparator(const HTN::pMethod& first, const HTN::pMethod& second){
        return (first->getUsefulness() > second->getUsefulness());
    }

    std::vector<pMethod> Planner::findMethodsForGoal(pTask goal){
        std::vector<pMethod> result;

        pMethod method;
        std::vector<pTask> methodGoals;
        for(size_t i=0; i<m_methods.size(); ++i){
            method = m_methods[i];
            methodGoals = method->getGoals();
            for(size_t j=0; j<methodGoals.size(); ++j){
                if(methodGoals[j]->getName() == goal->getName()){
                    result.push_back(method);
                    break;
                }
            }
        }

        std::sort(result.begin(), result.end(), methodComparator);

        return result;
    }

    pTask Planner::getMainGoal() const {
        assert(m_goals.size() > 0);
        pTask result;
        for(size_t i=0; i<m_goals.size()-m_operators.size(); ++i)
            if(boost::dynamic_pointer_cast<Goal>(m_goals[i])->isMainGoal()){
                result = m_goals[i];
            }
            return result;
    }

    bool Planner::isGoalSatisfied(State& state, pGoal goal){
        std::vector<pCond> conditions = goal->getConditions();
        for(size_t i=0; i<conditions.size(); ++i){
            if(!conditions[i]->validateCondition(state))
                return false;
        }
        return true;
    }

    //////////////////////////////////////////////////////////////////////////

    bool Planner::contains( std::string key ){
        std::map<std::string, aiVariant>::const_iterator it = m_worldState.find(key);
        if(it != m_worldState.end())
            return true;
        return false;
    }

    void Planner::setStateVec3( std::string key, mkVec3 vec3 )
    {
        m_worldState[key] = vec3;
    }

    mkVec3 Planner::getStateVec3( std::string key, bool& isValid )
    {
        try{
            mkVec3 res;
            res = boost::get<mkVec3>(m_worldState[key]);
            isValid = true;
            return res;
        } catch (boost::bad_get&){ Ogre::LogManager::getSingleton().logMessage("getStateVec3: bad_get exception!"); }
        isValid = false;
        return mkVec3::ZERO;
    }

    void Planner::setStateBool( std::string key, bool val )
    {
        m_worldState[key] = val;
    }

    bool Planner::getStateBool( std::string key, bool& isValid )
    {
        try{
            bool res;
            res = boost::get<bool>(m_worldState[key]);
            isValid = true;
            return res;
        } catch (boost::bad_get&){ Ogre::LogManager::getSingleton().logMessage("getStateBool: bad_get exception!"); }
        isValid = false;
        return false;
    }

    void Planner::setStateFloat( std::string key, float val )
    {
        m_worldState[key] = val;
    }

    float Planner::getStateFloat( std::string key, bool& isValid )
    {
        try{
            float res;
            res = boost::get<float>(m_worldState[key]);
            isValid = true;
            return res;
        } catch (boost::bad_get&){ Ogre::LogManager::getSingleton().logMessage("getStateFloat: bad_get exception!"); }
        isValid = false;
        return -1.f;
    }

    void Planner::setStateVariant( std::string key, aiVariant val )
    {
        m_worldState[key] = val;
    }

    aiVariant Planner::getStateVariant( std::string key )
    {
        return m_worldState[key];
    }
}