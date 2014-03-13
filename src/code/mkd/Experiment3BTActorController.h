#pragma once
#include "IActorController.h"
#include "BT/Actions.h"
#include "BT/Parser.h"
#include "BT/BlackBoard.h"
#include "contrib/DebugDrawer.h"

class Experiment3BTActorController : public IActorController {
public:
    explicit Experiment3BTActorController(ActorAI* ai);
    ~Experiment3BTActorController();

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
};