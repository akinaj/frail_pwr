#include "pch.h"
#include "HTNActorController.h"
#include "contrib/DebugDrawer.h"

HTNActorController::HTNActorController( ActorAI* ai ) : IActorController(ai)
{
    //actions////////////////////////////////////////////////////////////////////////
    m_actions["opPatrol"] = &HTNActorController::actionPatrol;
    m_actions["opAttackMelee"] = &HTNActorController::actionAttackMelee;
    m_actions["opRotateToEnemy"] = &HTNActorController::actionRotateToEnemy;
    m_actions["opAttackFireball"] = &HTNActorController::actionAttackFireball;
    m_actions["opReduceDistance"] = &HTNActorController::actionReduceDistance;
    m_actions["opRevealAttacker"] = &HTNActorController::actionRevealAttacker;
    m_actions["opAngerMode"] = &HTNActorController::actionAngerMode;
    m_actions["opExploreSpot"] = &HTNActorController::actionExploreSpot;
    m_actions["opKeepDistance"] = &HTNActorController::actionKeepDistance;
    //animations////////////////////////////////////////////////////////////////////////
    m_actions["animAttackMelee"] = &HTNActorController::animAttackMelee;
    m_actions["animAttackPunch"] = &HTNActorController::animAttackPunch;
    m_actions["animAngerMode"] = &HTNActorController::animAngerMode;

    m_isTaskExecuted = false;
    m_interrupted = false;
    m_isAttacked = false;
    m_enemyRunningAway = false;
    m_prevEnemyDist = 1000.f;
    m_prevEnemyDistTime = 0.f;
    m_prevDistSum = 0.f;
    m_angerMode = false;
    m_enemyLastPos = mkVec3::ZERO;

    m_currentIdx = 0;
}

HTNActorController::~HTNActorController(){
    delete m_planner;
}

//////////////////////////////////////////////////////////////////////////

void HTNActorController::onCreate(){
    m_planner = new HTN::Planner();
    m_planner->init(getAI()->getHtnMethodsPath(),getAI()->getHtnOperatorsPath(),getAI()->getHtnGoalsPath());

    getAI()->setDirection(mkVec3::UNIT_Z);
    //ai_specific////////////////////////////////////////////////////////////////////////
    m_planner->setStateFloat("rngMelee",getAI()->getMeleeRange());
    m_planner->setStateFloat("rngFbMax",getAI()->getShootingRange());
    m_planner->setStateFloat("dmgConeSize",getAI()->getMeleeConeSize());
    m_planner->setStateFloat("HealthAMLimit",(getAI()->getMaxHealth())/2);
    //conditions////////////////////////////////////////////////////////////////////////
    m_planner->setStateBool("IsEnemyVisible",false);
    m_planner->setStateBool("IsEnemyDead",false);
    m_planner->setStateBool("IsEnemyAttack",false);
    m_planner->setStateBool("IsEnemyRunningAway",false);
    m_planner->setStateBool("IsActorAM",false);
    m_planner->setStateBool("IsEnemySeen",false);
    m_planner->setStateBool("IsSpotReached",false);
    m_planner->setStateFloat("EnemyDistance",FLT_MAX);
    m_planner->setStateFloat("ActorHealth",0.f);
    m_planner->setStateFloat("HealthAMLimit",0.f);
    m_planner->setStateFloat("EnemyDgrDiff",0.f);

    Ogre::LogManager::getSingleton().logMessage("HTN controller created!");
}

void HTNActorController::onTakeDamage(const SDamageInfo& dmg_info){
    m_isAttacked = true;
    m_attackDir = mkVec3::ZERO-dmg_info.dir;
}

void HTNActorController::onUpdate(float dt){
    updateWorldState(dt);
    executePlan(m_planner->getPlan(getAI()->getHtnMethodsPath(),getAI()->getHtnOperatorsPath(),getAI()->getHtnGoalsPath()), dt);
}

void HTNActorController::onDebugDraw(){
    getAI()->drawSensesInfo();
    if(m_enemyLastPos != mkVec3::ZERO)
        DebugDrawer::getSingleton().drawCircle(m_enemyLastPos, 0.2f, 30, Ogre::ColourValue::Black, true);
}

void HTNActorController::onDie()
{
    m_angerMode = false;
}

//////////////////////////////////////////////////////////////////////////

void HTNActorController::updateWorldState(float dt){
    if(m_isAttacked)
        m_planner->getWorldState()["IsEnemyAttack"] = true;
    else
        m_planner->getWorldState()["IsEnemyAttack"] = false;

    m_planner->getWorldState()["ActorHealth"] = getAI()->getHealth();
    m_planner->getWorldState()["IsActorAM"] = m_angerMode;

    m_target = getAI()->findClosestEnemyInSight();
    m_planner->getWorldState()["IsEnemyVisible"] = m_target ? true : false;
    if(m_target){
        float angleDiff = (float)(getAI()->getCharToEnemyAngle(m_target->getSimPos()).valueDegrees());
        m_planner->getWorldState()["EnemyDgrDiff"] = angleDiff;
        ActorAI *targetAI = dynamic_cast<ActorAI*>(m_target);
        Player *targetPlayer = dynamic_cast<Player*>(m_target);
        if(targetAI)
            m_planner->getWorldState()["IsEnemyDead"] = targetAI->getHealth() > 0.f ? false : true;
        else if(targetPlayer)
            m_planner->getWorldState()["IsEnemyDead"] = targetPlayer->getHealth() > 0.f ? false : true;

        float enemyDistance = (float)(m_target->getSimPos() - getAI()->getSimPos()).length();
        m_planner->getWorldState()["EnemyDistance"] = enemyDistance;

        m_prevDistSum += enemyDistance - m_prevEnemyDist;
        m_prevEnemyDist = enemyDistance;

        m_prevEnemyDistTime = m_prevEnemyDistTime - dt;
        if(m_prevEnemyDistTime <= 0.f){
            if(m_prevDistSum > 1.f)
                m_enemyRunningAway = true;
            else
                m_enemyRunningAway = false;
            m_prevDistSum = 0.f;
            m_prevEnemyDistTime = 0.5f;
        }

        m_planner->getWorldState()["IsEnemyRunningAway"] = m_enemyRunningAway;

        m_enemyLastPos = m_target->getSimPos();
        m_planner->getWorldState()["IsEnemySeen"] = true;
    } else {
        m_planner->getWorldState()["IsEnemyVisible"] = false;
        m_planner->getWorldState()["IsEnemyDead"] = false;
        m_planner->getWorldState()["EnemyDistance"] = 1000.f;
        m_planner->getWorldState()["EnemyDistanceDiff"] = 0.f;
        m_planner->getWorldState()["IsEnemyRunningAway"] = false;
        m_planner->getWorldState()["EnemyDgrDiff"] = 0.f;
    }

    if((float)(m_enemyLastPos - getAI()->getSimPos()).length() < 3.f){
        m_planner->getWorldState()["IsSpotReached"] = true;
        m_planner->getWorldState()["IsEnemySeen"] = false;
    } else {
        m_planner->getWorldState()["IsSpotReached"] = false;
    }
}

void HTNActorController::executePlan(std::vector<HTN::pTask>& plan, float dt){
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

void HTNActorController::executeTask(HTN::pOperator nextTask){
    //Ogre::LogManager::getSingleton().logMessage(nextTask->getName());
    ctrlrAction action = m_actions[nextTask->getName()];
    (this->*action)(nextTask->getDuration());
    m_currentTask = nextTask;
}

bool HTNActorController::outcomeValidation(HTN::pOperator op){
    std::vector<std::pair<std::string, std::string>> outcomeVect = op->getOutcome();

    bool result;
    for(size_t i=0; i<outcomeVect.size(); ++i){
        notEqual(worldStateValue(outcomeVect[i].first),worldStateValue(outcomeVect[i].second), result);
        if(!result)
            return false;
    }

    return true;
}

aiVariant HTNActorController::worldStateValue(std::string name) const{
    return m_planner->getStateVariant(name);
}

bool HTNActorController::isOperatorInterrupted(HTN::pOperator current, HTN::pOperator next){
    std::vector<std::string> interruptions = current->getInterruptions();

    for(size_t i=0; i<interruptions.size(); ++i){
        if(std::strcmp(next->getName().c_str(), interruptions[i].c_str()) == 0)
            return true;
    }

    return false;
}

//----------actions----------
bool HTNActorController::actionPatrol(float duration){
    mkVec3 new_direction = getRandomHorizontalDir();
    RayCastResult ray_result = getAI()->raycast(new_direction, 1.0f, 5.f);
    while(ray_result.hit && ray_result.collision_type == RayCastResult::Environment){
        new_direction = getRandomHorizontalDir();
        ray_result = getAI()->raycast(new_direction, 1.0f, 5.f);
    }

    size_t steps = 40;
    getAI()->startSmoothChangeDir(new_direction, steps, duration/2);
    //getAI()->setDirection(new_direction);
    getAI()->setSpeed(0.5f);

    return true;
}

bool HTNActorController::actionRotateToEnemy(float duration){
    if(!m_target)
        return false;

    size_t steps = 40;
    mkVec3 new_direction = m_target->getSimPos() - getAI()->getSimPos();
    new_direction.normalise();
    getAI()->startSmoothChangeDir(new_direction, steps, duration);
    getAI()->setSpeed(0.f);

    return true;
}

bool HTNActorController::actionAttackMelee(float duration){
    if(!m_target)
        return false;

    getAI()->hitMelee();
    getAI()->setSpeed(0.f);

    return true;
}

bool HTNActorController::actionAttackFireball(float duration){
    if(!m_target)
        return false;
    getAI()->hitFireball(m_target->getSimPos());
    getAI()->setSpeed(0.f);
    return true;
}

bool HTNActorController::actionReduceDistance(float duration){
    if(!m_target)
        return false;
    RayCastResult ray_result = getAI()->raycast(m_target->getSimPos(), 0.1f, 1.f);
    if (ray_result.hit && ray_result.collision_type == RayCastResult::Environment)
        getAI()->jump();

    mkVec3 destDir = (m_target->getSimPos()-getAI()->getSimPos()).normalisedCopy();
    size_t steps = 40;
    getAI()->startSmoothChangeDir(destDir, steps, duration);
    getAI()->setSpeed(1.f);

    return true;
}

bool HTNActorController::actionRevealAttacker(float duration){
    getAI()->setSpeed(0.3f);
    size_t steps = 40;
    m_isAttacked = false;
    getAI()->startSmoothChangeDir(m_attackDir, steps, (duration+duration+duration)/4);

    return true;
}

bool HTNActorController::actionAngerMode(float duration){
    if(!m_target)
        return false;

    getAI()->hitAngerMode();
    getAI()->setSpeed(0.f);
    m_angerMode = true;
    return true;
}

bool HTNActorController::actionExploreSpot(float duration)
{
    if(m_enemyLastPos == mkVec3::ZERO)
        return false;

    size_t steps = 40;
    mkVec3 new_direction = m_enemyLastPos - getAI()->getSimPos();
    new_direction.normalise();
    getAI()->startSmoothChangeDir(new_direction, steps, duration);
    getAI()->setSpeed(1.f);
    return true;
}

bool HTNActorController::actionKeepDistance(float duration)
{
    if(!m_target)
        return false;

    size_t steps = 40;
    mkVec3 new_direction = getAI()->getSimPos() - m_target->getSimPos();
    new_direction.normalise();

    RayCastResult ray_result = getAI()->raycast(new_direction, 1.0f, 10.f);
    if(ray_result.hit && ray_result.collision_type == RayCastResult::Environment){
        return false;
    } else {
        getAI()->startSmoothChangeDir(new_direction, steps, duration/3);
        getAI()->setSpeed(1.0f);
        return true;
    }
    return false;
}

bool HTNActorController::animAttackMelee( float duration )
{
    getAI()->runAnimation("Attack3",duration);
    getAI()->setSpeed(0.f);
    return true;
}

bool HTNActorController::animAttackPunch( float duration )
{
    getAI()->runAnimation("Attack1",duration);
    getAI()->setSpeed(0.f);
    return true;
}

bool HTNActorController::animAngerMode( float duration )
{
    getAI()->runAnimation("HighJump",duration);
    getAI()->setSpeed(0.f);
    return true;
}
