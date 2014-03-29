#include "pch.h"
#include "SampleBTActorController.h"

SampleBTActorController::SampleBTActorController( ActorAI* ai ) : IActorController(ai)
{
    m_bb = new BT::BlackBoard();
    m_bb->init();

    m_parser = new BT::Parser();
}

SampleBTActorController::~SampleBTActorController()
{
    delete m_bb;
    delete m_root;
    delete m_parser;
}

//////////////////////////////////////////////////////////////////////////

void SampleBTActorController::onCreate()
{
    getAI()->setDirection(mkVec3::UNIT_Z);

    m_parser->parseXmlTree(getAI()->getBtTreePath(), getAI(), m_root);
    m_parser->parseAliases(m_bb);

    Ogre::LogManager::getSingleton().logMessage("BT controller created!");
}

void SampleBTActorController::onTakeDamage( const SDamageInfo& dmg_info )
{

}

void SampleBTActorController::onUpdate( float dt )
{
    updateWorldState(dt);
    BT::Status s = m_root->tick(m_bb);

    if(m_parser->isFileModified(getAI()->getBtTreePath()) && m_root->isTerminated()){
        m_parser->parseXmlTree(getAI()->getBtTreePath(), getAI(), m_root);
        m_parser->parseAliases(m_bb);
    }
}

void SampleBTActorController::onDebugDraw()
{
    getAI()->drawSensesInfo();
}

void SampleBTActorController::onDie()
{
}

//////////////////////////////////////////////////////////////////////////

void SampleBTActorController::updateWorldState(float dt)
{
    Character* target = getAI()->findClosestEnemyInSight();
    if(target){
        m_bb->setStateBool("IsEnemyVisible", true);
    } else {
        m_bb->setStateBool("IsEnemyVisible", false);
    }
}