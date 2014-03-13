/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "StateMachineActorController.h"

class CtfMgr;

// Actor controller using state machine for CTF game
class CtfSMActorController : public StateMachineActorController
{
public:
    explicit CtfSMActorController(ActorAI* ai);

    virtual void onCreate();
    virtual void onDebugDraw();

    const mkVec3& getDefensePos() const { return m_defensePos; }
    const mkVec3& getBasePos() const { return m_myBasePos; }
    mkVec3 getOurFlagPos() const;

    const Character* findEnemyInSight() const;
    const Character* findVisibleEnemyInShootingRange(float range_part) const;

    bool shouldDefend();
    bool shouldGrabEnemyFlag();
    bool shouldInterceptOwnFlag();
    bool isFlagCarriedByUs();

    EConflictSide::TYPE getEnemyTeam() const;

    CtfMgr* getCtfMgr() const;

private:
    mkVec3 m_myBasePos;
    mkVec3 m_defensePos;

    CtfMgr* m_ctfMgr;
};

// Those states are abomination, really!
namespace ctf_sm
{
    class BaseCtfState : public sm::State
    {
    public:
        explicit BaseCtfState(CtfSMActorController* controller);

        // not virtual override!
        CtfSMActorController* getController() const;

        CtfMgr* getCtfMgr() const;
    };

    class PatrolCtfState : public BaseCtfState
    {
    public:
        PatrolCtfState(CtfSMActorController* controller, const mkVec3& dir);

        void onUpdate(float dt);
        void onEnter(State* prev_state);

    private:
        mkVec3 m_direction;
        float m_stateStartTime;
    };

    class GoToState : public BaseCtfState // this should probably be handled by actor
    {
    public:
        GoToState(CtfSMActorController* controller, const mkVec3& dest_pos, float radius = 0.5f);

        void onUpdate(float dt);
        void onEnter(State* prev_state);

        virtual void onArrived() { }

        void updateDestPos(const mkVec3& dest_pos) { m_destPos = dest_pos; }

    private:
        mkVec3 m_destPos;
        float m_destRadius;

    protected:
        void rotateToDest(); // this goto is dumb and does no pathfinding or obstacle avoidance
    };

    class Defense_GoToDefensePosCtfState : public GoToState
    {
    public:
        explicit Defense_GoToDefensePosCtfState(CtfSMActorController* controller);
        virtual void onArrived();
        virtual void onUpdate(float dt);
    };

    class Defense_LookAround : public BaseCtfState
    {
    public:
        explicit Defense_LookAround(CtfSMActorController* controller);

        void onUpdate(float dt);
        void onEnter(State* prev_state);

    private:
        float m_lastDirectionChangeTime;
        mkVec3 m_lastDirection;
        mkVec3 m_wantedDirection;

        void changeWantedDir();
    };

    class Defense_Shoot : public BaseCtfState
    {
    public:
        Defense_Shoot(CtfSMActorController* controller, const Character* enemy);

        void onUpdate(float dt);

    private:
        const Character* m_enemy;
    };

    class Attack_GoToEnemyFlag : public GoToState
    {
    public:
        Attack_GoToEnemyFlag(CtfSMActorController* controller, EConflictSide::TYPE enemy_team, const mkVec3& flag_pos);
        virtual void onArrived();

        void onUpdate(float dt);

    private:
        EConflictSide::TYPE m_enemyTeam;
    };

    class Attack_ReturnWithEnemyFlag : public GoToState
    {
    public:
        explicit Attack_ReturnWithEnemyFlag(CtfSMActorController* controller);
        virtual void onArrived();
    };

    class Defense_InterceptOwnFlag : public GoToState
    {
    public:
        explicit Defense_InterceptOwnFlag(CtfSMActorController* controller);
        
        virtual void onArrived();
        void onUpdate(float dt);
    };

    class Defense_ReturnFlagToBase : public GoToState
    {
    public:
        explicit Defense_ReturnFlagToBase(CtfSMActorController* controller);

        virtual void onArrived();
        void onUpdate(float dt);
    };
}
