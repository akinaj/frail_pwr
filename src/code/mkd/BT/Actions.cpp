#include "pch.h"
#include "Actions.h"

namespace BT {
    Action::Action(ActorAI* ai) 
        : m_AI(ai), m_actionStarted(0.f), m_duration(0.f)
    {

    }

    Action::~Action()
    {

    }

    Status Action::update()
    {
        return BH_FAILURE;
    }

    void Action::onInitialize(BlackBoard* bb)
    {
        m_bb = bb;
        m_actionStarted = g_game->getTimeMs();
    }

    void Action::onTerminate( Status status )
    {

    }

    void Action::addCondition(Condition* condition)
    {
        m_conditions.push_back(condition);
    }

    void Action::addInterruption(Condition* condition)
    {
        m_interruptions.push_back(condition);
    }

    bool Action::validateConditions(BlackBoard* bb)
    {
        for(size_t i=0; i<m_conditions.size(); ++i){
            if(!m_conditions[i].validateCondition(bb))
                return false;
        }
        return true;
    }

    bool Action::validateInterruptions(BlackBoard* bb)
    {
        for(size_t i=0; i<m_interruptions.size(); ++i){
            if(m_interruptions[i].validateCondition(bb))
                return true;
        }
        return false;
    }

    inline Status Action::runUntil()
    {
        if(g_game->getTimeMs() - getActionStarted() >= getDuration())
            return BH_SUCCESS;

        if(getStatus() == BH_RUNNING)
            return BH_RUNNING;

        return BH_INVALID;
    }

    inline bool Action::isValid()
    {
        if(isInterruptible()){
            if(!validateConditions(m_bb) || validateInterruptions(m_bb)){
                m_AI->stopSmoothChangeDir();
                m_AI->stopAnimation();
                return false;
            }
        } else {
            if(!(validateConditions(m_bb) || getStatus() == BH_RUNNING))
                return false;
        }

        return true;
    }

    //////////////////////////////////////////////////////////////////////////

    namespace BOSS {
        Patrol::Patrol(ActorAI* ai) 
            : Action(ai)
        {

        }

        Patrol::~Patrol()
        {

        }

        Status Patrol::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            //Ogre::LogManager::getSingleton().logMessage("Patrol!");
            mkVec3 new_direction = getRandomHorizontalDir();
            RayCastResult ray_result = m_AI->raycast(new_direction, 1.0f, 5.f);
            while(ray_result.hit && ray_result.collision_type == RayCastResult::Environment){
                new_direction = getRandomHorizontalDir();
                ray_result = m_AI->raycast(new_direction, 1.0f, 5.f);
            }

            size_t steps = 40;
            m_AI->startSmoothChangeDir(new_direction, steps, getDuration()/2);
            m_AI->setSpeed(0.5f);

            return BH_RUNNING;
        }

        void Patrol::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void Patrol::onTerminate( Status status )
        {
            m_AI->stopSmoothChangeDir();
        }

        //////////////////////////////////////////////////////////////////////////

        RevealAttacker::RevealAttacker(ActorAI* ai) 
            : Action(ai)
        {

        }

        RevealAttacker::~RevealAttacker()
        {

        }

        Status RevealAttacker::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 attackDir = m_bb->getStateVec3("AttackDir",isValid);

            if(!isValid)
                return BH_FAILURE;

            //Ogre::LogManager::getSingleton().logMessage("Reveal!");
            m_AI->setSpeed(0.3f);
            size_t steps = 40;
            m_AI->startSmoothChangeDir(attackDir, steps, getDuration());

            return BH_RUNNING;
        }

        void RevealAttacker::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void RevealAttacker::onTerminate( Status status )
        {   
            m_bb->setStateBool("IsEnemyAttack",false);
            m_bb->setStateVec3("AttackDir",mkVec3::ZERO);
            m_AI->stopSmoothChangeDir();
        }

        //////////////////////////////////////////////////////////////////////////

        ReduceDistance::ReduceDistance(ActorAI* ai) 
            : Action(ai)
        {

        }

        ReduceDistance::~ReduceDistance()
        {

        }

        Status ReduceDistance::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if(!isValid)
                return BH_FAILURE;

            RayCastResult ray_result = m_AI->raycast(enemyPos, 0.1f, 1.f);
            if (ray_result.hit && ray_result.collision_type == RayCastResult::Environment)
                m_AI->jump();

            //Ogre::LogManager::getSingleton().logMessage("Reduce!");
            mkVec3 destDir = (enemyPos-m_AI->getSimPos()).normalisedCopy();
            size_t steps = 40;
            m_AI->startSmoothChangeDir(destDir, steps, getDuration());
            m_AI->setSpeed(1.f);

            return BH_RUNNING;
        }

        void ReduceDistance::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void ReduceDistance::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }

        //////////////////////////////////////////////////////////////////////////

        AttackMelee::AttackMelee(ActorAI* ai) 
            : Action(ai)
        {

        }

        AttackMelee::~AttackMelee()
        {

        }

        Status AttackMelee::update()
        {
            if(isInterruptible()){
                if(!validateConditions(m_bb))
                    return BH_FAILURE;
            } else {
                if(!(validateConditions(m_bb) || getStatus() == BH_RUNNING))
                    return BH_FAILURE;
            }

            if(g_game->getTimeMs() - getActionStarted() >= getDuration())
                return BH_SUCCESS;

            if(getStatus() == BH_RUNNING)
                return BH_RUNNING;

            //Ogre::LogManager::getSingleton().logMessage("AttackMelee!");
            m_AI->hitMelee();
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AttackMelee::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AttackMelee::onTerminate( Status status )
        {   

        }

        //////////////////////////////////////////////////////////////////////////

        AttackFireball::AttackFireball(ActorAI* ai) 
            : Action(ai)
        {

        }

        AttackFireball::~AttackFireball()
        {

        }

        Status AttackFireball::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if(!isValid)
                return BH_FAILURE;

            //Ogre::LogManager::getSingleton().logMessage("AttackFireball!");
            m_AI->hitFireball(enemyPos);
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AttackFireball::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AttackFireball::onTerminate( Status status )
        {   

        }

        //////////////////////////////////////////////////////////////////////////

        AngerMode::AngerMode(ActorAI* ai) 
            : Action(ai)
        {

        }

        AngerMode::~AngerMode()
        {

        }

        Status AngerMode::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            //Ogre::LogManager::getSingleton().logMessage("AngerMode!");
            m_AI->hitAngerMode();
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AngerMode::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AngerMode::onTerminate( Status status )
        {   
            if(status == BH_SUCCESS)
                m_bb->setStateBool("IsActorAM",true);
        }

        //////////////////////////////////////////////////////////////////////////

        ExploreSpot::ExploreSpot(ActorAI* ai) 
            : Action(ai)
        {

        }

        ExploreSpot::~ExploreSpot()
        {

        }

        Status ExploreSpot::update()
        {
            if(!isValid())
                return BH_FAILURE;

            bool isValid = true;
            mkVec3 lastSeenSpot = m_bb->getStateVec3("LastEnemySpot", isValid);

            if(!isValid)
                return BH_FAILURE;

            if((lastSeenSpot - m_AI->getSimPos()).length() < 3.f)
                return BH_SUCCESS;

            if(getStatus() != BH_RUNNING){
                //Ogre::LogManager::getSingleton().logMessage("Explore spot!");
                size_t steps = 40;
                mkVec3 new_direction = lastSeenSpot - m_AI->getSimPos();
                new_direction.normalise();
                m_AI->startSmoothChangeDir(new_direction, steps, getDuration());
                m_AI->setSpeed(1.f);
            }

            return BH_RUNNING;
        }

        void ExploreSpot::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void ExploreSpot::onTerminate( Status status )
        {   
            m_bb->setStateBool("IsEnemySeen",false);
        }

        //////////////////////////////////////////////////////////////////////////

        Rotate::Rotate(ActorAI* ai) 
            : Action(ai)
        {

        }

        Rotate::~Rotate()
        {

        }

        Status Rotate::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if(!isValid)
                return BH_FAILURE;

            //Ogre::LogManager::getSingleton().logMessage("Rotate!");
            size_t steps = 40;
            mkVec3 new_direction = enemyPos - m_AI->getSimPos();
            new_direction.normalise();
            m_AI->startSmoothChangeDir(new_direction, steps, getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void Rotate::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void Rotate::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }

        //////////////////////////////////////////////////////////////////////////

        KeepDistance::KeepDistance(ActorAI* ai) 
            : Action(ai)
        {

        }

        KeepDistance::~KeepDistance()
        {

        }

        Status KeepDistance::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            mkVec3 enemyPos = m_bb->getStateVec3("EnemyPos", isValid);

            if(!isValid)
                return BH_FAILURE;

            //Ogre::LogManager::getSingleton().logMessage("KeepDistance!");
            size_t steps = 40;
            mkVec3 new_direction = m_AI->getSimPos() - enemyPos;
            new_direction.normalise();

            RayCastResult ray_result = m_AI->raycast(new_direction, 1.0f, 10.f);
            if(ray_result.hit && ray_result.collision_type == RayCastResult::Environment){
                return BH_FAILURE;
            } else {
                m_AI->startSmoothChangeDir(new_direction, steps, getDuration()/3);
                m_AI->setSpeed(1.0f);
                return BH_RUNNING;
            }

            return BH_RUNNING;
        }

        void KeepDistance::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void KeepDistance::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }

        //////////////////////////////////////////////////////////////////////////

        AnimAttackMelee::AnimAttackMelee(ActorAI* ai) 
            : Action(ai)
        {

        }

        AnimAttackMelee::~AnimAttackMelee()
        {

        }

        Status AnimAttackMelee::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            m_AI->runAnimation("Attack3",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AnimAttackMelee::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AnimAttackMelee::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        AnimAttackPunch::AnimAttackPunch(ActorAI* ai) 
            : Action(ai)
        {

        }

        AnimAttackPunch::~AnimAttackPunch()
        {

        }

        Status AnimAttackPunch::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            m_AI->runAnimation("Attack1",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AnimAttackPunch::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AnimAttackPunch::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        AnimAngerMode::AnimAngerMode(ActorAI* ai) 
            : Action(ai)
        {

        }

        AnimAngerMode::~AnimAngerMode()
        {

        }

        Status AnimAngerMode::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            m_AI->runAnimation("HighJump",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AnimAngerMode::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AnimAngerMode::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        AnimJump::AnimJump(ActorAI* ai) 
            : Action(ai)
        {

        }

        AnimJump::~AnimJump()
        {

        }

        Status AnimJump::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            m_AI->runAnimation("Jump",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AnimJump::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AnimJump::onTerminate( Status status )
        {
            if(status == BH_SUCCESS)
                m_bb->setStateFloat("LastJumpTime",g_game->getTimeMs());
        }

        //////////////////////////////////////////////////////////////////////////

        AnimBackflip::AnimBackflip(ActorAI* ai) 
            : Action(ai)
        {

        }

        AnimBackflip::~AnimBackflip()
        {

        }

        Status AnimBackflip::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            m_AI->runAnimation("Backflip",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void AnimBackflip::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void AnimBackflip::onTerminate( Status status )
        {
            m_bb->setStateBool("GotHit",false);
        }

    }

    namespace EXPERIMENTS {

        GotoShop::GotoShop(ActorAI* ai) 
            : Action(ai)
        {

        }

        GotoShop::~GotoShop()
        {

        }

        Status GotoShop::update()
        {
            if(!isValid())
                return BH_FAILURE;

            bool isValid = true;
            mkVec3 shopSpot = m_bb->getStateVec3("ShopSpot", isValid);
            float spotRadius = m_bb->getStateFloat("SpotRadius", isValid);

            if(!isValid)
                return BH_FAILURE;

            if((shopSpot - m_AI->getSimPos()).length() < spotRadius)
                return BH_SUCCESS;

            mkVec3 new_direction = shopSpot - m_AI->getSimPos();
            new_direction.normalise();

            size_t steps = 40;
            m_AI->startSmoothChangeDir(new_direction, steps, 500.f);
            m_AI->setSpeed(0.7f);


            RayCastResult ray_result = m_AI->raycast(new_direction, 0.1f, 0.5f);
            if (ray_result.hit && ray_result.collision_type == RayCastResult::Environment)
                m_AI->jump();

            return BH_RUNNING;
        }

        void GotoShop::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void GotoShop::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        GotoMine::GotoMine(ActorAI* ai) 
            : Action(ai)
        {

        }

        GotoMine::~GotoMine()
        {

        }

        Status GotoMine::update()
        {
            if(!isValid())
                return BH_FAILURE;

            bool isValid = true;
            mkVec3 mineSpot = m_bb->getStateVec3("MineSpot", isValid);
            float spotRadius = m_bb->getStateFloat("SpotRadius", isValid);

            if(!isValid)
                return BH_FAILURE;

            if((mineSpot - m_AI->getSimPos()).length() < spotRadius)
                return BH_SUCCESS;

            mkVec3 new_direction = mineSpot - m_AI->getSimPos();
            new_direction.normalise();
            size_t steps = 40;
            m_AI->startSmoothChangeDir(new_direction, steps, 500.f);
            m_AI->setSpeed(0.7f);

            RayCastResult ray_result = m_AI->raycast(new_direction, 0.1f, 0.5f);
            if (ray_result.hit && ray_result.collision_type == RayCastResult::Environment)
                m_AI->jump();

            return BH_RUNNING;
        }

        void GotoMine::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void GotoMine::onTerminate( Status status )
        {
        }

        //////////////////////////////////////////////////////////////////////////

        DigGold::DigGold(ActorAI* ai) 
            : Action(ai)
        {

        }

        DigGold::~DigGold()
        {

        }

        Status DigGold::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            float gold = m_bb->getStateFloat("NPCGold", isValid);
            bool pickaxe = m_bb->getStateBool("IsPickaxeBought",isValid);

            if(!isValid)
                return BH_FAILURE;

            if(!pickaxe){
                m_bb->setStateFloat("NPCGold",gold+10.f);
                m_AI->runAnimation("Attack1",getDuration());
            } else {
                m_bb->setStateFloat("NPCGold",gold+25.f);
                m_AI->runAnimation("Attack3",getDuration());
            }

            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void DigGold::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void DigGold::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        BuyPickaxe::BuyPickaxe(ActorAI* ai) 
            : Action(ai)
        {

        }

        BuyPickaxe::~BuyPickaxe()
        {

        }

        Status BuyPickaxe::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            float gold = m_bb->getStateFloat("NPCGold", isValid);

            if(!isValid)
                return BH_FAILURE;

            m_bb->setStateFloat("NPCGold",gold-200.f);
            m_bb->setStateBool("IsPickaxeBought",true);
            m_AI->runAnimation("Attack3",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void BuyPickaxe::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void BuyPickaxe::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        BuyHelmet::BuyHelmet(ActorAI* ai) 
            : Action(ai)
        {

        }

        BuyHelmet::~BuyHelmet()
        {

        }

        Status BuyHelmet::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            float gold = m_bb->getStateFloat("NPCGold", isValid);

            if(!isValid)
                return BH_FAILURE;

            m_bb->setStateFloat("NPCGold",gold-400.f);
            m_bb->setStateBool("IsHelmetBought",true);
            m_AI->runAnimation("Attack3",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void BuyHelmet::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void BuyHelmet::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        BuyLantern::BuyLantern(ActorAI* ai) 
            : Action(ai)
        {

        }

        BuyLantern::~BuyLantern()
        {

        }

        Status BuyLantern::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            bool isValid = true;
            float gold = m_bb->getStateFloat("NPCGold", isValid);

            if(!isValid)
                return BH_FAILURE;

            m_bb->setStateFloat("NPCGold",gold-600.f);
            m_bb->setStateBool("IsLanternBought",true);
            m_AI->runAnimation("Attack3",getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void BuyLantern::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void BuyLantern::onTerminate( Status status )
        {   
        }

        //////////////////////////////////////////////////////////////////////////

        Idle::Idle(ActorAI* ai) 
            : Action(ai)
        {

        }

        Idle::~Idle()
        {

        }

        Status Idle::update()
        {
            if(!isValid())
                return BH_FAILURE;

            Status s = runUntil();
            if(s != BH_INVALID)
                return s;

            //Ogre::LogManager::getSingleton().logMessage("Idle!");
            size_t steps = 40;
            mkVec3 new_direction = getRandomHorizontalDir();
            new_direction.normalise();
            m_AI->startSmoothChangeDir(new_direction, steps, getDuration());
            m_AI->setSpeed(0.f);

            return BH_RUNNING;
        }

        void Idle::onInitialize(BlackBoard* bb)
        {
            __super::onInitialize(bb);
        }

        void Idle::onTerminate( Status status )
        {   
            m_AI->stopSmoothChangeDir();
        }

    }
}