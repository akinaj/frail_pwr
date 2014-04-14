#include "pch.h"
#include "Parser.h"

namespace BT {

    Parser::Parser()
    {
        m_xmlDoc = new pugi::xml_document;

        m_condMap["Equal"] = &equal;
        m_condMap["NotEqual"] = &notEqual;
        m_condMap["More"] = &more;
        m_condMap["MoreEqual"] = &moreEqual;
        m_condMap["Less"] = &less;
        m_condMap["LessEqual"] = &lessEqual;
    }

    Parser::~Parser()
    {
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

    void Parser::parseXmlTree( std::string dirPath, ActorAI* ai, Behavior *&root)
    {
        m_lastModTime = lastModifiedTime(dirPath);
        m_AI = ai;
        assert(dirPath.size() > 0);
        pugi::xml_parse_result loadResult = m_xmlDoc->load_file(dirPath.c_str());

        if(!loadResult)
            return;

        m_xmlRoot = m_xmlDoc->child("node");
        root = createNode(m_xmlRoot);
        if(!root)
            return;

        parseNode(m_xmlRoot,root);
    }

    bool Parser::isFileModified( const std::string& filePath )
    {
        if(m_lastModTime == lastModifiedTime(filePath)){
            return false;
        } else {
            return true;
        }
    }

    bool Parser::parseAliases( BlackBoard* bb )
    {
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
                bb->setStateBool(aliasSplit[0],bval);
                continue;
            }
            catch (boost::bad_lexical_cast&) {}

            if(aliasSplit[1] == "true"){
                bval = true;
                bb->setStateBool(aliasSplit[0],bval);
                continue;
            } else if(aliasSplit[1] == "false"){
                bval = false;
                bb->setStateBool(aliasSplit[0],bval);
                continue;
            }

            //floats
            try
            {
                fval = boost::lexical_cast<float>(aliasSplit[1]);
                bb->setStateFloat(aliasSplit[0],fval);
                continue;
            }
            catch (boost::bad_lexical_cast&) {}

            //worldState values
            if(bb->contains(aliasSplit[1])){
                bb->setStateVariant(aliasSplit[0],bb->getStateVariant(aliasSplit[1]));
                continue;
            }
            return false;
        }
        return true;
    }

    bool Parser::parseNode( pugi::xml_node& xmlNode, Behavior* bhNode )
    {
        if(bhNode->getType() == BH_Action){
            Action *bhAction = dynamic_cast<Action *>(bhNode);
            if (NULL != bhAction)
            {
                //Ogre::LogManager::getSingleton().logMessage("Action");
                parseAction(xmlNode, bhAction);
            }
        } else if(bhNode->getType() == BH_Selector){
            Selector *bhSelector = dynamic_cast<Selector *>(bhNode);
            if (NULL != bhSelector)
            {
                //Ogre::LogManager::getSingleton().logMessage("Selector");
                parseComposite(xmlNode, bhSelector);
            }
        } else if(bhNode->getType() == BH_PSelector){
            PrioritySelector *bhPriority = dynamic_cast<PrioritySelector *>(bhNode);
            if (NULL != bhPriority)
            {
                //Ogre::LogManager::getSingleton().logMessage("pSelector");
                parseComposite(xmlNode, bhPriority);
            }
        } else if(bhNode->getType() == BH_Sequence){
            Sequence *bhSeq = dynamic_cast<Sequence *>(bhNode);
            if (NULL != bhSeq)
            {
                //std::cout << "Seq" << std::endl;
                parseComposite(xmlNode, bhSeq);
            }
        } else if(bhNode->getType() == BH_Parallel) {
            Parallel *bhParallel = dynamic_cast<Parallel *>(bhNode);
            if (NULL != bhParallel)
            {
                //std::cout << "Parallel" << std::endl;
                parseComposite(xmlNode, bhParallel);
            }
        } else if(bhNode->getType() == BH_Condition) {
            ConditionNode *bhCondition = dynamic_cast<ConditionNode *>(bhNode);
            if (NULL != bhCondition)
            {
                //std::cout << "Condition" << std::endl;
                parseCondition(xmlNode, bhCondition);
            }
        }
        return true;
    }

    bool Parser::parseComposite(pugi::xml_node& xmlNode, Composite* cmpNode)
    {
        if(!xmlNode.attribute("usefulness").empty())
            cmpNode->setUsefulness((float)atof(xmlNode.attribute("usefulness").value()));

        for(pugi::xml_node node = xmlNode.first_child(); node; node = node.next_sibling()){
            Behavior* bhChild = createNode(node);
            if(bhChild){
                cmpNode->addChild(bhChild);
                parseNode(node,bhChild);
            }
        }
        return true;
    }

    bool Parser::parseAction( pugi::xml_node& xmlNode, Action* actionNode )
    {
        std::vector<std::string> condSplit(3);
        actionNode->setName(xmlNode.attribute("action").value());
        if(!xmlNode.attribute("usefulness").empty())
            actionNode->setUsefulness((float)atof(xmlNode.attribute("usefulness").value()));
        if(!xmlNode.attribute("duration").empty())
            actionNode->setDuration((float)atof(xmlNode.attribute("duration").value()));
        actionNode->setInterruptible(std::string(xmlNode.attribute("interruptible").value()) != "0");

        for(pugi::xml_node condition = xmlNode.first_child(); condition; condition = condition.next_sibling()){
            condSplit.clear();
            condSplit = splitString(condition.child_value());
            Condition* cond = new Condition(m_condMap[condSplit[0]], condSplit[1], condSplit[2]);
            if(std::strcmp(condition.name(),"pre") == 0){
                actionNode->addCondition(cond);
            } else {
                actionNode->addInterruption(cond);
            }
        }

        return true;
    }

    bool Parser::parseCondition( pugi::xml_node& xmlNode, ConditionNode* conditionNode ){
        std::vector<std::string> condSplit(3);

        for(pugi::xml_node condition = xmlNode.first_child(); condition; condition = condition.next_sibling()){
            condSplit.clear();
            condSplit = splitString(condition.child_value());
            Condition* cond = new Condition(m_condMap[condSplit[0]], condSplit[1], condSplit[2]);
            conditionNode->addCondition(cond);
        }

        return true;
    }

    //TODO factory or RTTI
    Behavior* Parser::createNode( pugi::xml_node& xmlNode )
    {
        Behavior* result;
        if(std::strcmp(xmlNode.attribute("type").value(),"Action") == 0){
            if(std::strcmp(xmlNode.attribute("action").value(),"RevealAttacker") == 0){
                result = new BT::BOSS::RevealAttacker(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"Patrol") == 0){
                result = new BT::BOSS::Patrol(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"AngerMode") == 0){
                result = new BT::BOSS::AngerMode(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"AttackFireball") == 0){
                result = new BT::BOSS::AttackFireball(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"AttackMelee") == 0){
                result = new BT::BOSS::AttackMelee(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"ReduceDistance") == 0){
                result = new BT::BOSS::ReduceDistance(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"ExploreSpot") == 0){
                result = new BT::BOSS::ExploreSpot(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"Rotate") == 0){
                result = new BT::BOSS::Rotate(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"KeepDistance") == 0){
                result = new BT::BOSS::KeepDistance(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"AnimAttackMelee") == 0){
                result = new BT::BOSS::AnimAttackMelee(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"AnimAttackPunch") == 0){
                result = new BT::BOSS::AnimAttackPunch(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"AnimAngerMode") == 0){
                result = new BT::BOSS::AnimAngerMode(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"AnimJump") == 0){
                result = new BT::BOSS::AnimJump(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"AnimBackflip") == 0){
                result = new BT::BOSS::AnimBackflip(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"BuyPickaxe") == 0){
                result = new BT::EXPERIMENTS::BuyPickaxe(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"BuyHelmet") == 0){
                result = new BT::EXPERIMENTS::BuyHelmet(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"BuyLantern") == 0){
                result = new BT::EXPERIMENTS::BuyLantern(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"GotoShop") == 0){
                result = new BT::EXPERIMENTS::GotoShop(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"DigGold") == 0){
                result = new BT::EXPERIMENTS::DigGold(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"GotoMine") == 0){
                result = new BT::EXPERIMENTS::GotoMine(m_AI);
            } else if(std::strcmp(xmlNode.attribute("action").value(),"Idle") == 0){
                result = new BT::EXPERIMENTS::Idle(m_AI);
            } else {
                throw "XMLError";
            }
            result->setType(BH_Action);
            return result;
        }
        else if(std::strcmp(xmlNode.attribute("type").value(),"Selector") == 0){
            result = new BT::Selector();
            result->setType(BH_Selector);
        }
        else if(std::strcmp(xmlNode.attribute("type").value(),"PrioritySelector") == 0){
            result = new BT::PrioritySelector();
            result->setType(BH_PSelector);
        }
        else if(std::strcmp(xmlNode.attribute("type").value(),"Sequence") == 0){
            result = new BT::Sequence();
            result->setType(BH_Sequence);
        }
        else if(std::strcmp(xmlNode.attribute("type").value(),"Parallel") == 0){
            if(std::strcmp(xmlNode.attribute("policy").value(),"requireOne") == 0)
                result = new BT::Parallel(Parallel::RequireOne);
            else 
                result = new BT::Parallel(Parallel::RequireAll);
            result->setType(BH_Parallel);
        } else if(std::strcmp(xmlNode.attribute("type").value(),"Condition") == 0){
            result = new BT::ConditionNode();
            result->setType(BH_Condition);
        }
        return result;
    }
}