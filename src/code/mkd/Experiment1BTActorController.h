#pragma once
#include "IActorController.h"
#include "BT/Actions.h"
#include "BT/Parser.h"
#include "BT/BlackBoard.h"
#include "contrib/DebugDrawer.h"

class Experiment1BTActorController : public IActorController {
public:
    explicit Experiment1BTActorController(ActorAI* ai);
    ~Experiment1BTActorController();

    virtual void onCreate();
    virtual void onTakeDamage( const SDamageInfo& dmg_info );
    virtual void onUpdate( float dt );
    virtual void onDebugDraw();
    virtual void onDie();

    void updateWorldState(float dt);
    Character* findClosestEnemyInSight() const;
private:
    BT::Behavior* m_root;
    BT::BlackBoard* m_bb;
    BT::Parser* m_parser;
    //////////////////////////////////////////////////////////////////////////
    float m_npcGold;
    mkVec3 m_shopPosition;
    mkVec3 m_minePosition;
};