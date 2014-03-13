#include "pch.h"
#include "BT/BlackBoard.h"

namespace BT {
    BlackBoard::BlackBoard(){}

    BlackBoard::~BlackBoard(){}

    void BlackBoard::init(){
        //const////////////////////////////////////////////////////////////////////////
        m_state["True"] = true;
        m_state["False"] = false;
        m_state["Zero"] = 0.f;
        m_state["Vec0"] = mkVec3::ZERO;
        //ai_specific////////////////////////////////////////////////////////////////////////
        m_state["rngMelee"] = 3.f;
        m_state["rngFbMax"] = 10.f;
        //conditions////////////////////////////////////////////////////////////////////////
        m_state["IsEnemyVisible"] = false;
        m_state["IsEnemyDead"] = false;
        m_state["EnemyDistance"] = 10000.f;
        m_state["IsEnemyAttack"] = false;
        m_state["IsEnemyRunningAway"] = false;
        m_state["ActorHealth"] = 0.f;
        m_state["HealthAMLimit"] = 0.f;
        m_state["IsActorAM"] = false;
        m_state["IsEnemySeen"] = false;
        m_state["EnemyDgrDiff"] = 0.f;
        //bb vals////////////////////////////////////////////////////////////////////////
        m_state["AttackDir"] = mkVec3::ZERO;
        m_state["EnemyPos"] = mkVec3::ZERO;
        m_state["LastEnemySpot"] = mkVec3::ZERO;
    }

    bool BlackBoard::contains( const std::string& key ) const
    {
        std::map<std::string, aiVariant>::const_iterator it = m_state.find(key);
        if(it != m_state.end())
            return true;
        return false;
    }

    void BlackBoard::setStateVec3( std::string key, mkVec3 vec3 )
    {
        m_state[key] = vec3;
    }

    mkVec3 BlackBoard::getStateVec3( std::string key, bool& isValid )
    {
        try{
            mkVec3 res;
            res = boost::get<mkVec3>(m_state[key]);
            isValid = true;
            return res;
        } catch (boost::bad_get&){ Ogre::LogManager::getSingleton().logMessage("getStateVec3: bad_get exception!"); }
        isValid = false;
        return mkVec3::ZERO;
    }

    void BlackBoard::setStateBool( std::string key, bool val )
    {
        m_state[key] = val;
    }

    bool BlackBoard::getStateBool( std::string key, bool& isValid )
    {
        try{
            bool res;
            res = boost::get<bool>(m_state[key]);
            isValid = true;
            return res;
        } catch (boost::bad_get&){ Ogre::LogManager::getSingleton().logMessage("getStateBool: bad_get exception!"); }
        isValid = false;
        return false;
    }

    void BlackBoard::setStateFloat( std::string key, float val )
    {
        m_state[key] = val;
    }

    float BlackBoard::getStateFloat( std::string key, bool& isValid )
    {
        try{
            float res;
            res = boost::get<float>(m_state[key]);
            isValid = true;
            return res;
        } catch (boost::bad_get&){ Ogre::LogManager::getSingleton().logMessage("getStateFloat: bad_get exception!"); }
        isValid = false;
        return -1.f;
    }

    void BlackBoard::setStateVariant( std::string key, aiVariant val )
    {
        m_state[key] = val;
    }

    aiVariant BlackBoard::getStateVariant( std::string key )
    {
        return m_state[key];
    }
}