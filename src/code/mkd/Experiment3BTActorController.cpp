#include "pch.h"
#include "Experiment3BTActorController.h"

Experiment3BTActorController::Experiment3BTActorController( ActorAI* ai ) : IActorController(ai)
{
    m_bb = new BT::BlackBoard();
    m_bb->init();

    m_parser = new BT::Parser();
}

Experiment3BTActorController::~Experiment3BTActorController()
{
    delete m_bb;
    delete m_root;
    delete m_parser;
}

//////////////////////////////////////////////////////////////////////////

void Experiment3BTActorController::onCreate()
{
    getAI()->setDirection(mkVec3::UNIT_Z);

    m_parser->parseXmlTree(getAI()->getBtTreePath(), getAI(), m_root);
    m_parser->parseAliases(m_bb);

    m_bb->setStateFloat("rngMelee",getAI()->getMeleeRange());
    m_bb->setStateFloat("LastJumpTime",0.f);
    m_bb->setStateBool("GotHit",false);

    Ogre::LogManager::getSingleton().logMessage("BT controller created!");
}

void Experiment3BTActorController::onTakeDamage( const SDamageInfo& dmg_info )
{
    m_bb->setStateBool("GotHit",true);
}

void Experiment3BTActorController::onUpdate( float dt )
{
    updateWorldState(dt);
    BT::Status s = m_root->tick(m_bb);

    if(m_parser->isFileModified(getAI()->getBtTreePath()) && m_root->isTerminated()){
        m_parser->parseXmlTree(getAI()->getBtTreePath(), getAI(), m_root);
        m_parser->parseAliases(m_bb);
    }
}

void Experiment3BTActorController::onDebugDraw()
{
    getAI()->drawSensesInfo();
}

void Experiment3BTActorController::onDie()
{
}

//////////////////////////////////////////////////////////////////////////

void Experiment3BTActorController::updateWorldState(float dt)
{
    Character* enemy = getAI()->findClosestEnemyInSight();
    bool isValid = true;
    float time = g_game->getTimeMs() - m_bb->getStateFloat("LastJumpTime",isValid);
    m_bb->setStateFloat("JumpTime",time);
    if(enemy){
        m_bb->setStateFloat("EnemyDistance",(enemy->getSimPos()-getAI()->getSimPos()).length());
    } else {
        m_bb->setStateFloat("EnemyDistance",FLT_MAX);
    }
}