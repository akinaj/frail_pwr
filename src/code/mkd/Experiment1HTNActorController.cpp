#include "pch.h"
#include "Experiment1HTNActorController.h"
#include "contrib/DebugDrawer.h"

#define SPOT_RADIUS 5.f

Experiment1HTNActorController::Experiment1HTNActorController( ActorAI* ai ) : IActorController(ai)
{
    //actions////////////////////////////////////////////////////////////////////////
    m_actions["opGoto"] = &Experiment1HTNActorController::actionGoto;
    m_actions["opDigGold"] = &Experiment1HTNActorController::actionDigGold;
    m_actions["opBuy"] = &Experiment1HTNActorController::actionBuy;

    m_isTaskExecuted = false;
    m_interrupted = false;

    m_npcGold = 0;
    m_pickaxe = false;
    m_helmet = false;
    m_lantern = false;

    m_currentIdx = 0;
}

Experiment1HTNActorController::~Experiment1HTNActorController(){
    delete m_planner;
}

//////////////////////////////////////////////////////////////////////////

void Experiment1HTNActorController::onCreate(){
    m_planner = new HTN::Planner();
    m_planner->init(getAI()->getHtnMethodsPath(),getAI()->getHtnOperatorsPath(),getAI()->getHtnGoalsPath());
    getAI()->setDirection(mkVec3::UNIT_Z);

    m_shopPosition = mkVec3(33.f, 0.1f, 2.f);
    m_minePosition = mkVec3(-33.f,0.1f,2.f);

    Ogre::LogManager::getSingleton().logMessage("HTN controller created!");
}

void Experiment1HTNActorController::onTakeDamage(const SDamageInfo& dmg_info){
}

void Experiment1HTNActorController::onUpdate(float dt){
    updateWorldState(dt);
    executePlan(m_planner->getPlan(getAI()->getHtnMethodsPath(),getAI()->getHtnOperatorsPath(),getAI()->getHtnGoalsPath()), dt);
}

void Experiment1HTNActorController::onDebugDraw(){
    //getAI()->drawSensesInfo();
    DebugDrawer::getSingleton().drawCircle(m_shopPosition, SPOT_RADIUS, 30, Ogre::ColourValue(0.f,1.f,0.2f,0.5f), true);
    DebugDrawer::getSingleton().drawCircle(m_minePosition, SPOT_RADIUS, 30, Ogre::ColourValue(1.f,0.9f,0.f,0.5f), true);
}

void Experiment1HTNActorController::onDie()
{
}

//////////////////////////////////////////////////////////////////////////

void Experiment1HTNActorController::updateWorldState(float dt){
    m_planner->setStateFloat("NPCGold",m_npcGold);
    m_planner->setStateBool("Pickaxe",m_pickaxe);
    m_planner->setStateBool("Helmet",m_helmet);
    m_planner->setStateBool("Lantern",m_lantern);
    
    float distance = (getAI()->getSimPos() - m_shopPosition).length();
    if(distance <= SPOT_RADIUS)
        m_planner->setStateBool("Shop",true);
    else
        m_planner->setStateBool("Shop",false);

    distance = (getAI()->getSimPos() - m_minePosition).length();
    if(distance <= SPOT_RADIUS)
        m_planner->setStateBool("Mine",true);
    else
        m_planner->setStateBool("Mine",false);
}

void Experiment1HTNActorController::executePlan(std::vector<HTN::pTask>& plan, float dt){
    if(plan.size() == 0){
        getAI()->setSpeed(0.f);
        return;
    }

    if(m_currentIdx >= plan.size())
        m_currentIdx = 0;

    if(m_currentIdx > 0 && plan[m_currentIdx-1] != m_currentTask)
        m_currentIdx = 0;

    HTN::pOperator nextTask = boost::dynamic_pointer_cast<HTN::Operator>(plan[m_currentIdx]);
    if(m_currentTask && m_currentTask->isInterruptible()){
        if(!outcomeValidation(m_currentTask) || isOperatorInterrupted(m_currentTask, nextTask))
            m_interrupted = true;
    }

    if(m_interrupted)
        getAI()->stopSmoothChangeDir();

    if(!m_isTaskExecuted || m_interrupted){
        if(nextTask->isAnim())
            ++m_currentIdx;
        else
            m_currentIdx = 0;

        if(m_interrupted){
            m_interrupted = false;
            //Ogre::LogManager::getSingleton().logMessage("+++Interrupted+++");
        }

        m_taskDuration = nextTask->getDuration()/1000;
        executeTask(nextTask);
        m_isTaskExecuted = true;
    } else {
        m_taskDuration = m_taskDuration - dt;
        if(m_taskDuration <= 0)
            m_isTaskExecuted = false;
    }
}

void Experiment1HTNActorController::executeTask(HTN::pOperator nextTask){
    //Ogre::LogManager::getSingleton().logMessage(nextTask->getName());
    ctrlrAction action = m_actions[nextTask->getName()];
    (this->*action)(nextTask);
    m_currentTask = nextTask;
}

bool Experiment1HTNActorController::outcomeValidation(HTN::pOperator op){
    std::vector<std::pair<std::string, std::string>> outcomeVect = op->getOutcome();

    bool result;
    for(size_t i=0; i<outcomeVect.size(); ++i){
        notEqual(worldStateValue(outcomeVect[i].first),worldStateValue(outcomeVect[i].second), result);
        if(!result)
            return false;
    }

    return true;
}

aiVariant Experiment1HTNActorController::worldStateValue(std::string name) const{
    return m_planner->getStateVariant(name);
}

bool Experiment1HTNActorController::isOperatorInterrupted(HTN::pOperator current, HTN::pOperator next){
    std::vector<std::string> interruptions = current->getInterruptions();

    for(size_t i=0; i<interruptions.size(); ++i){
        if(std::strcmp(next->getName().c_str(), interruptions[i].c_str()) == 0)
            return true;
    }

    return false;
}

//----------actions----------
bool Experiment1HTNActorController::actionGoto( HTN::pOperator op )
{
    size_t steps = 40;
    mkVec3 new_direction;
    if(op->getParameters()[0] == "Shop"){
        new_direction = m_shopPosition - getAI()->getSimPos();
    } else if (op->getParameters()[0] == "Mine"){
        new_direction = m_minePosition - getAI()->getSimPos();
    }
    new_direction.normalise();
    getAI()->startSmoothChangeDir(new_direction, steps, 500.f);
    getAI()->setSpeed(0.5f);

    RayCastResult ray_result = getAI()->raycast(new_direction, 0.1f, 0.5f);
    if (ray_result.hit && ray_result.collision_type == RayCastResult::Environment)
        getAI()->jump();

    return true;
}

bool Experiment1HTNActorController::actionDigGold( HTN::pOperator op )
{
    float mult = 1.f;
    mult += m_pickaxe == true ? 1.f : 0.f;
    mult += m_helmet == true ? 1.f : 0.f;
    mult += m_lantern == true ? 1.f : 0.f;

    m_npcGold += 10.f * mult;
    if(!m_pickaxe){
        getAI()->runAnimation("Attack1",op->getDuration());
    } else {
        getAI()->runAnimation("Attack3",op->getDuration());
    }

    getAI()->setSpeed(0.f);
    return true;
}

bool Experiment1HTNActorController::actionBuy( HTN::pOperator op )
{
    bool isValid = true;
    m_npcGold -= m_planner->getStateFloat(op->getParameters()[1], isValid);
    if (op->getParameters()[0] == "Pickaxe")
    {
        m_pickaxe = true;
    } else if (op->getParameters()[0] == "Helmet")
    {
        m_helmet = true;
    } else if (op->getParameters()[0] == "Lantern")
    {
        m_lantern = true;
    }
    getAI()->runAnimation("Attack1",op->getDuration());
    getAI()->setSpeed(0.f);
    return true;
}