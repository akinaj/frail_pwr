#pragma once
#include "IActorController.h"
#include "Game.h"
#include "Player.h"
#include "utils.h"
#include "HTN\Planner.h"

class Experiment3HTNActorController : public IActorController
{
public:
    typedef bool (Experiment3HTNActorController::*ctrlrAction)(HTN::pOperator op);

    explicit Experiment3HTNActorController(ActorAI* ai);
    ~Experiment3HTNActorController();

    virtual void onCreate();
    virtual void onTakeDamage(const SDamageInfo& dmg_info);
    virtual void onUpdate(float dt);
    virtual void onDebugDraw();
    virtual void onDie();
private:
    //////////////////////////////////////////////////////////////////////////
    std::map<std::string, ctrlrAction> m_actions;
    HTN::Planner *m_planner;
    HTN::pOperator m_currentTask;
    unsigned int m_currentIdx;
    float m_taskDuration;
    bool m_isTaskExecuted;
    bool m_interrupted;
    //////////////////////////////////////////////////////////////////////////
    bool m_gotHit;
    float m_jumpTime;
    //////////////////////////////////////////////////////////////////////////
    void executePlan(std::vector<HTN::pTask>& plan, float dt);
    void executeTask(HTN::pOperator op);
    void updateWorldState(float dt);
    bool outcomeValidation(HTN::pOperator op);
    aiVariant worldStateValue(std::string name) const;
    bool isOperatorInterrupted(HTN::pOperator current, HTN::pOperator next);

    //----------actions----------
    bool actionIdle(HTN::pOperator op);
    bool animAttackMelee(HTN::pOperator op);
    bool actionAttackMelee(HTN::pOperator op);
    bool actionBackflip(HTN::pOperator op);
    bool actionJump(HTN::pOperator op);
};