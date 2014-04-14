#pragma once
#include "StateMachineActorController.h"

class SampleFSMActorController : public StateMachineActorController
{
public:
    explicit SampleFSMActorController(ActorAI* ai);

    virtual void onCreate();
    virtual void onDebugDraw();
};

namespace sample_sm
{
    class BaseState : public sm::State
    {
    public:
        explicit BaseState(SampleFSMActorController* controller);

        SampleFSMActorController* getController() const;
    };

    class IdleState : public BaseState
    {
    public:
        IdleState(SampleFSMActorController* controller);

        void onUpdate(float dt);
        void onEnter(State* prev_state);

        virtual void onTakeDamage();

        virtual void onDebugDraw();

        virtual void onLeave( State* next_state );

    private:
        float m_stateStartTime;
    };

    class AttackState : public BaseState
    {
    public:
        AttackState(SampleFSMActorController* controller, Character* target);

        void onUpdate(float dt);
        void onEnter(State* prev_state);

        virtual void onLeave( State* next_state );

    private:
        Character* m_target;
        float m_stateStartTime;
    };
}
