#pragma once
#include "utils.h"
#include "Behavior.h"
#include "Composite.h"
#include "Decorator.h"
#include "Actions.h"
#include "BlackBoard.h"

class ActorAI;

namespace BT {
    typedef bool(*preFunc)(aiVariant&, aiVariant&, bool&);

    class Parser {
    public:
        Parser();
        ~Parser();

        void parseXmlTree(std::string dirPath, ActorAI* ai, Behavior *&root);
        bool parseAliases(BlackBoard* bb);
        bool isFileModified(const std::string& filePath);
    private:
        bool parseNode(pugi::xml_node& xmlNode, Behavior* bhNode);
        bool parseComposite(pugi::xml_node& xmlNode, Composite* cmpNode);
        bool parseAction(pugi::xml_node& xmlNode, Action* actionNode);
        bool parseCondition(pugi::xml_node& xmlNode, ConditionNode* conditionNode);
        Behavior* createNode(pugi::xml_node& xmlNode);
        time_t m_lastModTime;

        std::vector<std::string> splitString(std::string input);

        pugi::xml_document *m_xmlDoc;
        std::map<std::string,preFunc> m_condMap;

        pugi::xml_node m_xmlRoot;

        ActorAI* m_AI;
    };

}