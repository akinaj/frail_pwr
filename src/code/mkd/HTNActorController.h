#pragma once
#include "IActorController.h"
#include "Player.h"
#include "utils.h"
#include "HTN\Planner.h"

class HTNActorController : public IActorController
{
public:
    typedef bool (HTNActorController::*ctrlrAction)(float);

	explicit HTNActorController(ActorAI* ai);
	~HTNActorController();

	virtual void onCreate();
	virtual void onTakeDamage(const SDamageInfo& dmg_info);
	virtual void onUpdate(float dt);
	virtual void onDebugDraw();
    virtual void onDie();
private:
    std::map<std::string, ctrlrAction> m_actions;
    HTN::Planner *m_planner;
    Character *m_target;
    mkVec3 m_enemyLastPos;
    bool m_isAttacked;
    mkVec3 m_attackDir;
    bool m_enemyRunningAway;
    float m_prevEnemyDist;
    float m_prevDistSum;
    float m_prevEnemyDistTime;
    bool m_angerMode;

    void updateWorldState(float dt);
    void executeTask(HTN::pOperator op);

	//----------actions----------
	bool actionPatrol(float duration);
    bool actionRotateToEnemy(float duration);
	bool actionAttackMelee(float duration);
	bool actionAttackFireball(float duration);
	bool actionReduceDistance(float duration);
	bool actionRevealAttacker(float duration);
    bool actionAngerMode(float duration);
    bool actionExploreSpot(float duration);
    bool actionKeepDistance(float duration);
    //////////////////////////////////////////////////////////////////////////
    bool animAttackMelee(float duration);
    bool animAttackPunch(float duration);
    bool animAngerMode(float duration);
};