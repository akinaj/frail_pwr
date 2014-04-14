#pragma once
#include "IActorController.h"
#include "Game.h"
#include "Player.h"
#include "utils.h"
#include "HTN\Planner.h"

class Experiment1HTNActorController : public IActorController
{
public:
    typedef bool (Experiment1HTNActorController::*ctrlrAction)(HTN::pOperator op);

    explicit Experiment1HTNActorController(ActorAI* ai);
    ~Experiment1HTNActorController();

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
    float m_npcGold;
    bool m_pickaxe;
    bool m_helmet;
    bool m_lantern;
    mkVec3 m_shopPosition;
    mkVec3 m_minePosition;
    //////////////////////////////////////////////////////////////////////////
    void executeTask(HTN::pOperator op);
    void updateWorldState(float dt);

    //----------actions----------
    bool actionGoto(HTN::pOperator op);
    bool actionDigGold(HTN::pOperator op);
    bool actionBuy(HTN::pOperator op);
};