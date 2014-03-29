#pragma once
#include "IActorController.h"
#include "Player.h"
#include "utils.h"
#include "HTN\Planner.h"

class SampleHTNActorController : public IActorController
{
public:
    typedef bool (SampleHTNActorController::*ctrlrAction)(float);

    explicit SampleHTNActorController(ActorAI* ai);
    ~SampleHTNActorController();

    virtual void onCreate();
    virtual void onTakeDamage(const SDamageInfo& dmg_info);
    virtual void onUpdate(float dt);
    virtual void onDebugDraw();
    virtual void onDie();
private:
    std::map<std::string, ctrlrAction> m_actions;
    HTN::Planner *m_planner;
    Character *m_target;

    void updateWorldState(float dt);
    void executeTask(HTN::pOperator op);

    //----------actions----------
    bool actionAttackMelee(float duration);
};