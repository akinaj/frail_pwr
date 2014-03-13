/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "Player.h"
#include "Level.h"
#include "Game.h"
#include "contrib/DebugDrawer.h"

IMPLEMENT_RTTI_NOSCRIPT(Player, Character);

START_RTTI_INIT(Player);
{
    FIELD_FLOAT(m_health);
    FIELD_FLOAT(m_maxHealth);
}
END_RTTI_INIT();

Player::Player()
    : m_health(100.f)
    , m_maxHealth(100.f)
    , m_dbgCurrAnimPreviewIdx(-1)
    , m_dbgAnimPreviewStartTime(-1.f)
    , m_timeInDeathLeft(-1.f)
{
    setName("Player");
}

void Player::onPostCreate()
{
    __super::onPostCreate();

    getLevel()->setLocalPlayer(this);
    setMaxSpeed(0.12f);
}

void Player::onDestroy()
{
    __super::onDestroy();

    getLevel()->setLocalPlayer(NULL);
}

void Player::shoot()
{
    static float RANGE_METERS = 200.f;
    Character::shoot(g_game->getCamera()->getPosition(), g_game->getCamera()->getForwardVec(), RANGE_METERS);
}

mkVec3 Player::getPosForVisualsNode() const
{
    /* old fpp code
    static float VERT_OFFSET = .2f;

    mkVec3 low_point = getSimPos();
    mkVec3 high_point = low_point;
    high_point.y += VERT_OFFSET;

    return high_point;*/
    return getSimPos() + getVisStartOffset();
}

void Player::addHealth( float val )
{
    m_health += val;
    if (m_health > getMaxHealth())
        m_health = getMaxHealth();
}

mkVec3 Player::CameraTarget_getWorldPosition() const
{
    return getWorldPosition();
}

mkVec3 Player::getMeleeForwardDir() const
{
    mkVec3 forward = g_game->getCamera()->getForwardVec();
    forward.y = 0;

    return forward;
}

float Player::getHealth() const
{
    return m_health;
}

float Player::getMaxHealth() const
{
    return m_maxHealth;
}

void Player::onTakeDamage( const SDamageInfo& dmg_info )
{
    m_health -= dmg_info.dmg;

    if (m_health < 0.f)
        m_health = 0.f;
}

mkVec3 Player::getDirForRangedAttack() const
{
    ICamera* cam = g_game->getCamera();

    //DebugDrawer::getSingleton().drawLine(cam->getPosition(), cam->getPosition() + cam->getForwardVec() * 100.f, Ogre::ColourValue::White);
    //DebugDrawer::getSingleton().drawSphere(cam->getPosition(), 0.1f, Ogre::ColourValue::Black);

    static float MAX_RANGE = 1000.f;
    const btVector3 start_pos = ogre_to_bullet(cam->getPosition());
    const btVector3 forward_vec = ogre_to_bullet(cam->getForwardVec());
    const btVector3 ray_end = start_pos + forward_vec.normalized() * MAX_RANGE;

    btCollisionWorld::ClosestRayResultCallback ray_callback(start_pos, ray_end);
    g_game->getPhysicsWorld()->rayTest(start_pos, ray_end, ray_callback);

    const mkVec3 my_start = getPosForRangedAttackStart();

    if (ray_callback.hasHit())
    {
        return (bullet_to_ogre(ray_callback.m_hitPointWorld) - my_start);
    }
    else
    {
        const mkVec3 lookat_pos = cam->getLookatPos();

        return (lookat_pos - my_start);
    }
}

void Player::dbgNextAnimPreview()
{
    if (getAnimContext().getAvailableClips().empty())
        return;

    m_dbgCurrAnimPreviewIdx = (m_dbgCurrAnimPreviewIdx + 1) % getAnimContext().getAvailableClips().size();
}

void Player::dbgPrevAnimPreview()
{
    if (getAnimContext().getAvailableClips().empty())
        return;

    m_dbgCurrAnimPreviewIdx = (m_dbgCurrAnimPreviewIdx - 1);
    if (m_dbgCurrAnimPreviewIdx < 0)
        m_dbgCurrAnimPreviewIdx = getAnimContext().getAvailableClips().size() - 1;
}

void Player::dbgPlayAnimPreview()
{
    if (getDbgClipName().empty())
        return;

    log_error("Clip name: '%s'", getDbgClipName().c_str());

    m_dbgAnimPreviewStartTime = getTimeMs();
    getAnimContext().setAnimationClipPlayed(getDbgClipName().c_str());
}

void Player::updateVisAnimation( float dt )
{
    if (getAnimContext().isReady())
    {
        if (updateDbgAnim())
            return;

        if (updateDeadAnim())
            return;
    }

    __super::updateVisAnimation(dt);
}

mkString Player::getDbgClipName() const
{
    const AnimatedMeshContext::TClipNamesVec& clips = getAnimContext().getAvailableClips();
    if (m_dbgCurrAnimPreviewIdx >= 0 && m_dbgCurrAnimPreviewIdx < (int)clips.size())
        return clips[m_dbgCurrAnimPreviewIdx];

    return "";
}

bool Player::isDead() const
{
    return false;//m_health <= 0.f;
}

bool Player::updateDbgAnim()
{
    if (!getDbgClipName().empty() && getAnimContext().isReady())
    {
        const float clip_len = getAnimContext().getClipLength(getDbgClipName().c_str());
        const float time_since_played = (getTimeMs() - m_dbgAnimPreviewStartTime) * 0.001f;

        if (time_since_played < clip_len)
        {
            getAnimContext().update(getTimeDelta());
            getAnimContext().setLooped(false);

            return true;
        }
    }

    return false;
}

bool Player::updateDeadAnim()
{
    if (m_health > 0.f)
        return false;

    const mkString& death_clip_name = "Death1";
    if (!getAnimContext().hasAnimationClip(death_clip_name.c_str()))
        return false;

    getAnimContext().setLooped(false);
    getAnimContext().setAnimationClipPlayed(death_clip_name.c_str());
    getAnimContext().update(getTimeDelta());

    return true;
}

void Player::onUpdate()
{
    __super::onUpdate();

    if (m_timeInDeathLeft == -1.f && m_health <= 0.f)
    {
        m_timeInDeathLeft = 3.f;
    }

    if (m_timeInDeathLeft >= 0.f)
    {
        m_timeInDeathLeft -= getTimeDelta();
        if (m_timeInDeathLeft < 0.f)
        {
            m_health = getMaxHealth();
            m_timeInDeathLeft = -1.f;
        }
    }

    if (m_health <= 0.f)
        setSpeed(0.f);
}