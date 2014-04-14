#include "pch.h"
#include "Experiment3HTNActorController.h"
#include "contrib/DebugDrawer.h"

Experiment3HTNActorController::Experiment3HTNActorController( ActorAI* ai ) : IActorController(ai)
{
    //actions////////////////////////////////////////////////////////////////////////
    m_actions["opBackflip"] = &Experiment3HTNActorController::actionBackflip;
    m_actions["opJump"] = &Experiment3HTNActorController::actionJump;
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
    HTN::pOperator newTask;
    HTN::PlanResult result = m_planner->resolvePlan(plan, dt, newTask);

    switch (result)
    {
    case HTN::PLAN_EMPTYPLAN:
        getAI()->setSpeed(0.f);
        break;
    case HTN::PLAN_INTERRUPTED:
        getAI()->stopSmoothChangeDir();
        getAI()->stopAnimation();
    case HTN::PLAN_NEW:
        executeTask(newTask);
        break;
    case HTN::PLAN_RUNNING:
        break;
    }
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

void Experiment3HTNActorController::executeTask(HTN::pOperator nextTask){
    //Ogre::LogManager::getSingleton().logMessage(nextTask->getName());
    ctrlrAction action = m_actions[nextTask->getName()];
    (this->*action)(nextTask);
    m_currentTask = nextTask;
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

bool Experiment3HTNActorController::actionAttackMelee( HTN::pOperator op )
{
    getAI()->runAnimation("Attack3",op->getDuration());
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