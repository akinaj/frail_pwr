#include "pch.h"
#include "BulletManager.h"
#include "contrib/DebugDrawer.h"

IMPLEMENT_RTTI_NOSCRIPT(BulletManager, GameObject);

START_RTTI_INIT(BulletManager);
{

}
END_RTTI_INIT();

BulletManager::BulletManager()
{

}

static float MAX_LIFE_TIME = 10000.f;

void BulletManager::onUpdate()
{
    __super::onUpdate();

    size_t num_bullets = m_bullets.size();

    for (size_t i = 0; i < num_bullets; )
    {
        const float life_time = getTimeMs() - m_bullets[i].creation_time;

        if (life_time > MAX_LIFE_TIME)
        {
            m_bullets[i] = m_bullets.back();
            num_bullets--;
        }
        else
        {
            ++i;
        }
    }

    m_bullets.resize(num_bullets);
}

void BulletManager::onRender()
{
    __super::onRender();

    for (size_t i = 0; i < m_bullets.size(); ++i)
    {
        Ogre::ColourValue colour = Ogre::ColourValue::White;
        if (m_bullets[i].hit)
            colour = Ogre::ColourValue::Red;

        float alpha = 1 - ((getTimeMs() - m_bullets[i].creation_time) / MAX_LIFE_TIME);
        colour.a = alpha;
        DebugDrawer::getSingleton().drawLine(m_bullets[i].start, m_bullets[i].end, colour);
    }
}

void BulletManager::addBullet( const mkVec3& start_point, const mkVec3& end_point, bool hit )
{
    static mkVec3 START_DEBUG_VIS_OFFSET = mkVec3::UNIT_Y * -0.1f;

    SBulletInfo info;
    info.start = start_point + START_DEBUG_VIS_OFFSET;
    info.end = end_point;
    info.hit = hit;
    info.creation_time = getTimeMs();

    m_bullets.push_back(info);
}
