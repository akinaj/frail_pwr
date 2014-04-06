/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "utils.h"
#include "PhysicsObjectUserData.h"
#include "ModelObject.h"
#include "AnimatedMeshContext.h"

class Level;

namespace EConflictSide
{
    enum TYPE
    {
        Unknown,
        RedTeam,
        BlueTeam,

        _COUNT,
        _FIRST = Unknown
    };

    bool areEnemies(EConflictSide::TYPE lhs, EConflictSide::TYPE rhs);
    bool areAllies(EConflictSide::TYPE lhs, EConflictSide::TYPE rhs);
    bool areNeutral(EConflictSide::TYPE lhs, EConflictSide::TYPE rhs);

    TYPE fromString(const mkString& str);
    mkString toString(TYPE arg);
}

struct SensorCollision
{
    char dupa;
};

struct RayCastResult
{
    bool hit;
    mkVec3 point;

    enum CollisionType
    {
        Actor,
        Environment
    };

    int collision_type;
};

typedef std::vector<SensorCollision> SensorCollisionContainer;
typedef std::vector<Character*> CharacterVec;

class Character : public ModelObject
{
    DECLARE_RTTI(Character);

public:
    Character();

    virtual void onCreate();
    virtual void onDestroy();
    virtual void onRender();

    virtual void shoot();

    void shoot(const mkVec3& origin, const mkVec3& dir, float max_range);

    void jump();
    void setDirection(mkVec3 dir);
    void setSpeed(float max_speed_part);
    void setMaxSpeed(float val);
    float getMaxSpeed() const;

    float getRealSpeed() const;

    btKinematicCharacterController* getKinematicController() const;

    void resetDirection();
    void addDirection(mkVec3 dir);

    mkVec3 getEyePos() const;
    mkVec3 getPosForHeight(float height) const;

    mkVec3 getSimPos() const;
    mkVec3 getSimDir() const;

    // @param height - 0..1, height for start point of ray
    RayCastResult raycast(const mkVec3& dir, float height, const float ray_len) const;

    bool canJump() const;
    void setCanJump(bool val) { m_canJump = val; }

    int getCharactersInSight(CharacterVec& out_characters) const;
    int getCharactersBySmell(CharacterVec& out_characters) const;

    // Simple 2D visibility test, ignores height
    bool isPositionVisible(const mkVec3& pos, float radius = 0.f) const;
    bool isPositionScented(const mkVec3& pos, float radius = 0.f) const;

    bool isCharacterVisible(const Character* other) const;
    bool isCharacterScented(const Character* other) const;

    mkVec3 getSize() const;

    void setHorizontalSightAngle(const float angle_rad);
    void setSightDistance(const float dist);

    bool isVisibleInSightQueries() const { return m_visibleInSightQueries; }
    void setIsVisibleInSightQueries(bool val) { m_visibleInSightQueries = val; }

    EConflictSide::TYPE getConflictSide() const { return m_conflictSide; }
    void setConflictSide(EConflictSide::TYPE arg) { m_conflictSide = arg; }

    bool isEnemy(const Character* other) const;
    bool isAlly(const Character* other) const;

    virtual void onDbgKeyDown(OIS::KeyCode key) { }

    void setShootingRange(float val) { m_shootingRange = val; }
    float getShootingRange() const { return m_shootingRange; }
    float getMeleeRange() const { return m_meleeRange; }
    float getSquaredShootingRange() const { return m_shootingRange * m_shootingRange; }

    bool isPosInShootingRange(const mkVec3& pos, float use_range_part = 1.f) const;

    void lookAt(const mkVec3& target_pos);

    virtual bool isDead() const;

    Level* getLevel() const;

    int getCharactersFromSameTeam(CharacterVec& out_characters) const;

    float getCurrentShotHitProb() const { return m_curShotHitProb; }
    void setShootingDamage(float val) { m_shootingDamage = val; }

    virtual void revive() { }
    void teleportTo(const mkVec3& pos);

    virtual void addHealth(float val) { }

    void startMeleeAttack();
    void updateMeleeAttack();

    void startRangedAttack(const mkVec3& targetPos = mkVec3::ZERO);
    void updateRangedAttack();

    float getSightDist() const { return m_sightDist; }

    void startBackflip();
    void startHighJump();

    void startFinishingAttack();
    void updateFinishingAttack();

    void startSmoothChangeDir(mkVec3 destinationDir, unsigned int stepCount, float taskDuration);
    void stopSmoothChangeDir();

    float getMeleeConeSize() const { return m_meleeConeSize; }

    Ogre::Radian getCharToEnemyAngle(const mkVec3& targetPos) const;

    float getSmellRange() const { return m_smellRange; }

    void runAnimation(const mkString& animName, float duration);
    void runAnimation(const mkString& animName, float duration, float animDuration);

    float getBodyScale() const;

    bool isObjectAvailable(std::string objectName) const;
    mkVec3 getObjectPosition(std::string objectName) const;
private:
    void smoothChangeDirection();
    void smoothRotate();

    void clear();
    void updateComponents();

    void handleContacts();

    void updateSensorObject();
    void gatherSensorContacts();

    void updateCurrentShotHitProb();
    void updateRespawn();

protected:
    virtual void updateVisAnimation(float dt);

    virtual void onDie();
    virtual void onUpdate();
    virtual void onDebugDraw();

    virtual mkVec3 getPosForVisualsNode() const;

    void drawSightArea();

    const mkVec3& getVisStartOffset() const { return m_visStartOffset; }

    virtual float getWalkAnimSpeed() const;

    virtual mkVec3 getRespawnPos() const;
    virtual float getMeleeDmg() const;
    virtual mkVec3 getMeleeForwardDir() const;

    virtual float getHealth() const;
    virtual float getMaxHealth() const;

    mkVec3 getPosForRangedAttackStart() const;
    virtual mkVec3 getDirForRangedAttack() const;

    AnimatedMeshContext& getAnimContext() { return m_visAnim; }
    const AnimatedMeshContext& getAnimContext() const { return m_visAnim; }

private:
    btRigidBody* m_physicsNode;
    btKinematicCharacterController* m_playerPhysicsController;
    btPairCachingGhostObject* m_physicsGhostObject;
    
    btSphereShape* m_sensorCollisionShape;
    btPairCachingGhostObject* m_sensorCollisionObject; // shape in front of character, used mainly by ai for obstacle avoidance etc

    mkVec3 m_size;

    mkVec3 m_currentDir;
    float m_currentSpeed;

    SensorCollisionContainer m_sensorCollisions;

    float m_lastShotTime;
    float m_curShotHitProb;
    
    float m_horSightAngleCos;
    
    float m_deathTime;

    AnimatedMeshContext m_visAnim;

    // tempshit
    float m_lastMeleeAttackTime;
    bool m_meleeDmgDone;

    float m_lastRangedAttackTime;
    bool m_rangedProjectileLaunched;
private:
    float m_backflipTime;
    float m_highJumpTime;
    bool m_finishingHit;
    bool m_visibleInSightQueries;
    //////////////////////////////////////////////////////////////////////////
    mkVec3 m_smoothDestDir;
    bool m_smoothDirChange;
    unsigned int m_smoothStep;
    unsigned int m_smoothStepCount;
    float m_smoothStepTime;
    Ogre::Radian m_smoothStepAngle;
    float m_smoothStepDuration;
    //////////////////////////////////////////////////////////////////////////
    mkString m_forcedAnim;
    float m_forcedAnimDuration;
    float m_forcedDt;
    float m_forcedAnimStart;
    std::map<mkString,float> m_animDurations;
    bool m_wasDead;
public:
    void hitMelee();
    void hitFireball(const mkVec3& targetPos);
    void hitAngerMode();
    bool hasBuff() const { return m_buff; }
    bool isInPowerLake() const { return m_powerLake; }
private:
    bool m_buff;
    bool m_powerLake;
    float m_maxSpeed;
    bool m_canJump;
    float m_sightDist;
    float m_smellRange;
    float m_horSightAngleRad;
    float m_shootingRange;
    float m_shootingDamage;
    EConflictSide::TYPE m_conflictSide;
    float m_physicsStepHeight;
    float m_jumpSpeed;
    mkString m_rangedLaunchPosHelperName;
    float m_animMultiplierMelee;
    float m_damageMultiplier;
    float m_meleeRange;
    float m_meleeConeSize;
    mkVec3 m_targetPos;
    mkVec3 m_visStartForwardVector; // forward vector of vis mesh before any transforms
    mkVec3 m_visStartOffset; // pivot offset
    float m_collisionCapsuleHeight;
    float m_collisionCapsuleRadius;
    float m_bodyScale;
};
