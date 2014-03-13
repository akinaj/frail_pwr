#include "pch.h"
#include "Experiment1BTActorController.h"

#define SPOT_RADIUS 5.f

Experiment1BTActorController::Experiment1BTActorController( ActorAI* ai ) : IActorController(ai)
{
    m_bb = new BT::BlackBoard();
    m_bb->init();

    m_npcGold = 0;

    m_parser = new BT::Parser();
}

Experiment1BTActorController::~Experiment1BTActorController()
{
    delete m_bb;
    delete m_root;
    delete m_parser;
}

//////////////////////////////////////////////////////////////////////////

void Experiment1BTActorController::onCreate()
{
    getAI()->setDirection(mkVec3::UNIT_Z);

    m_parser->parseXmlTree(getAI()->getBtTreePath(), getAI(), m_root);
    m_parser->parseAliases(m_bb);

    m_shopPosition = mkVec3(33.f, 0.1f, 2.f);
    m_minePosition = mkVec3(-33.f,0.1f,2.f);

    m_bb->setStateVec3("MineSpot",m_minePosition);
    m_bb->setStateVec3("ShopSpot",m_shopPosition);
    m_bb->setStateFloat("SpotRadius",SPOT_RADIUS);
    m_bb->setStateFloat("NPCGold",m_npcGold);
    m_bb->setStateBool("IsPickaxeBought",false);
    m_bb->setStateBool("IsHelmetBought",false);
    m_bb->setStateBool("IsLanternBought",false);

    Ogre::LogManager::getSingleton().logMessage("BT controller created!");
}

void Experiment1BTActorController::onTakeDamage( const SDamageInfo& dmg_info )
{
}

void Experiment1BTActorController::onUpdate( float dt )
{
    updateWorldState(dt);
    BT::Status s = m_root->tick(m_bb);

    if(m_parser->isFileModified(getAI()->getBtTreePath()) && m_root->isTerminated()){
        m_parser->parseXmlTree(getAI()->getBtTreePath(), getAI(), m_root);
        m_parser->parseAliases(m_bb);
    }
}

void Experiment1BTActorController::onDebugDraw()
{
    getAI()->drawSensesInfo();
    DebugDrawer::getSingleton().drawCircle(m_shopPosition, SPOT_RADIUS, 30, Ogre::ColourValue(0.f,1.f,0.2f,0.5f), true);
    DebugDrawer::getSingleton().drawCircle(m_minePosition, SPOT_RADIUS, 30, Ogre::ColourValue(1.f,0.9f,0.f,0.5f), true);
}

void Experiment1BTActorController::onDie()
{
}

//////////////////////////////////////////////////////////////////////////

void Experiment1BTActorController::updateWorldState(float dt)
{
    float distance = (getAI()->getSimPos() - m_shopPosition).length();
    if(distance <= SPOT_RADIUS)
        m_bb->setStateBool("IsShopSpotReached",true);
    else
        m_bb->setStateBool("IsShopSpotReached",false);

    distance = (getAI()->getSimPos() - m_minePosition).length();
    if(distance <= SPOT_RADIUS)
        m_bb->setStateBool("IsGoldSpotReached",true);
    else
        m_bb->setStateBool("IsGoldSpotReached",false);
}