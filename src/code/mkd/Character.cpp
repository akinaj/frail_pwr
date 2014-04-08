/***********************************
* mkdemo 2011-2013                *
* author: Maciej Kurowski 'kurak' *
***********************************/
#include "pch.h"
#include "Character.h"
#include "Game.h"
#include "Level.h"
#include "contrib/DebugDrawer.h"
#include "scripting/LuaSimpleBinding.h"
#include "Rocket.h"

IMPLEMENT_RTTI_SCRIPTED(Character, ModelObject);

START_RTTI_INIT(Character);
{
    FIELD_FLOAT(m_maxSpeed);
    FIELD_BOOL(m_canJump);
    FIELD_FLOAT(m_sightDist);
    FIELD_FLOAT(m_smellRange);
    FIELD_FLOAT(m_horSightAngleRad);
    FIELD_BOOL(m_visibleInSightQueries);
    FIELD_FLOAT(m_shootingRange);
    FIELD_FLOAT(m_shootingDamage);
    FIELD_VEC3(m_currentDir);
    FIELD_FLOAT(m_physicsStepHeight);
    FIELD_FLOAT(m_jumpSpeed);
    FIELD_STRING(m_rangedLaunchPosHelperName);
    FIELD_FLOAT(m_animMultiplierMelee);
    FIELD_FLOAT(m_damageMultiplier);
    FIELD_FLOAT(m_meleeRange);
    FIELD_FLOAT(m_meleeConeSize);
    FIELD_FLOAT(m_collisionCapsuleHeight);
    FIELD_FLOAT(m_collisionCapsuleRadius);
    FIELD_VEC3(m_visStartForwardVector);
    FIELD_VEC3(m_visStartOffset);
    FIELD_FLOAT(m_bodyScale);
    FIELD_ENUM_GEN(m_conflictSide, EConflictSide);
    FIELD_BOOL(m_buff);
    FIELD_BOOL(m_powerLake);
}
END_RTTI_INIT();

EXPORT_VOID_ARG_METHOD_SCRIPT(Character, addHealth, float);
EXPORT_VOID_ARG_METHOD_SCRIPT(Character, setIsVisibleInSightQueries, bool);
EXPORT_VOID_ARG_METHOD_SCRIPT(Character, teleportTo, const mkVec3&);
EXPORT_VOID_METHOD_SCRIPT(Character, jump);
EXPORT_VOID_ARG_METHOD_SCRIPT(Character, setDirection, const mkVec3&);
EXPORT_VOID_ARG_METHOD_SCRIPT(Character, setSpeed, float);

START_SCRIPT_REGISTRATION(Character, ctx);
{
    ctx->registerFunc("AddHealth", VOID_METHOD_SCRIPT(Character, addHealth));
    ctx->registerFunc("SetIsVisibleInSightQueries", VOID_METHOD_SCRIPT(Character, setIsVisibleInSightQueries));
    ctx->registerFunc("TeleportTo", VOID_METHOD_SCRIPT(Character, teleportTo));
    ctx->registerFunc("Jump", VOID_METHOD_SCRIPT(Character, jump));
    ctx->registerFunc("SetDirection", VOID_METHOD_SCRIPT(Character, setDirection));
    ctx->registerFunc("SetSpeed", VOID_METHOD_SCRIPT(Character, setSpeed));
}
END_SCRIPT_REGISTRATION(ctx);

bool EConflictSide::areEnemies( TYPE lhs, TYPE rhs )
{
    return lhs != EConflictSide::Unknown && rhs != EConflictSide::Unknown && lhs != rhs;
}

bool EConflictSide::areAllies( TYPE lhs, TYPE rhs )
{
    return lhs != EConflictSide::Unknown && rhs != EConflictSide::Unknown && lhs == rhs;
}

bool EConflictSide::areNeutral( TYPE lhs, TYPE rhs )
{
    return !areEnemies(lhs, rhs) && !areAllies(lhs, rhs);
}

EConflictSide::TYPE EConflictSide::fromString( const mkString& str )
{
#define MAKE_CASE(val) if (are_strings_equal_case_insensitive(#val, str)) { return val; }
    MAKE_CASE(Unknown);
    MAKE_CASE(RedTeam);
    MAKE_CASE(BlueTeam);
#undef  MAKE_CASE

    MK_ASSERT_MSG(false, "Unknown conflict side '%s'", str.c_str());
    return EConflictSide::Unknown;
}

mkString EConflictSide::toString( TYPE arg )
{
#define MAKE_CASE(val) case val: return #val;
    switch(arg)
    {
        MAKE_CASE(Unknown);
        MAKE_CASE(RedTeam);
        MAKE_CASE(BlueTeam);
    }
#undef MAKE_CASE

    return "";
}

const btVector3 ogre_to_bullet(const mkVec3& vec);

Character::Character()
{
    m_currentDir = mkVec3::ZERO;
    m_movementDir = mkVec3::ZERO;

    m_physicsNode = NULL;
    m_playerPhysicsController = NULL;
    m_physicsGhostObject = NULL;

    m_sensorCollisionShape = NULL;
    m_sensorCollisionObject = NULL;

    m_maxSpeed = 1.f;
    m_currentSpeed = 0.f;
    m_lastShotTime = -1.f;
    m_curShotHitProb = 1.f;

    m_canJump = true;

    m_sightDist = -1.f;
    m_smellRange = -1.f;
    m_horSightAngleRad = 0.f;
    m_horSightAngleCos = 1.f;

    m_shootingRange = 20.f;

    m_visibleInSightQueries = true;

    m_conflictSide = EConflictSide::Unknown;

    m_shootingDamage = 25.f;
    m_deathTime = -1.f;

    m_lastMeleeAttackTime = -1.f;
    m_meleeDmgDone = false;

    m_lastRangedAttackTime = -1.f;
    m_rangedProjectileLaunched = false;

    m_physicsStepHeight = 0.1f;
    m_jumpSpeed = 7.5f;

    m_castsShadows = true;

    m_animMultiplierMelee = 1.f;
    m_damageMultiplier = 1.f;
    m_meleeRange = 0.f;
    m_meleeConeSize = 45.f;

    m_backflipTime = -1.f;
    m_highJumpTime = -1.f;
    m_finishingHit = true;

    m_smoothDirChange = false;
    m_smoothStep = 0;
    m_smoothStepTime = 0.f;

    m_targetPos = mkVec3::ZERO;

    m_forcedAnimStart = -1.f;

    m_wasDead = false;
    m_buff = false;
    m_powerLake = false;

    m_animDurations["Attack3"] = 650.f;
    m_animDurations["Attack1"] = 550.f;
    m_animDurations["Backflip"] = 900.f;
    m_animDurations["HighJump"] = 1000.f;
    m_animDurations["Jump"] = 900.f;
    m_collisionCapsuleHeight = 1.f;
    m_collisionCapsuleRadius = 0.5f;

    m_visStartForwardVector = mkVec3::UNIT_Z;
    m_visStartOffset = mkVec3::ZERO;

    m_bodyScale = 1.f;
}

void Character::jump()
{
    if (canJump())
        m_playerPhysicsController->jump();
}

void Character::setDirection( mkVec3 dir )
{
    m_currentDir = dir;
    m_currentDir.y = 0;
}

void Character::setSpeed( float value )
{
    m_currentSpeed = clamp(value, 0.f, 1.f) * m_maxSpeed;
}

float Character::getRealSpeed() const
{
    return m_currentSpeed;
}

void Character::setMaxSpeed( float val )
{
    m_maxSpeed = val;
}

float Character::getMaxSpeed() const
{
    return m_maxSpeed;
}

mkVec3 Character::getSimPos() const
{
    btVector3 controller_location = m_physicsGhostObject->getWorldTransform().getOrigin();
    return mkVec3(controller_location.x(), controller_location.y(), controller_location.z());
}

void Character::onRender()
{
    __super::onRender();

    onDebugDraw();
    updateVisAnimation(getTimeDelta());
}

void Character::updateVisAnimation(float dt)
{
    if (!m_visAnim.isReady())
        return;

    if (isDead())
    {
        if (m_visAnim.isPlayingAnimation())
            m_visAnim.stopAnimation();

        return;
    }

    float wanted_dt = dt;

    static float MELEE_ATTACK_DURATION = 650.f;
    float melee_attack_duration = MELEE_ATTACK_DURATION / m_animMultiplierMelee;
    static float RANGED_ATTACK_DURATION = 550.f;
    static float BACKFLIP_DURATION = 900.f;
    static float HIGHJUMP_DURATION = 1000.f;
    if(m_visAnim.hasAnimationClip(m_forcedAnim.c_str()) && (getTimeMs() - m_forcedAnimStart) < m_forcedAnimDuration){
        m_visAnim.setAnimationClipPlayed(m_forcedAnim.c_str());
        wanted_dt = dt * m_forcedDt;
    }
    else if (m_lastMeleeAttackTime > 0.f && (getTimeMs() - m_lastMeleeAttackTime) < melee_attack_duration && m_visAnim.hasAnimationClip("Attack3"))
    {
        m_visAnim.setAnimationClipPlayed("Attack3");
        wanted_dt = dt * m_animMultiplierMelee;
    }
    else if (m_lastRangedAttackTime > 0.f && (getTimeMs() - m_lastRangedAttackTime) < RANGED_ATTACK_DURATION && m_visAnim.hasAnimationClip("Attack1"))
    {
        m_visAnim.setAnimationClipPlayed("Attack1");
    }
    else if (m_backflipTime > 0.f && m_visAnim.hasAnimationClip("Backflip") && (getTimeMs() - m_backflipTime) < BACKFLIP_DURATION){
        m_visAnim.setAnimationClipPlayed("Backflip");
    }
    else if (m_highJumpTime > 0.f && m_visAnim.hasAnimationClip("HighJump") && (getTimeMs() - m_highJumpTime) < HIGHJUMP_DURATION){
        m_visAnim.setAnimationClipPlayed("HighJump");
    }
    else if (m_currentSpeed > 0.001f)
    {
        if (m_visAnim.hasAnimationClip("Run"))
            m_visAnim.setAnimationClipPlayed("Run");
        else if (m_visAnim.hasAnimationClip("Walk"))
            m_visAnim.setAnimationClipPlayed("Walk");

        wanted_dt *= getWalkAnimSpeed();
    }
    else if (m_visAnim.hasAnimationClip("Idle"))
    {
        m_visAnim.setAnimationClipPlayed("Idle");
    }
    else if (m_visAnim.hasAnimationClip("Idle1"))
        m_visAnim.setAnimationClipPlayed("Idle1");

    m_visAnim.update(wanted_dt);
    m_visAnim.setLooped(true);
}

void Character::updateComponents()
{
    if (m_currentSpeed > 0.001f && !isDead())
    {
        //btVector3 movement_vec = ogre_to_bullet(m_currentDir);
        btVector3 movement_vec;
        if(m_movementDir == mkVec3::ZERO)
            movement_vec = ogre_to_bullet(m_currentDir);
        else
            movement_vec = ogre_to_bullet(m_movementDir);
        movement_vec.safeNormalize();
        movement_vec = movement_vec * m_currentSpeed; // speed in m/s (no mul by time delta, physics engine handles that)
        m_playerPhysicsController->setWalkDirection(movement_vec);

        const mkQuat wanted_orientation = m_visStartForwardVector.getRotationTo(m_currentDir);
        setOrientation(wanted_orientation);
    }
    else if(m_currentSpeed == 0.f && m_smoothDirChange && !isDead()){
        const mkQuat wanted_orientation = m_visStartForwardVector.getRotationTo(m_currentDir);
        setOrientation(wanted_orientation);

        m_playerPhysicsController->setWalkDirection(btVector3(0, 0, 0));
    }
    else
    {
        m_playerPhysicsController->setWalkDirection(btVector3(0, 0, 0));
    }

    if(m_wasDead && !isDead()){
        setOrientation(m_visStartForwardVector.getRotationTo(mkVec3::UNIT_Z));
        m_wasDead = false;
    }

    if (isDead()){
        setOrientation(m_visStartForwardVector.getRotationTo(mkVec3::UNIT_Y));
        m_wasDead = true;
    }
}

void Character::resetDirection()
{
    m_currentDir = mkVec3(0, 0, 0);
}

void Character::addDirection( mkVec3 dir )
{
    m_currentDir += dir;
    m_currentDir.y = 0;
}

btKinematicCharacterController* Character::getKinematicController() const
{
    return m_playerPhysicsController;
}

// Not working for custom collision algorithms!
template <typename CollisionHandler>
uint32 handleGhostObjectCollisions(btPairCachingGhostObject* ghost_object, CollisionHandler collision_handler)
{
    btManifoldArray manifold_array;
    btBroadphasePairArray& pair_array = ghost_object->getOverlappingPairCache()->getOverlappingPairArray();
    const int num_pairs = pair_array.size();
    btOverlappingPairCache* pair_cache = g_game->getPhysicsWorld()->getPairCache();

    int num_contacts = 0;

    for (int i = 0; i < num_pairs; i++)
    {
        manifold_array.clear();

        const btBroadphasePair& pair = pair_array[i];

        btBroadphasePair* collision_pair = pair_cache->findPair(pair.m_pProxy0,pair.m_pProxy1);
        if (!collision_pair)
            continue;

        if (collision_pair->m_algorithm)
            collision_pair->m_algorithm->getAllContactManifolds(manifold_array);

        for (int j = 0; j < manifold_array.size(); j++)
        {
            btPersistentManifold* manifold = manifold_array[j];
            void* body0 = manifold->getBody0();
            void* body1 = manifold->getBody1();

            if (body0 == ghost_object)
                collision_handler(ghost_object, (btCollisionObject*)body1);
            else if (body1 == ghost_object)
                collision_handler(ghost_object, (btCollisionObject*)body0);
            else
                MK_ASSERT(false);

            ++num_contacts;
        }
    }

    return num_contacts;
}

struct CharacterContactHandler
{
    void operator()(btGhostObject* ghost_obj, btCollisionObject* colliding_obj) const
    {
        PhysicsObjUserData* my_data = (PhysicsObjUserData*)ghost_obj->getUserPointer();
        PhysicsObjUserData* other_data = (PhysicsObjUserData*)colliding_obj->getUserPointer();

        if (other_data != 0 && (other_data->type & PhysicsObjUserData::EnemyObject)
            && my_data != 0 && (my_data->type & PhysicsObjUserData::PlayerObject))
        {
        }
    }
};

void Character::handleContacts()
{
    handleGhostObjectCollisions(m_physicsGhostObject, CharacterContactHandler());
}

void Character::updateSensorObject()
{
    if (!m_sensorCollisionObject)
        return;

    // No need to update sensor when character is not moving
    if (m_currentSpeed < 0.001f)
        return;

    const btTransform& char_transform = m_physicsGhostObject->getWorldTransform();
    btTransform sensor_offset;
    sensor_offset.setIdentity();
    sensor_offset.setOrigin(char_transform.getOrigin() + ogre_to_bullet(m_currentDir.normalisedCopy() * m_sensorCollisionShape->getRadius()) + btVector3(0, 2.25, 0));

    m_sensorCollisionObject->setWorldTransform(sensor_offset);
}

struct SensorCollisionHarvester
{
    SensorCollisionContainer* output;

    void operator()(btGhostObject* ghost_obj, btCollisionObject* colliding_obj) const
    {
        PhysicsObjUserData* my_data = (PhysicsObjUserData*)ghost_obj->getUserPointer();
        PhysicsObjUserData* other_data = (PhysicsObjUserData*)colliding_obj->getUserPointer();

        if (colliding_obj->getCollisionFlags() & btCollisionObject::CF_STATIC_OBJECT)
        {
            SensorCollision cl;
            output->push_back(cl);
        }
    }
};

void Character::gatherSensorContacts()
{
    if (!m_sensorCollisionObject)
        return;

    SensorCollisionHarvester handler;
    handler.output = &m_sensorCollisions;
    m_sensorCollisions.clear();

    handleGhostObjectCollisions(m_sensorCollisionObject, handler);
}

RayCastResult Character::raycast(const mkVec3& dir, float height, const float ray_len) const
{
    height = clamp(height, 0.f, 1.f);

    const btVector3 start_point = ogre_to_bullet(getPosForHeight(height));
    const btVector3 end_point = ogre_to_bullet(bullet_to_ogre(start_point) + dir.normalisedCopy() * ray_len);

    btCollisionWorld::ClosestRayResultCallback ray_callback(start_point, end_point);
    g_game->getPhysicsWorld()->rayTest(start_point, end_point, ray_callback);

    RayCastResult result;
    if (ray_callback.hasHit())
    {
        result.hit = true;
        result.collision_type = RayCastResult::Environment;

        btCollisionObject* object = ray_callback.m_collisionObject;
        PhysicsObjUserData* user_data = (PhysicsObjUserData*)object->getUserPointer();

        if (user_data != 0 && (user_data->type & PhysicsObjUserData::CharacterObject))
            result.collision_type = RayCastResult::Actor;

        result.point = bullet_to_ogre(ray_callback.m_hitPointWorld);
    }
    else
        result.hit = false;

    return result;
}

void Character::shoot()
{
    static float kShotRangeMeters = 200.0f;

    const float hit_prob = getCurrentShotHitProb();
    if (randFloat() < hit_prob)
        shoot(getWorldPosition(), m_currentDir, kShotRangeMeters);

    m_lastShotTime = g_game->getTimeMs();
    updateCurrentShotHitProb();
}

void Character::shoot( const mkVec3& origin, const mkVec3& dir, float max_range )
{
    const btVector3 start_pos = ogre_to_bullet(origin);
    const btVector3 forward_vec = ogre_to_bullet(dir);
    const btVector3 ray_end = start_pos + forward_vec.normalized() * max_range;

    btCollisionWorld::ClosestRayResultCallback ray_callback(start_pos, ray_end);
    g_game->getPhysicsWorld()->rayTest(start_pos, ray_end, ray_callback);

    if (ray_callback.hasHit())
    {
        btCollisionObject* object = ray_callback.m_collisionObject;
        PhysicsObjUserData* user_data = (PhysicsObjUserData*)object->getUserPointer();

        if (user_data != 0)
        {
            if (user_data->type & PhysicsObjUserData::CharacterObject)
            {
                SDamageInfo dmg_info(EDamageType::Bullet, m_shootingDamage, bullet_to_ogre(forward_vec), bullet_to_ogre(ray_callback.m_hitPointWorld));
                user_data->character->takeDamage(dmg_info);
            }

            if (user_data->type & PhysicsObjUserData::DynGameObject)
            {
                mkVec3 dir = bullet_to_ogre(-ray_callback.m_hitNormalWorld);
                mkVec3 pos = bullet_to_ogre(ray_callback.m_hitPointWorld);

                SDamageInfo dmg_info(EDamageType::Bullet, m_shootingDamage, dir, pos);
                user_data->game_obj->takeDamage(dmg_info);
            }
        }
    }
}

void Character::onDie()
{

}

void Character::onUpdate()
{
    __super::onUpdate();

    if(m_smoothDirChange)
        smoothChangeDirection();
    updateFinishingAttack();
    updateMeleeAttack();
    updateRangedAttack();
    updateRespawn();
    updateCurrentShotHitProb();
    updateComponents();

    setWorldPosition(getPosForVisualsNode());

    handleContacts();
    updateSensorObject();
    gatherSensorContacts();
}

void Character::onCreate()
{
    __super::onCreate();

    Ogre::SceneManager* smgr = g_game->getOgreSceneMgr();

    // HACK - spawn character slightly higher than requested with world pos,
    // because (especially after deserialization) when spawned exactly on ground
    // level, collision with floor was not detected and Character fell into abyss :(
    mkVec3 start_world_pos = getWorldPosition() + mkVec3(0, 0.5f, 0);

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(ogre_to_bullet(start_world_pos));

    m_physicsGhostObject = new btPairCachingGhostObject();
    m_physicsGhostObject->setWorldTransform(transform);

    btConvexShape* capsule = new btCapsuleShape(m_collisionCapsuleRadius, m_collisionCapsuleHeight);

    m_physicsGhostObject->setCollisionShape(capsule);
    m_physicsGhostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

    m_playerPhysicsController = new btKinematicCharacterController(m_physicsGhostObject, capsule, m_physicsStepHeight);
    m_playerPhysicsController->setJumpSpeed(m_jumpSpeed);

    btDynamicsWorld* dyn_world = g_game->getPhysicsWorld();
    dyn_world->addCollisionObject(m_physicsGhostObject, btBroadphaseProxy::CharacterFilter,
        btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter | btBroadphaseProxy::CharacterFilter);
    dyn_world->addAction(m_playerPhysicsController);

    btVector3 aabb_min, aabb_max;
    capsule->getAabb(transform, aabb_min, aabb_max);
    //m_size = bullet_to_ogre(aabb_max - aabb_min);
    //HACK
    m_size = mkVec3(.9f, 2.f, .9f);

    setHorizontalSightAngle(m_horSightAngleRad);

    if (getVisSceneNode())
        getVisSceneNode()->setScale(m_bodyScale, m_bodyScale, m_bodyScale);

    m_visAnim.setMesh(getVisMesh());
    //m_visAnim.setDrawElementBoxes(true);
}

void Character::onDestroy()
{
    __super::onDestroy();

    getLevel()->getPhysicsWorld()->removeCollisionObject(m_physicsGhostObject);
    getLevel()->getPhysicsWorld()->removeAction(m_playerPhysicsController);
}

void Character::onDebugDraw()
{
    m_visAnim.debugDraw();

    float VERT_OFFSET = getBodyScale() * 100.f;
    mkVec3 origin = getSimPos() + mkVec3::UNIT_Y * VERT_OFFSET;

    static float WIDTH = 1.f;
    static float HEIGHT = 0.1f;

    float part_health;
    if (getHealth() <= 0.f)
        part_health = 0.f;
    else
        part_health = getHealth() / getMaxHealth();

    mkVec3 cam_dir = g_game->getCamera()->getForwardVec();
    mkVec3 right = g_game->getCamera()->getRightVec();
    mkVec3 up = cam_dir.crossProduct(right);

    mkVec3 left_side = origin - right * WIDTH * 0.5f;

    mkVec3 left_top_corner = left_side + up * HEIGHT * 0.5f;
    mkVec3 left_bottom_corner = left_side - up * HEIGHT * 0.5f;

    mkVec3 health_right_side = left_side + right * WIDTH * part_health;

    mkVec3 right_top_corner = health_right_side + up * HEIGHT * 0.5f;
    mkVec3 right_bottom_corner = health_right_side - up * HEIGHT * 0.5f;

    {
        mkVec3 vertices[] = {left_top_corner, left_bottom_corner, right_bottom_corner, right_top_corner};
        DebugDrawer::getSingleton().drawQuad(vertices, Ogre::ColourValue::Red, true);
    }

    left_top_corner = right_top_corner;
    left_bottom_corner = right_bottom_corner;
    right_top_corner = left_side + right * WIDTH + up * HEIGHT * 0.5f;
    right_bottom_corner = left_side + right * WIDTH - up * HEIGHT * 0.5f;

    {
        mkVec3 vertices[] = {left_top_corner, left_bottom_corner, right_bottom_corner, right_top_corner};
        DebugDrawer::getSingleton().drawQuad(vertices, Ogre::ColourValue(0.1f, 0.1f, 0.1f), true);
    }

    //DebugDrawer::getSingleton().drawSphere(getPosForRangedAttackStart(), 0.1f, Ogre::ColourValue::Blue);
    //DebugDrawer::getSingleton().drawSphere(getPosForRangedAttackStart() + getDirForRangedAttack(), 1.1f, Ogre::ColourValue::Green);
}

void Character::drawSightArea()
{
    const Ogre::ColourValue color(0.1f, 0.1f, 1.f, 0.1f);
    mkVec3 simPos = getSimPos();
    simPos.y = Ogre::Real(0.1);
    DebugDrawer::getSingleton().drawCircleSector(simPos, m_sightDist, m_horSightAngleRad * 2.f, getSimDir(), 15, color, true, true);
    // Shooting range
    DebugDrawer::getSingleton().drawCircle(simPos, getShootingRange(), 30, Ogre::ColourValue(1.f, 0.f, 0.f, getCurrentShotHitProb()));
}

mkVec3 Character::getSimDir() const
{
    return m_currentDir;
}

bool Character::canJump() const
{
    return m_canJump && m_playerPhysicsController->canJump();
}

int Character::getCharactersInSight(CharacterVec& out_characters) const
{
    int visible_characters = 0;

    TGameObjectVec all_characters;
    getLevel()->findObjectsOfType(all_characters, &Character::Type);
    for (int i = 0; i < (int)all_characters.size(); ++i)
    {
        Character* other_character = static_cast<Character*>(all_characters[i]);
        if (other_character == this)
            continue;

        if (!other_character->isVisibleInSightQueries())
            continue;

        if (isCharacterVisible(other_character))
        {
            out_characters.push_back(other_character);
            ++visible_characters;
        }
    }

    return visible_characters;
}

int Character::getCharactersBySmell( CharacterVec& out_characters ) const
{
    int visible_characters = 0;

    TGameObjectVec all_characters;
    getLevel()->findObjectsOfType(all_characters, &Character::Type);
    for (int i = 0; i < (int)all_characters.size(); ++i)
    {
        Character* other_character = static_cast<Character*>(all_characters[i]);
        if (other_character == this)
            continue;

        if (!other_character->isVisibleInSightQueries())
            continue;

        if (isCharacterScented(other_character))
        {
            out_characters.push_back(other_character);
            ++visible_characters;
        }
    }

    return visible_characters;
}


bool Character::isCharacterVisible( const Character* other_character ) const
{
    const mkVec3 size = other_character->getSize();
    const float pos_visibility_radius = 0.f;//std::max(size.x, size.z)
    const mkVec3 sim_pos = other_character->getSimPos();

    return isPositionVisible(sim_pos, pos_visibility_radius);
}

bool Character::isCharacterScented( const Character* other ) const
{
    const mkVec3 size = other->getSize();
    const float pos_visibility_radius = 0.f;//std::max(size.x, size.z)
    const mkVec3 sim_pos = other->getSimPos();

    return isPositionScented(sim_pos, pos_visibility_radius);
}


bool Character::isPositionVisible(const mkVec3& pos, float radius) const
{
    const mkVec3 my_pos = getSimPos();
    const mkVec3 pos_2d = mkVec3(pos.x, my_pos.y, pos.z);
    const float dist2 = my_pos.squaredDistance(pos_2d);
    if (dist2 > m_sightDist * m_sightDist)
        return false;

    const mkVec3 my_dir = getSimDir().normalisedCopy();
    const mkVec3 dir_to_pos = (pos_2d - my_pos).normalisedCopy();
    const float dot = my_dir.dotProduct(dir_to_pos);

    if (dot < m_horSightAngleCos)
        return false;

    RayCastResult ray_result = raycast(dir_to_pos, 1.f, Ogre::Math::Sqrt(dist2));
    if (ray_result.hit && ray_result.collision_type == RayCastResult::Environment)
        return false;

    return true;
}

bool Character::isPositionScented( const mkVec3& pos, float radius /*= 0.f*/ ) const
{
    const mkVec3 my_pos = getSimPos();
    const mkVec3 pos_2d = mkVec3(pos.x, my_pos.y, pos.z);
    const float dist2 = my_pos.squaredDistance(pos_2d);
    if (dist2 > m_smellRange * m_smellRange)
        return false;

    const mkVec3 dir_to_pos = (pos_2d - my_pos).normalisedCopy();
    RayCastResult ray_result = raycast(dir_to_pos, 1.f, Ogre::Math::Sqrt(dist2));
    if (ray_result.hit && ray_result.collision_type == RayCastResult::Environment)
        return false;

    return true;
}


mkVec3 Character::getSize() const
{
    return m_size; // todo change vertical size when crouching
}

void Character::setHorizontalSightAngle(const float angle_rad)
{
    m_horSightAngleRad = angle_rad;
    m_horSightAngleCos = Ogre::Math::Cos(angle_rad);
}

void Character::setSightDistance(const float dist)
{
    m_sightDist = dist;
}

mkVec3 Character::getEyePos() const
{
    return getPosForHeight(1.f);
}

mkVec3 Character::getPosForHeight(float height) const
{
    if (getVisMesh())
    {
        Ogre::AxisAlignedBox aabb = getVisMesh()->getWorldBoundingBox();
        mkVec3 low_point = aabb.getCenter();
        mkVec3 high_point = aabb.getCenter();
        low_point.y = aabb.getMinimum().y;
        high_point.y = aabb.getMaximum().y;

        return low_point + (high_point - low_point) * height;
    }

    return getSimPos();
}

mkVec3 Character::getPosForVisualsNode() const
{
    mkVec3 low_point = getSimPos();
    mkVec3 high_point = low_point;
    high_point.y += m_size.y * .5f; // HACK for player camera

    return high_point + m_visStartForwardVector;
}

bool Character::isEnemy( const Character* other ) const
{
    return EConflictSide::areEnemies(getConflictSide(), other->getConflictSide());
}

bool Character::isAlly( const Character* other ) const
{
    return EConflictSide::areAllies(getConflictSide(), other->getConflictSide());
}

void Character::lookAt( const mkVec3& target_pos )
{
    setDirection((target_pos - getSimPos()).normalisedCopy());
}

bool Character::isDead() const
{
    return false;
}

Level* Character::getLevel() const
{
    return g_game->getCurrentLevel();
}

int Character::getCharactersFromSameTeam( CharacterVec& out_characters ) const
{
    TGameObjectVec all_characters;
    getLevel()->findObjectsOfType(all_characters, &Character::Type);
    int result = 0;
    for (size_t i = 0; i < all_characters.size(); ++i)
    {
        Character* character = static_cast<Character*>(all_characters[i]);
        if (character->getConflictSide() == getConflictSide())
        {
            out_characters.push_back(character);
            ++result;
        }
    }

    return result;
}

void Character::updateCurrentShotHitProb()
{
    if (m_lastShotTime > 0.f)
    {
        const float cur_time = g_game->getTimeMs();
        const float time_from_last_shot = cur_time - m_lastShotTime;
        const float rest_time = 2000.f;

        m_curShotHitProb = clamp(time_from_last_shot / rest_time, 0.f, 1.f);
    }
}

bool Character::isPosInShootingRange( const mkVec3& pos, float use_range_part ) const
{
    const float dist = getSimPos().distance(pos);
    const float range = getShootingRange() * use_range_part;

    return dist < range;
}

void Character::updateRespawn()
{
    if (!isDead())
        m_deathTime = -1.f;
    else if (m_deathTime == -1.f)
    {
        m_deathTime = g_game->getTimeMs();

        teleportTo(getRespawnPos());
    }
    else
    {
        const float time_from_death = g_game->getTimeMs() - m_deathTime;
        const float respawn_time = 5000.f;

        if (time_from_death > respawn_time)
            revive();
    }
}

void Character::teleportTo( const mkVec3& pos )
{
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(ogre_to_bullet(pos));
    m_physicsGhostObject->setWorldTransform(transform);

    setWorldPosition(getPosForVisualsNode());
}

mkVec3 Character::getRespawnPos() const
{
    return getWorldPosition();
}

float Character::getWalkAnimSpeed() const
{
    static float BASE_SPEED = 3.f; // 0.01 for robot.mesh "Walk" anim
    float mul = m_currentSpeed / (BASE_SPEED * getBodyScale());

    return mul;
}

void Character::startMeleeAttack()
{
    m_lastMeleeAttackTime = getTimeMs();
    m_meleeDmgDone = false;
}

void Character::updateMeleeAttack()
{
    float dmg_radius = m_meleeRange+1.f;
    Ogre::Degree dmgSizeDgr(m_meleeConeSize);
    Ogre::Radian dmgAngle(dmgSizeDgr);
    static float DMG_TIME = 490.f;
    float dmg_time = DMG_TIME / m_animMultiplierMelee;
    if (!m_meleeDmgDone && m_lastMeleeAttackTime > 0.f && (getTimeMs() - m_lastMeleeAttackTime) >= dmg_time)
    {
        TGameObjectVec objects;

        getLevel()->findObjectsInRadius(objects, &ModelObject::Type, getSimPos(), dmg_radius);

        for (size_t i = 0; i < objects.size(); ++i)
        {
            ModelObject* mo = static_cast<ModelObject*>(objects[i]);
            if (mo != this)
            {
                mkVec3 mydir = getMeleeForwardDir();
                mydir.y = 0.f;
                mydir.normalise();

                mkVec3 dir_to_enemy = mo->getWorldPosition() - getSimPos();
                dir_to_enemy.y = 0.f;
                dir_to_enemy.normalise();

                Ogre::Radian angle = mydir.angleBetween(dir_to_enemy);

                if (angle <= dmgAngle)
                {
                    SDamageInfo dmg_info(EDamageType::Punch, getMeleeDmg(), mydir, mo->getWorldPosition());
                    mo->takeDamage(dmg_info);
                }
            }
        }

        m_meleeDmgDone = true;
    }
}

float Character::getMeleeDmg() const
{
    static float DMG = 10.f;
    float dmg = DMG * m_damageMultiplier;
    return dmg;
}

mkVec3 Character::getMeleeForwardDir() const
{
    return getSimDir();
}

float Character::getHealth() const
{
    return 100.f;
}

float Character::getMaxHealth() const
{
    return 100.f;
}

void Character::startRangedAttack(const mkVec3& targetPos)
{
    m_lastRangedAttackTime = getTimeMs();
    m_rangedProjectileLaunched = false;
    m_targetPos = targetPos;
}

void Character::updateRangedAttack()
{
    static float RANGED_ATTACK_LAUNCH_TIME = 350.f;
    if (m_lastRangedAttackTime > 0.f && !m_rangedProjectileLaunched && (getTimeMs() - m_lastRangedAttackTime) > RANGED_ATTACK_LAUNCH_TIME)
    {
        Rocket* rocket = getLevel()->createObject<Rocket>(true, "Default");        

        rocket->setDirection(getDirForRangedAttack());
        rocket->setWorldPosition(getPosForRangedAttackStart());
        rocket->setIgnoredObject(this);

        m_rangedProjectileLaunched = true;

        m_targetPos = mkVec3::ZERO;
    }
}

mkVec3 Character::getPosForRangedAttackStart() const
{
    if (m_rangedLaunchPosHelperName.empty())
        return getEyePos();
    else
        return getAnimContext().getMeshElementWorldPos(m_rangedLaunchPosHelperName.c_str());
}

mkVec3 Character::getDirForRangedAttack() const
{
    if(m_targetPos != mkVec3::ZERO){
        mkVec3 targetPos = m_targetPos;
        targetPos.y = 0.5f;
        return (targetPos-getPosForRangedAttackStart()).normalisedCopy();
    }
    else 
        return getSimDir();
}

void Character::startBackflip(){
    m_backflipTime = getTimeMs();
}

void Character::startHighJump(){
    m_highJumpTime = getTimeMs();
}

void Character::startFinishingAttack(){
    m_highJumpTime = getTimeMs();
    m_finishingHit = false;
}

void Character::updateFinishingAttack()
{
    float dmg_radius = m_meleeRange+0.5f;
    Ogre::Degree dmgSizeDgr(m_meleeConeSize);
    Ogre::Radian dmgAngle(dmgSizeDgr);
    static float DMG_TIME = 995.f;
    if (!m_finishingHit && m_highJumpTime > 0.f && (getTimeMs() - m_highJumpTime) >= DMG_TIME)
    {
        TGameObjectVec objects;

        getLevel()->findObjectsInRadius(objects, &ModelObject::Type, getSimPos(), dmg_radius+0.5f);

        for (size_t i = 0; i < objects.size(); ++i)
        {
            ModelObject* mo = static_cast<ModelObject*>(objects[i]);
            if (mo != this)
            {
                mkVec3 mydir = getMeleeForwardDir();
                mydir.y = 0.f;
                mydir.normalise();

                mkVec3 dir_to_enemy = mo->getWorldPosition() - getSimPos();
                dir_to_enemy.y = 0.f;
                dir_to_enemy.normalise();

                Ogre::Radian angle = mydir.angleBetween(dir_to_enemy);

                if (angle <= dmgAngle)
                {
                    SDamageInfo dmg_info(EDamageType::Punch, getMeleeDmg()*4.f, mydir, mo->getWorldPosition());
                    mo->takeDamage(dmg_info);
                }
            }
        }

        m_finishingHit = true;
    }
}

void Character::startSmoothChangeDir(mkVec3 destinationDir, unsigned int stepCount, float taskDuration){
    if(!m_smoothDirChange){
        m_smoothDirChange = true;
        m_smoothDestDir = destinationDir;
        m_smoothStepCount = stepCount;
        m_smoothStepDuration = (taskDuration-100)/stepCount;
        m_smoothStep = 0;
        m_smoothStepTime = 0.f;
    }
}

void Character::stopSmoothChangeDir(){
    m_smoothDirChange = false;
    m_smoothStep = 0;
    m_smoothStepTime = 0.f;
    m_smoothStepCount = 0;
}

void Character::smoothChangeDirection() {
    if(m_smoothStep == 0){
        Ogre::Radian angle = getSimDir().angleBetween(m_smoothDestDir);
        m_smoothStepAngle = angle / (float)m_smoothStepCount;
        ++m_smoothStep;
    } else if(m_smoothStep == m_smoothStepCount){
        stopSmoothChangeDir();
    } else {
        smoothRotate();
    }
}

void Character::smoothRotate(){
    float currTime = g_game->getTimeMs();
    if(currTime - m_smoothStepTime >= m_smoothStepDuration){
        mkVec3 destVect;

        mkVec3 xProd = getSimDir().crossProduct(m_smoothDestDir);
        xProd.normalise();
        bool smoothRotateDirection = xProd.y > 0.f ? false : true;

        if(smoothRotateDirection){
            destVect = Ogre::Quaternion(-m_smoothStepAngle, Ogre::Vector3::UNIT_Y) * getSimDir();
        } else {
            destVect = Ogre::Quaternion(m_smoothStepAngle, Ogre::Vector3::UNIT_Y) * getSimDir();
        }
        destVect.normalise();

        setDirection(destVect);
        m_smoothStepTime = g_game->getTimeMs();
        ++m_smoothStep;
    }
}

Ogre::Radian Character::getCharToEnemyAngle(const mkVec3& targetPos) const
{
    mkVec3 mydir = getMeleeForwardDir();
    mydir.y = 0.f;
    mydir.normalise();

    mkVec3 dir_to_enemy = targetPos - getSimPos();
    dir_to_enemy.y = 0.f;
    dir_to_enemy.normalise();

    return mydir.angleBetween(dir_to_enemy);
}

void Character::runAnimation( const mkString& animName, float duration )
{
    if(duration > 0.f){
        m_forcedAnim = animName;
        m_forcedAnimDuration = duration;
        m_forcedDt = m_animDurations[animName] / duration;
        m_forcedAnimStart = getTimeMs();
    }
}

void Character::runAnimation( const mkString& animName, float duration, float animDuration )
{
    if(duration > 0.f && animDuration > 0.f){
        m_forcedAnim = animName;
        m_forcedAnimDuration = duration;
        m_forcedDt = animDuration / duration;
        m_forcedAnimStart = getTimeMs();
    }
}

void Character::hitMelee()
{
    float dmg_radius = m_meleeRange+1.f;
    Ogre::Degree dmgSizeDgr(m_meleeConeSize);
    Ogre::Radian dmgAngle(dmgSizeDgr);

    TGameObjectVec objects;
    getLevel()->findObjectsInRadius(objects, &ModelObject::Type, getSimPos(), dmg_radius);

    for (size_t i = 0; i < objects.size(); ++i)
    {
        ModelObject* mo = static_cast<ModelObject*>(objects[i]);
        if (mo != this)
        {
            mkVec3 mydir = getMeleeForwardDir();
            mydir.y = 0.f;
            mydir.normalise();

            mkVec3 dir_to_enemy = mo->getWorldPosition() - getSimPos();
            dir_to_enemy.y = 0.f;
            dir_to_enemy.normalise();

            Ogre::Radian angle = mydir.angleBetween(dir_to_enemy);

            if (angle <= dmgAngle)
            {
                SDamageInfo dmg_info(EDamageType::Punch, getMeleeDmg(), mydir, mo->getWorldPosition());
                mo->takeDamage(dmg_info);
            }
        }
    }
}

void Character::hitFireball( const mkVec3& targetPos )
{
    m_targetPos = targetPos;
    Rocket* rocket = getLevel()->createObject<Rocket>(true, "Default");        

    rocket->setDirection(getDirForRangedAttack());
    rocket->setWorldPosition(getPosForRangedAttackStart());
    rocket->setIgnoredObject(this);

    m_rangedProjectileLaunched = true;

    m_targetPos = mkVec3::ZERO;
}

void Character::hitAngerMode()
{
    float dmg_radius = m_meleeRange+1.f;
    Ogre::Degree dmgSizeDgr(m_meleeConeSize);
    Ogre::Radian dmgAngle(dmgSizeDgr);

    TGameObjectVec objects;

    getLevel()->findObjectsInRadius(objects, &ModelObject::Type, getSimPos(), dmg_radius);

    for (size_t i = 0; i < objects.size(); ++i)
    {
        ModelObject* mo = static_cast<ModelObject*>(objects[i]);
        if (mo != this)
        {
            mkVec3 mydir = getMeleeForwardDir();
            mydir.y = 0.f;
            mydir.normalise();

            mkVec3 dir_to_enemy = mo->getWorldPosition() - getSimPos();
            dir_to_enemy.y = 0.f;
            dir_to_enemy.normalise();

            Ogre::Radian angle = mydir.angleBetween(dir_to_enemy);

            if (angle <= dmgAngle)
            {
                SDamageInfo dmg_info(EDamageType::Punch, getMeleeDmg()*4.f, mydir, mo->getWorldPosition());
                mo->takeDamage(dmg_info);
            }
        }
    }

}

float Character::getBodyScale() const
{
    return m_bodyScale;
}

bool Character::isObjectAvailable( std::string objectName ) const
{
    Level* level = getLevel();
    if(level->findObjectByName(objectName))
        return true;

    return false;
}

mkVec3 Character::getObjectPosition( std::string objectName ) const
{
    ModelObject *model = dynamic_cast<ModelObject *>(getLevel()->findObjectByName(objectName));
    if (NULL != model)
    {
        return model->getWorldPosition();
    }

    return mkVec3::ZERO;
}