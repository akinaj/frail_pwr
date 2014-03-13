#include "pch.h"
#include "HTN\Parser.h"
#include <cstring>

namespace HTN {
    Parser::Parser(){
        m_xmlDoc = new pugi::xml_document;

        m_condMap["Equal"] = &equal;
        m_condMap["NotEqual"] = &notEqual;
        m_condMap["More"] = &more;
        m_condMap["MoreEqual"] = &moreEqual;
        m_condMap["Less"] = &less;
        m_condMap["LessEqual"] = &lessEqual;
    }

    Parser::~Parser(){
        delete m_xmlDoc;
    }

    std::vector<std::string> Parser::splitString(std::string input){
        std::vector<std::string> result;
        result.reserve(3);

        std::string delimiter = " ";

        size_t pos = 0;
        std::string token;
        while ((pos = input.find(delimiter)) != std::string::npos) {
            token = input.substr(0, pos);
            result.push_back(token);
            input.erase(0, pos + delimiter.length());
        }
        result.push_back(input);

        return result;
    }

    bool Parser::parseAliases(const std::string& dirPath, State& worldState )
    {
        pugi::xml_parse_result loadResult = m_xmlDoc->load_file(dirPath.c_str());

        if(!loadResult)
            return false;

        std::vector<std::string> aliasSplit(2);
        pugi::xml_node aliasesNode = m_xmlDoc->child("aliases");
        for(pugi::xml_node alias = aliasesNode.first_child(); alias; alias = alias.next_sibling()){
            aliasSplit.clear();
            aliasSplit = splitString(alias.child_value());
            bool bval;
            float fval;

            //bools
            try
            {
                bval = boost::lexical_cast<bool>(aliasSplit[1]);
                worldState[aliasSplit[0]] = bval;
                continue;
            }
            catch (boost::bad_lexical_cast&) {}

            if(aliasSplit[1] == "true"){
                bval = true;
                worldState[aliasSplit[0]] = bval;
                continue;
            } else if(aliasSplit[1] == "false"){
                bval = false;
                worldState[aliasSplit[0]] = bval;
                continue;
            }

            //floats
            try
            {
                fval = boost::lexical_cast<float>(aliasSplit[1]);
                worldState[aliasSplit[0]] = fval;
                continue;
            }
            catch (boost::bad_lexical_cast&) {}

            //worldState values
            State::const_iterator it = worldState.find(aliasSplit[1]);
            if(it != worldState.end()){
                worldState[aliasSplit[0]] = worldState[aliasSplit[1]];
                continue;
            }

            return false;
        }
        return true;
    }

    bool Parser::parseGoals(const std::string& dirPath, std::vector<pTask>& goals, std::vector<pTask>& operators){
        if(goals.size() > 0)
            return false;

        m_goalLastMod = lastModifiedTime(dirPath);

        pugi::xml_parse_result loadResult = m_xmlDoc->load_file(dirPath.c_str());

        if(!loadResult)
            return false;

        std::string taskName;
        bool isMain = false;
        std::vector<std::string> condSplit(3);

        pugi::xml_node xmlTasks = m_xmlDoc->child("tasks");
        for(pugi::xml_node xmlTask = xmlTasks.first_child(); xmlTask; xmlTask = xmlTask.next_sibling()){
            // name
            if(xmlTask.attribute("name").empty())
                return false;
            taskName = xmlTask.attribute("name").value();

            if(xmlTask.attribute("main").empty())
                isMain = false;
            else
                isMain = std::string(xmlTask.attribute("main").value()) != "0";

            pTask task(new Goal(taskName, isMain));
            goals.push_back(task);

            for(pugi::xml_node postCond = xmlTask.child("postconditions").first_child(); postCond; postCond = postCond.next_sibling()){
                condSplit.clear();
                condSplit = splitString(postCond.child_value());
                pCond cond(new Condition(m_condMap[condSplit[0]], condSplit[1], condSplit[2]));
                boost::dynamic_pointer_cast<HTN::Goal>(task)->addPostcond(cond);
            }
        }
        goals.insert(goals.end(), operators.begin(), operators.end());
        return true;
    }

    bool Parser::parseOperators(const std::string& dirPath, std::vector<pTask>& operators){
        if(operators.size() > 0)
            return false;

        m_operatorLastMod = lastModifiedTime(dirPath);

        pugi::xml_parse_result loadResult = m_xmlDoc->load_file(dirPath.c_str());

        if(!loadResult)
            return false;

        std::string taskName;
        float duration = 20.f;
        bool interruptible = true;
        bool isAnim = false;
        std::vector<std::string> outSplit(2);

        pugi::xml_node xmlTasks = m_xmlDoc->child("tasks");
        for(pugi::xml_node xmlTask = xmlTasks.first_child(); xmlTask; xmlTask = xmlTask.next_sibling()){
            // name
            if(xmlTask.attribute("name").empty())
                return false;
            taskName = xmlTask.attribute("name").value();

            if(!xmlTask.attribute("duration").empty())
                duration = (float)atof(xmlTask.attribute("duration").value());
            if(!xmlTask.attribute("interruptible").empty())
                interruptible = std::string(xmlTask.attribute("interruptible").value()) != "0";
            if(!xmlTask.attribute("isAnim").empty())
                isAnim = std::string(xmlTask.attribute("isAnim").value()) != "0";

            pTask task(new Operator(taskName, duration, interruptible, isAnim));
            operators.push_back(task);

            for(pugi::xml_node outcome = xmlTask.child("outcome").first_child(); outcome; outcome = outcome.next_sibling()){
                outSplit.clear();
                outSplit = splitString(outcome.child_value());

                boost::dynamic_pointer_cast<HTN::Operator>(task)->addOutcome(std::make_pair(outSplit[0],outSplit[1]));
            }

            for(pugi::xml_node interrupt = xmlTask.child("interruptions").first_child(); interrupt; interrupt = interrupt.next_sibling()){	
                boost::dynamic_pointer_cast<HTN::Operator>(task)->addInterruption(std::string(interrupt.child_value()));
            }
        }
        return true;
    }

    bool Parser::parseMethods(const std::string& dirPath, std::vector<pMethod>& methods, const std::vector<pTask>& goals, const std::vector<pTask>& operators){
        if(methods.size() > 0)
            return false;

        m_methodLastMod = lastModifiedTime(dirPath);

        pugi::xml_parse_result loadResult = m_xmlDoc->load_file(dirPath.c_str());

        if(!loadResult)
            return false;

        std::string taskName;
        float usefulness = 0.f;
        bool runAll = true;
        std::vector<std::string> condSplit(3);
        pTask tempTask;
        bool found = false;

        pugi::xml_node xmlTasks = m_xmlDoc->child("methods");
        for(pugi::xml_node xmlTask = xmlTasks.first_child(); xmlTask; xmlTask = xmlTask.next_sibling()){
            // name
            if(xmlTask.attribute("name").empty())
                return false;
            taskName = xmlTask.attribute("name").value();

            if(!xmlTask.attribute("usefulness").empty())
                usefulness = (float)atof(xmlTask.attribute("usefulness").value());

            if(!xmlTask.attribute("runAll").empty())
                runAll = std::string(xmlTask.attribute("runAll").value()) != "0";

            pMethod task(new Method(taskName, usefulness, runAll));
            methods.push_back(task);

            for(pugi::xml_node goal = xmlTask.child("goals").first_child(); goal; goal = goal.next_sibling()){
                found = false;
                for(size_t i=0; i<goals.size(); ++i){
                    if(std::strcmp(goals[i]->getName().c_str(), goal.child_value()) == 0){
                        tempTask = goals[i];
                        found = true;
                        break;
                    }
                }
                if(found)
                    boost::dynamic_pointer_cast<HTN::Method>(task)->addGoal(tempTask);
            }

            for(pugi::xml_node preCond = xmlTask.child("preconditions").first_child(); preCond; preCond = preCond.next_sibling()){
                condSplit.clear();
                condSplit = splitString(preCond.child_value());
                pCond cond(new Condition(m_condMap[condSplit[0]], condSplit[1], condSplit[2]));
                boost::dynamic_pointer_cast<HTN::Method>(task)->addPrecond(cond);
            }

            for(pugi::xml_node subtask = xmlTask.child("subtasks").first_child(); subtask; subtask = subtask.next_sibling()){
                condSplit.clear();
                condSplit = splitString(subtask.child_value());

                found = false;
                //GOALS////////////////////////////////////////////////////////////////////////
                for(size_t i=0; i<goals.size(); ++i){
                    if(std::strcmp(goals[i]->getName().c_str(), condSplit[0].c_str()) == 0){
                        tempTask = goals[i];
                        found = true;
                        break;
                    }
                }
                //OPERATORS////////////////////////////////////////////////////////////////////////
                for(size_t i=0; i<operators.size(); ++i){
                    if(std::strcmp(operators[i]->getName().c_str(), condSplit[0].c_str()) == 0){
                        tempTask = operators[i];
                        found = true;
                        break;
                    }
                }

                if(found){
                    pTask newTask(tempTask->clone());
                    newTask->clearParameters();
                    for(size_t i=1; i<condSplit.size(); ++i){
                        newTask->addParameter(condSplit[i]);
                    }

                    boost::dynamic_pointer_cast<HTN::Method>(task)->addSubtask(newTask);
                }
            }
        }
        return true;
    }

    bool Parser::isFileModified( const std::string& methodsPath, const std::string& operatorsPath, const std::string& goalsPath )
    {
        if(m_methodLastMod == lastModifiedTime(methodsPath) 
            && m_goalLastMod == lastModifiedTime(goalsPath) 
            && m_operatorLastMod == lastModifiedTime(operatorsPath)){
                return false;
        } else {
            return true;
        }
    }



}