/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "IActorController.h"
#include "utils.h"

namespace sm
{
    class State;
}

class StateMachineActorController : public IActorController
{
public:
    explicit StateMachineActorController(ActorAI* ai);
    ~StateMachineActorController();

    virtual void onCreate();
    virtual void onTakeDamage(const SDamageInfo& dmg_info);
    virtual void onUpdate(float dt);
    virtual void onDebugDraw();

    void scheduleTransitionInNextFrame(sm::State* next);

protected:
    void updateStateTransition();

private:
    sm::State* m_currentState;
    sm::State* m_nextState;
};

namespace sm
{
    class State
    {
    public:
        explicit State(StateMachineActorController* controller) : m_controller(controller) { }
        virtual ~State() { }

        virtual void onUpdate(float dt) { }
        virtual void onTakeDamage() { }
        virtual void onDebugDraw() { }

        virtual void onEnter(State* prev_state) { }
        virtual void onLeave(State* next_state) { }

        StateMachineActorController* getController() const { return m_controller; }
        ActorAI* getAI() const { return getController()->getAI(); }

    protected:
        StateMachineActorController* m_controller;
    };

    class PatrolState : public State
    {
    public:
        PatrolState(StateMachineActorController* controller, const mkVec3& direction);

        void onUpdate(float dt);
        void onEnter(State* prev_state);

    private:
        mkVec3 m_direction;
        float m_stateStartTime;

        const Character* findEnemyInSight() const;
    };

    class ChaseState : public State
    {
    public:
        ChaseState(StateMachineActorController* controller, const Character* enemy);

        void onUpdate(float dt);
        void onEnter(State* prev_state);

    private:
        const Character* m_enemy;
    };

    class ShootState : public State
    {
    public:
        ShootState(StateMachineActorController* controller, const Character* enemy);

        void onUpdate(float dt);
        void onEnter(State* prev_state);
        virtual void onDebugDraw();

    private:
        const Character* m_enemy;
        float m_lastShotTime;

        float getShotPrepareProgress() const;
        void shoot();

        bool isEnemyDeadOrLost() const;
        bool isEnemyInShootingRange() const;
        void goToPatrol();
        void resumeChase();
    };

}
