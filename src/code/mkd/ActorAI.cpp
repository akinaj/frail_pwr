/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "utils.h"
#include "Game.h"
#include "ActorAI.h"
#include "IActorController.h"
#include "contrib/DebugDrawer.h"
#include "Level.h"
#include "ActorControllerFactory.h"
#include "scripting/LuaSimpleBinding.h"

// TODO create actorAiCtf with ctf-specific stuff
#include "ctf/TeamFlag.h"
#include "ctf/CtfMgr.h"

IMPLEMENT_RTTI_SCRIPTED(ActorAI, Character);

START_RTTI_INIT(ActorAI);
{
    FIELD_FLOAT(m_health);
    FIELD_FLOAT(m_maxHealth);
    FIELD_INT32(m_teamNumber);
    FIELD_STRING(m_characterCtrlName);
    FIELD_STRING(m_btTreePath);
    FIELD_STRING(m_htnMethodsPath);
    FIELD_STRING(m_htnOperatorsPath);
    FIELD_STRING(m_htnGoalsPath);
}
END_RTTI_INIT();

static void _script_findSpottedActors(GameObject* actor, TGameObjectVec& out_vec)
{
    if (!actor->getTypeInfo()->isDerivedOrExact(&ActorAI::Type))
        return;

    const CharacterVec& actors = static_cast<ActorAI*>(actor)->getSpottedActors();
    out_vec.reserve(actors.size());
    for (size_t i = 0; i < actors.size(); ++i)
        out_vec.push_back(actors[i]);
}

EXPORT_VOID_METHOD_SCRIPT(ActorAI, drawSensesInfo);

START_SCRIPT_REGISTRATION(ActorAI, ctx);
{
    ctx->registerFunc("GetSpottedActors", _script_findSpottedActors);
    ctx->registerFunc("DrawSensesInfo", VOID_METHOD_SCRIPT(ActorAI, drawSensesInfo));
}
END_SCRIPT_REGISTRATION(ctx);

ActorAI::ActorAI()
    : m_controller(NULL)
    , m_lastDamageTime(-1.f)
    , m_health(100.f)
    , m_teamNumber(-1)
{
    setCanJump(false);
    m_btTreePath = "";
    m_htnMethodsPath = "";
    m_htnOperatorsPath = "";
    m_htnGoalsPath = "";
}

void ActorAI::onTakeDamage(const SDamageInfo& dmg_info)
{
    __super::onTakeDamage(dmg_info);

    m_health -= dmg_info.dmg;
    m_lastDamageTime = g_game->getTimeMs();

    if (m_controller)
        m_controller->onTakeDamage(dmg_info);

    if (isDead())
        onDie();
}

void ActorAI::onDie()
{
    __super::onDie();
    m_controller->onDie();

    if (m_carriedFlag.isSet())
        dropFlag();
}

void ActorAI::onCreate()
{
    __super::onCreate();

    btGhostObject* ghost_obj = getKinematicController()->getGhostObject();

    PhysicsObjUserData* user_data = new PhysicsObjUserData;
    user_data->character = this;
    user_data->type = PhysicsObjUserData::CharacterObject | PhysicsObjUserData::EnemyObject;
    ghost_obj->setUserPointer(user_data);

    if (!m_characterCtrlName.empty())
        setController(getLevel()->getActorControllerFactory()->create(m_characterCtrlName.c_str(), this));
}

void ActorAI::onDestroy()
{
    __super::onDestroy();

    // destroy character ctrl?
}

void ActorAI::onUpdate()
{
    __super::onUpdate();

    updateSpottedActors();

    if (m_controller && !isDead())
        m_controller->onUpdate(getTimeDelta());
}

void ActorAI::updateSpottedActors()
{
    m_spottedActors.clear();
    getCharactersInSight(m_spottedActors);
    getCharactersBySmell(m_spottedActors);
}

void ActorAI::onDebugDraw()
{
    __super::onDebugDraw();

    if (!isDead())
    {
        drawMovementInfo();
        drawTakenDamage();

        //drawSensesInfo(); // depends on controller

        if (m_controller)
            m_controller->onDebugDraw();
    }

    if (getScriptContext())
        getScriptContext()->tryCallFunc("onDebugDraw");
}

void ActorAI::drawSensesInfo()
{
    drawSightArea();
    drawSight();
    drawSmellSense();
}

void ActorAI::drawSight()
{
    const mkVec3 my_pos = getEyePos();
    for (int i = 0; i < (int)m_spottedActors.size(); ++i)
    {
        Character* spotted_actor = m_spottedActors[i];
        const mkVec3 pos = spotted_actor->getEyePos();

        Ogre::ColourValue colour = Ogre::ColourValue(0.8f, 0.8f, 0.8f, 0.5f); // default colour for neutral actors
        if (isEnemy(spotted_actor))
            colour = Ogre::ColourValue::Red;
        else if (isAlly(spotted_actor))
            colour = Ogre::ColourValue::Blue;

        DebugDrawer::getSingleton().drawLine(my_pos, pos, colour);
//         const mkVec3 rel_pos = pos - my_pos;
//         const mkVec3 arrow_end = my_pos + rel_pos * 0.75f;
//        DebugDrawer::getSingleton().drawArrow(my_pos, arrow_end, Ogre::ColourValue::White);
    }
}

void ActorAI::drawMovementInfo()
{
    static float len = 2.f;
    DebugDrawer::getSingleton().drawArrow(getSimPos() + mkVec3(0, 0.05f, 0.f), getSimPos() + getSimDir() * len + mkVec3(0, 0.05f, 0.f), Ogre::ColourValue::White);
    //DebugDrawer::getSingleton().drawCircleSector(getSimPos(), 4.f, M_PI_2, getSimDir(), 10, Ogre::ColourValue::White);
}

void ActorAI::drawSmellSense()
{
    if(getSmellRange() > 0.f){
        mkVec3 circlePos = getSimPos();
        circlePos.y = 0.05f;
        DebugDrawer::getSingleton().drawCircle(circlePos, getSmellRange(), 30, Ogre::ColourValue(0.f,1.f,0.f,0.1f), true);
    }
}

void ActorAI::drawTakenDamage()
{
    const float damage_visible_time = 750.f;
    const float time_since_last_dmg = g_game->getTimeMs() - m_lastDamageTime;

    if (time_since_last_dmg < damage_visible_time && m_lastDamageTime != -1.f)
    {
        const float power = 1.f - time_since_last_dmg / damage_visible_time;
        Ogre::ColourValue colour = Ogre::ColourValue(power, 0.f, 0.f, power);
        mkVec3 position = getSimPos();
        position.y = Ogre::Real(0.1);
        DebugDrawer::getSingleton().drawCircle(position, 5.f, 30, colour, true);
    }
}

void ActorAI::setController( IActorController* ctrlr )
{
    MK_ASSERT(ctrlr);
    MK_ASSERT(m_controller != ctrlr);

    if (m_controller)
        m_controller->setAI(NULL);
    
    m_controller = ctrlr;
}

mkVec3 ActorAI::getPosForVisualsNode() const
{
    return getSimPos() + getVisStartOffset();
}

void ActorAI::onDbgKeyDown( OIS::KeyCode key )
{
    __super::onDbgKeyDown(key);

    if (m_controller)
        m_controller->onDbgKeyDown(key);
}

void ActorAI::revive()
{
    m_health = m_maxHealth;
}

bool ActorAI::pickUpFlag( EConflictSide::TYPE flag_owner )
{
    MK_ASSERT(!m_carriedFlag.isSet());

    if (getLevel()->getCtfMgr()->isTeamFlagAlreadyCarried(flag_owner))
    {
        log_info("%d from %s tried to pick up %s flag, but it's already carried by someone else", getTeamNumber(),
            EConflictSide::toString(getConflictSide()).c_str(), EConflictSide::toString(flag_owner).c_str());
        return false;
    }

    TeamFlag* team_flag = getLevel()->getCtfMgr()->getTeamFlag(flag_owner);
    MK_ASSERT(team_flag);

    const float pickup_radius = 2.5f;
    const mkVec3 flag_pos = team_flag->getWorldPosition();
    const mkVec3 my_pos = getSimPos();
    const float dist2_2d = my_pos.squaredDistance(mkVec3(flag_pos.x, my_pos.y, flag_pos.z));

    if (dist2_2d < pickup_radius * pickup_radius)
    {
        log_info("%d from %s picks up %s flag", getTeamNumber(),
            EConflictSide::toString(getConflictSide()).c_str(),
            EConflictSide::toString(flag_owner).c_str());

        team_flag->setCarrier(this);
        m_carriedFlag.setPtr(team_flag);
        return true;
    }
    else
    {
        log_info("%d from %s tried to pick up %s flag but is too far (dist2 = %.1f)", getTeamNumber(),
            EConflictSide::toString(getConflictSide()).c_str(), EConflictSide::toString(flag_owner).c_str(), dist2_2d);

        return false;
    }
}

void ActorAI::dropFlag()
{
    TeamFlag* flag_ptr = m_carriedFlag.fetchPtr();
    if (flag_ptr)
    {
        log_info("%d from %s drops %s flag", getTeamNumber(),
            EConflictSide::toString(getConflictSide()).c_str(),
            EConflictSide::toString(flag_ptr->getTeam()).c_str());

        flag_ptr->resetCarrier();
    }

    m_carriedFlag.setPtr(NULL);
}

bool ActorAI::isCarryingFlag() const
{
    return m_carriedFlag.isSet();
}

bool ActorAI::isCarryingOwnFlag() const
{
    TeamFlag* flag_ptr = m_carriedFlag.fetchPtr();
    return flag_ptr && flag_ptr->getTeam() == getConflictSide();
}

TeamFlag* ActorAI::getCarriedFlag() const
{
    return m_carriedFlag.fetchPtr();
}

mkVec3 ActorAI::getRespawnPos() const
{
    if (getLevel()->getCtfMgr()->isInCtfMode())
        return getLevel()->getCtfMgr()->getTeamBasePos(getConflictSide());
    else
        return getSimPos();
}

void ActorAI::addHealth( float val )
{
    m_health += val;
}

float ActorAI::getHealth() const
{
    return m_health;
}

float ActorAI::getMaxHealth() const
{
    return m_maxHealth;
}

Character* ActorAI::findClosestEnemyInSight() const
{
    const CharacterVec& spotted_actors = getSpottedActors();
    Character* chosen = NULL;
    float minActorDistance = getSightDist()+1.f;
    float currentActorDistance = 0.f;
    for (size_t i = 0; i < spotted_actors.size(); ++i)
    {
        Character* other_character = spotted_actors[i];
        if (isEnemy(other_character) && !other_character->isDead()){
            currentActorDistance = (other_character->getSimPos() - getSimPos()).length();
            if(currentActorDistance < minActorDistance){
                chosen = other_character;
                minActorDistance = currentActorDistance;
            }
        }
    }

    return chosen;
}
