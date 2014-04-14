#include "pch.h"
#include "SampleHTNActorController.h"
#include "contrib/DebugDrawer.h"

SampleHTNActorController::SampleHTNActorController( ActorAI* ai ) : IActorController(ai)
{
    m_planner = new HTN::Planner();
    m_planner->init(getAI()->getHtnMethodsPath(),getAI()->getHtnOperatorsPath(),getAI()->getHtnGoalsPath());

    //actions////////////////////////////////////////////////////////////////////////
    m_actions["opAttackMelee"] = &SampleHTNActorController::actionAttackMelee;
}

SampleHTNActorController::~SampleHTNActorController(){
    delete m_planner;
}

//////////////////////////////////////////////////////////////////////////

void SampleHTNActorController::onCreate(){
    getAI()->setDirection(mkVec3::UNIT_Z);

    m_planner->setStateBool("IsEnemyVisible",false);

    Ogre::LogManager::getSingleton().logMessage("HTN controller created!");
}

void SampleHTNActorController::onTakeDamage(const SDamageInfo& dmg_info){

}

void SampleHTNActorController::onUpdate(float dt){
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

void SampleHTNActorController::onDebugDraw(){
    getAI()->drawSensesInfo();
}

void SampleHTNActorController::onDie()
{
}

//////////////////////////////////////////////////////////////////////////

void SampleHTNActorController::updateWorldState(float dt){
    m_target = getAI()->findClosestEnemyInSight();
    m_planner->setStateBool("IsEnemyVisible",m_target ? true : false);
}

void SampleHTNActorController::executeTask(HTN::pOperator nextTask){
    ctrlrAction action = m_actions[nextTask->getName()];
    (this->*action)(nextTask->getDuration());
}

//----------actions----------
bool SampleHTNActorController::actionAttackMelee(float duration){
    if(!m_target)
        return false;

    getAI()->setSpeed(0.f);
    getAI()->runAnimation("Attack3",duration);
    getAI()->hitMelee();

    return true;
}