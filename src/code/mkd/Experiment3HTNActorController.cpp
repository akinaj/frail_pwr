#include "pch.h"
#include "Experiment3HTNActorController.h"
#include "contrib/DebugDrawer.h"

#define SPOT_RADIUS 5.f

Experiment3HTNActorController::Experiment3HTNActorController( ActorAI* ai ) : IActorController(ai)
{
    //actions////////////////////////////////////////////////////////////////////////
    m_actions["opBackflip"] = &Experiment3HTNActorController::actionBackflip;
    m_actions["opJump"] = &Experiment3HTNActorController::actionJump;
    m_actions["animAttackMelee"] = &Experiment3HTNActorController::animAttackMelee;
    m_actions["opAttackMelee"] = &Experiment3HTNActorController::actionAttackMelee;
    m_actions["opIdle"] = &Experiment3HTNActorController::actionIdle;

    m_isTaskExecuted = false;
    m_interrupted = false;

    m_currentIdx = 0;
}

Experiment3HTNActorController::~Experiment3HTNActorController(){
    delete m_planner;
}

//////////////////////////////////////////////////////////////////////////

void Experiment3HTNActorController::onCreate(){
    m_planner = new HTN::Planner();
    m_planner->init(getAI()->getHtnMethodsPath(),getAI()->getHtnOperatorsPath(),getAI()->getHtnGoalsPath());
    getAI()->setDirection(mkVec3::UNIT_Z);

    m_gotHit = false;
    m_jumpTime = 0.f;
    m_planner->setStateBool("Running",false);
    m_planner->setStateFloat("rngMelee",getAI()->getMeleeRange());

    Ogre::LogManager::getSingleton().logMessage("HTN controller created!");
}

void Experiment3HTNActorController::onTakeDamage(const SDamageInfo& dmg_info){
    m_gotHit = true;
}

void Experiment3HTNActorController::onUpdate(float dt){
    updateWorldState(dt);
    
    std::vector<HTN::pTask> plan = m_planner->getPlan(getAI()->getHtnMethodsPath(),getAI()->getHtnOperatorsPath(),getAI()->getHtnGoalsPath());
    executePlan(plan, dt);
}

void Experiment3HTNActorController::onDebugDraw(){
    getAI()->drawSensesInfo();
}

void Experiment3HTNActorController::onDie()
{
}

//////////////////////////////////////////////////////////////////////////

void Experiment3HTNActorController::updateWorldState(float dt){
    Character* enemy = getAI()->findClosestEnemyInSight();

    m_planner->setStateBool("GotHit",m_gotHit);
    m_planner->setStateFloat("JumpTime",g_game->getTimeMs() - m_jumpTime);
    if(enemy){
        m_planner->setStateFloat("EnemyDistance",(enemy->getSimPos()-getAI()->getSimPos()).length());
    } else {
        m_planner->setStateFloat("EnemyDistance",FLT_MAX);
    }
}

void Experiment3HTNActorController::executePlan(std::vector<HTN::pTask>& plan, float dt){
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

void Experiment3HTNActorController::executeTask(HTN::pOperator nextTask){
    //Ogre::LogManager::getSingleton().logMessage(nextTask->getName());
    ctrlrAction action = m_actions[nextTask->getName()];
    (this->*action)(nextTask);
    m_currentTask = nextTask;
}

bool Experiment3HTNActorController::outcomeValidation(HTN::pOperator op){
    std::vector<std::pair<std::string, std::string>> outcomeVect = op->getOutcome();

    bool result;
    for(size_t i=0; i<outcomeVect.size(); ++i){
        notEqual(worldStateValue(outcomeVect[i].first),worldStateValue(outcomeVect[i].second), result);
        if(!result)
            return false;
    }

    return true;
}

aiVariant Experiment3HTNActorController::worldStateValue(std::string name) const{
    return m_planner->getStateVariant(name);
}

bool Experiment3HTNActorController::isOperatorInterrupted(HTN::pOperator current, HTN::pOperator next){
    std::vector<std::string> interruptions = current->getInterruptions();

    for(size_t i=0; i<interruptions.size(); ++i){
        if(std::strcmp(next->getName().c_str(), interruptions[i].c_str()) == 0)
            return true;
    }

    return false;
}

//----------actions----------

bool Experiment3HTNActorController::actionIdle( HTN::pOperator op )
{
    mkVec3 newDir = getRandomHorizontalDir();
    newDir.normalise();
    getAI()->startSmoothChangeDir(newDir, 40, op->getDuration());
    getAI()->setSpeed(0.f);
    return true;
}

bool Experiment3HTNActorController::animAttackMelee( HTN::pOperator op )
{
    getAI()->runAnimation("Attack3",op->getDuration());
    getAI()->setSpeed(0.f);
    return true;
}

bool Experiment3HTNActorController::actionAttackMelee( HTN::pOperator op )
{
    getAI()->hitMelee();
    getAI()->setSpeed(0.f);
    return true;
}

bool Experiment3HTNActorController::actionBackflip( HTN::pOperator op )
{
    m_gotHit = false;
    getAI()->runAnimation("Backflip",op->getDuration());
    getAI()->setSpeed(0.f);
    return true;
}

bool Experiment3HTNActorController::actionJump( HTN::pOperator op )
{
    m_jumpTime = g_game->getTimeMs();
    getAI()->runAnimation("Jump",op->getDuration());
    getAI()->setSpeed(0.f);
    return true;
}