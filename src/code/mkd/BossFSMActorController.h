#pragma once
#include "StateMachineActorController.h"

class BossFSMActorController : public StateMachineActorController
{
public:
    explicit BossFSMActorController(ActorAI* ai);

    virtual void onCreate();
    virtual void onDebugDraw();
};

namespace boss_sm
{
    class BaseState : public sm::State
    {
    public:
        explicit BaseState(BossFSMActorController* controller);

        BossFSMActorController* getController() const;
    };

    class PatrolState : public BaseState
    {
    public:
        PatrolState(BossFSMActorController* controller, const mkVec3& dir);

        void onUpdate(float dt);
        void onEnter(State* prev_state);

    private:
        mkVec3 m_direction;
        float m_stateStartTime;
    };
}
