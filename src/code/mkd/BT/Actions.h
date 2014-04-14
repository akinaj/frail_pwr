#pragma once
#include "BT/Behavior.h"
#include "ActorAI.h"
#include "Character.h"
#include "Game.h"

namespace BT {
    class Action : public Behavior {
    public:
        explicit Action(ActorAI* ai);
        virtual ~Action();

        virtual Status update();
        virtual void onInitialize(BlackBoard* bb);
        virtual void onTerminate(Status status);

        bool validateConditions(BlackBoard* bb);
        bool validateInterruptions(BlackBoard* bb);
        void addCondition(Condition* condition);
        void addInterruption(Condition* condition);
        size_t conditionSize() { return m_conditions.size(); }

        inline Status runUntil();
        inline bool isValid();

        std::string getName() const { return m_name; }
        void setName(std::string val) { m_name = val; }
        float getDuration() const { return m_duration; }
        void setDuration(float val) { m_duration = val; }
        bool isInterruptible() const { return m_interruptible; }
        void setInterruptible(bool val) { m_interruptible = val; }
        float getActionStarted() const { return m_actionStarted; }
        void setActionStarted(float val) { m_actionStarted = val; }
    protected:
        ActorAI* m_AI;
        boost::ptr_vector<Condition> m_conditions;
        boost::ptr_vector<Condition> m_interruptions;
        std::string m_name;
        float m_duration;
        bool m_interruptible;
        float m_actionStarted;
    };

    namespace BOSS {

        class Patrol : public Action {
        public:
            explicit Patrol(ActorAI* ai);
            virtual ~Patrol();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class RevealAttacker : public Action {
        public:
            explicit RevealAttacker(ActorAI* ai);
            virtual ~RevealAttacker();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class ReduceDistance : public Action {
        public:
            explicit ReduceDistance(ActorAI* ai);
            virtual ~ReduceDistance();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AttackMelee : public Action {
        public:
            explicit AttackMelee(ActorAI* ai);
            virtual ~AttackMelee();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AttackFireball : public Action {
        public:
            explicit AttackFireball(ActorAI* ai);
            virtual ~AttackFireball();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AngerMode : public Action {
        public:
            explicit AngerMode(ActorAI* ai);
            virtual ~AngerMode();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class ExploreSpot : public Action {
        public:
            explicit ExploreSpot(ActorAI* ai);
            virtual ~ExploreSpot();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class Rotate : public Action {
        public:
            explicit Rotate(ActorAI* ai);
            virtual ~Rotate();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class KeepDistance : public Action {
        public:
            explicit KeepDistance(ActorAI* ai);
            virtual ~KeepDistance();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AnimAttackMelee : public Action {
        public:
            explicit AnimAttackMelee(ActorAI* ai);
            virtual ~AnimAttackMelee();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AnimAttackPunch : public Action {
        public:
            explicit AnimAttackPunch(ActorAI* ai);
            virtual ~AnimAttackPunch();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AnimAngerMode : public Action {
        public:
            explicit AnimAngerMode(ActorAI* ai);
            virtual ~AnimAngerMode();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AnimJump : public Action {
        public:
            explicit AnimJump(ActorAI* ai);
            virtual ~AnimJump();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class AnimBackflip : public Action {
        public:
            explicit AnimBackflip(ActorAI* ai);
            virtual ~AnimBackflip();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

    }

    namespace EXPERIMENTS {
        class GotoShop : public Action {
        public:
            explicit GotoShop(ActorAI* ai);
            virtual ~GotoShop();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class GotoMine : public Action {
        public:
            explicit GotoMine(ActorAI* ai);
            virtual ~GotoMine();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class DigGold : public Action {
        public:
            explicit DigGold(ActorAI* ai);
            virtual ~DigGold();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class BuyPickaxe : public Action {
        public:
            explicit BuyPickaxe(ActorAI* ai);
            virtual ~BuyPickaxe();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class BuyHelmet : public Action {
        public:
            explicit BuyHelmet(ActorAI* ai);
            virtual ~BuyHelmet();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class BuyLantern : public Action {
        public:
            explicit BuyLantern(ActorAI* ai);
            virtual ~BuyLantern();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };

        class Idle : public Action {
        public:
            explicit Idle(ActorAI* ai);
            virtual ~Idle();

            virtual Status update();
            virtual void onInitialize(BlackBoard* bb);
            virtual void onTerminate(Status status);
        };
    }
}