/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "BoidActorController.h"
#include "contrib/DebugDrawer.h"

BoidActorController::BoidActorController( ActorAI* ai )
    : IActorController(ai)
    , m_wantedVelocity(mkVec3::ZERO)
    , m_dbgCohesionVec(mkVec3::ZERO)
    , m_dbgAlignmentVec(mkVec3::ZERO)
    , m_dbgSeparationVec(mkVec3::ZERO)
{

}

BoidActorController::~BoidActorController()
{

}

void BoidActorController::onCreate()
{
    m_wantedVelocity = getRandomHorizontalDir();
    applyWantedVelocity();
}

void BoidActorController::onTakeDamage(const SDamageInfo& dmg_info)
{

}

void BoidActorController::onDebugDraw()
{
    const mkVec3 my_pos = getAI()->getSimPos() + mkVec3(0, 1, 0);
    DebugDrawer::getSingleton().drawArrow(my_pos, my_pos + m_dbgCohesionVec, Ogre::ColourValue(1, 0, 0, 1));
    DebugDrawer::getSingleton().drawArrow(my_pos, my_pos + m_dbgSeparationVec, Ogre::ColourValue(0, 1, 0, 1));
    DebugDrawer::getSingleton().drawArrow(my_pos, my_pos + m_dbgAlignmentVec, Ogre::ColourValue(1, 1, 0, 1));

    for (size_t i = 0; i < m_obstacles.size(); ++i)
    {
        DebugDrawer::getSingleton().drawLine(my_pos, m_obstacles[i].pos, Ogre::ColourValue(1, 1, 1, 0.1f));
    }
}

void BoidActorController::onUpdate( float dt )
{
    if (dt == 0.f)
        return;

    updateObstacles();
    updateNeighbors();
    updateWantedVelocity(dt);
    applyWantedVelocity();
}

void BoidActorController::updateWantedVelocity(float dt)
{
    // Framerate independent, but numbers where easier to tweak with this factor
    static float ACC_FACTOR = 1/60.f;

    const float max_speed = getAI()->getMaxSpeed();
    m_wantedVelocity += calcAcceleration() * ACC_FACTOR;
    if (m_wantedVelocity.squaredLength() > max_speed * max_speed)
    {
        m_wantedVelocity.normalise();
        m_wantedVelocity *= max_speed;
    }
}

void BoidActorController::applyWantedVelocity()
{
    const mkVec3 dir = m_wantedVelocity.normalisedCopy();
    getAI()->setDirection(dir);

    const float max_speed = getAI()->getMaxSpeed();
    const float speed = m_wantedVelocity.length();
    const float speed_part = clamp(speed / max_speed, 0.f, 1.f);
    getAI()->setSpeed(speed_part * .5f);
}

mkVec3 damp_force(const mkVec3& force, float max_len)
{
    const float len = force.length();
    if (len == 0.f)
        return force;

    if (len > max_len)
        return max_len * mkVec3(force) / len;

    return force;
}

mkVec3 BoidActorController::calcAcceleration()
{
    mkVec3 acc = mkVec3::ZERO;

    static float alignment_weight = 1.f;
    static float separation_weight = 3.f;
    static float cohesion_weight = 1.f;

    static float max_steering_force = 0.1f;

    const mkVec3 alignment_acc = damp_force(calcAlignmentAcc(), max_steering_force);
    const mkVec3 separation_acc = calcSeparationAcc();
    const mkVec3 cohesion_acc = damp_force(calcCohesionAcc(), max_steering_force);

    m_dbgSeparationVec *= separation_weight;

    acc += alignment_acc * alignment_weight;
    acc += separation_acc * separation_weight;
    acc += cohesion_acc * cohesion_weight;

    acc.y = 0.f;

    return acc;
}

mkVec3 BoidActorController::calcAlignmentAcc()
{
    if (m_neighbors.empty())
        return mkVec3::ZERO;

    mkVec3 mean_velocity = mkVec3::ZERO;
    const float mean_factor = 1.f / m_neighbors.size();
    for (size_t i = 0; i < m_neighbors.size(); ++i)
    {
        mean_velocity += m_neighbors[i]->getSimDir() * m_neighbors[i]->getRealSpeed() * mean_factor;
    }

    m_dbgAlignmentVec = mean_velocity;
    return mean_velocity;
}

mkVec3 BoidActorController::calcCohesionAcc()
{
    if (m_neighbors.empty())
        return mkVec3::ZERO;

    mkVec3 mean_pos = mkVec3::ZERO;
    const float mean_factor = 1.f / m_neighbors.size();
    for (size_t i = 0; i < m_neighbors.size(); ++i)
    {
        mean_pos += m_neighbors[i]->getSimPos() * mean_factor;
    }

    const mkVec3 cohesion_vec = mean_pos - getAI()->getSimPos();
    m_dbgCohesionVec = cohesion_vec;

    const mkVec3 wanted_change = cohesion_vec - m_wantedVelocity; // todo easing
    return wanted_change;
}

mkVec3 BoidActorController::calcSeparationAcc()
{
    static float min_dist = 7.f;
    mkVec3 mean_sep = mkVec3::ZERO;
    const mkVec3 my_pos = getAI()->getSimPos();
    float neighbors_to_separate = 0.f;
    for (size_t i = 0; i < m_neighbors.size(); ++i)
    {
        const mkVec3 other_pos = m_neighbors[i]->getSimPos();
        const float dist_to_neighbor = my_pos.distance(other_pos);

        if (dist_to_neighbor <= min_dist)
        {
            neighbors_to_separate += 1.f;
            mkVec3 separation_dir = my_pos - other_pos;
            separation_dir.normalise();

            separation_dir /= dist_to_neighbor;

            mean_sep += separation_dir;
        }
    }

    static bool avoid_obstacles = true;
    if (avoid_obstacles)
    {
        static float obstacle_radius = 3.5f;

        for (size_t i = 0; i < m_obstacles.size(); ++i)
        {
            const mkVec3 obstacle_pos = m_obstacles[i].pos;
            const float dist_to_obstacle = my_pos.distance(obstacle_pos);

            if (dist_to_obstacle <= obstacle_radius)
            {
                neighbors_to_separate += 1.f;
                mkVec3 separation_dir = my_pos - obstacle_pos;
                separation_dir.normalise();

                separation_dir /= obstacle_radius;

                mean_sep += separation_dir;
            }
        }
    }

    if (neighbors_to_separate > 0.f)
        mean_sep *= 1.f / neighbors_to_separate;

    m_dbgSeparationVec = mean_sep;

    return mean_sep;
}

void BoidActorController::updateNeighbors()
{
    m_neighbors.clear();
    
    CharacterVec characters;
    getAI()->getCharactersFromSameTeam(characters);

    static float neighbor_radius = 10.f;
    const mkVec3 my_pos = getAI()->getSimPos();
    for (size_t i = 0; i < characters.size(); ++i)
    {
        Character* other = characters[i];
        if (other == getAI())
            continue;

        const mkVec3 pos = other->getSimPos();
        const float dist2 = my_pos.squaredDistance(pos);
        if (dist2 < neighbor_radius * neighbor_radius)
            m_neighbors.push_back(other);
    }
}

void BoidActorController::updateObstacles()
{
    static int num_rays = 10;
    static float ray_len = 15.f;

    m_obstacles.clear();

    const float angle_step = Ogre::Math::TWO_PI / num_rays;
    for (int i = 0; i < num_rays; ++i)
    {
        const float angle = i * angle_step;
        const mkVec3 dir = rotate_horz_vec(getAI()->getSimDir(), angle);

        RayCastResult result = getAI()->raycast(dir, 0.1f, ray_len);
        if (result.hit && result.collision_type != RayCastResult::Actor)
        {
            Obstacle obstacle;
            obstacle.pos = result.point;

            m_obstacles.push_back(obstacle);
        }
    }
}
