#pragma once
#include "Character.h"
#include "Camera.h"

class Player : public Character, public ICameraTarget
{
    DECLARE_RTTI(Player);

public:
    Player();

    virtual void onPostCreate();
    virtual void onDestroy();

    virtual void onUpdate();

    virtual void shoot();

    virtual mkVec3 getPosForVisualsNode() const;

    void addHealth(float val);

    virtual mkVec3 CameraTarget_getWorldPosition() const;

    virtual void onTakeDamage(const SDamageInfo& dmg_info);

    void dbgNextAnimPreview();
    void dbgPrevAnimPreview();

    void dbgPlayAnimPreview();

    virtual void updateVisAnimation(float dt);

    virtual bool isDead() const;

    virtual float getHealth() const;
    virtual float getMaxHealth() const;
protected:
    virtual mkVec3 getMeleeForwardDir() const;
    virtual mkVec3 getDirForRangedAttack() const;

    float m_health;
    float m_maxHealth;

    mkString getDbgClipName() const;

    int m_dbgCurrAnimPreviewIdx;
    float m_dbgAnimPreviewStartTime;

    float m_timeInDeathLeft;

    bool updateDbgAnim();
    bool updateDeadAnim();
};
