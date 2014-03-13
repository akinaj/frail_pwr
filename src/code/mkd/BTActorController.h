#pragma once
#include "IActorController.h"
#include "BT/Behavior.h"
#include "BT/Parser.h"
#include "BT/BlackBoard.h"
#include "contrib/DebugDrawer.h"

class BTActorController : public IActorController {
public:
    explicit BTActorController(ActorAI* ai);
    ~BTActorController();

    virtual void onCreate();
    virtual void onTakeDamage( const SDamageInfo& dmg_info );
    virtual void onUpdate( float dt );
    virtual void onDebugDraw();
    virtual void onDie();

    void updateWorldState(float dt);
private:
    BT::Behavior* m_root;
    BT::BlackBoard* m_bb;
    BT::Parser* m_parser;

    Character* m_target;
    mkVec3 m_attackDir;
    mkVec3 m_enemySpot;
    float m_prevEnemyDistTime;
    float m_prevEnemyDist;
    float m_prevDistSum;
    bool m_resetAngerMode;
};