/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "Character.h"
#include "ObjectRef.h"

class IActorController;
class MeshObject;
class TeamFlag;

class ActorAI : public Character
{
    DECLARE_RTTI(ActorAI);

public:
    ActorAI();

    virtual void onDie();
    virtual void onTakeDamage(const SDamageInfo& dmg_info);
    virtual void onUpdate();
    virtual void onCreate();
    virtual void onDestroy();
    virtual void onDebugDraw();

    virtual void onDbgKeyDown(OIS::KeyCode key);

    IActorController* getController() const { return m_controller; }
    void setController(IActorController* ctrlr);

    const CharacterVec& getSpottedActors() const { return m_spottedActors; }

    virtual bool isDead() const { return m_health <= 0.f; }
    void revive();

    bool pickUpFlag(EConflictSide::TYPE flag_owner);
    void dropFlag();
    bool isCarryingFlag() const;
    bool isCarryingOwnFlag() const;
    TeamFlag* getCarriedFlag() const;

    void setTeamNumber(int val) { m_teamNumber = val; }
    int getTeamNumber() const { return m_teamNumber; }

    void drawSensesInfo();

    void addHealth(float val);
    virtual float getHealth() const;
    virtual float getMaxHealth() const;

    mkString getBtTreePath() const { return m_btTreePath; }
    mkString getHtnMethodsPath() const { return m_htnMethodsPath; }
    mkString getHtnOperatorsPath() const { return m_htnOperatorsPath; }
    mkString getHtnGoalsPath() const { return m_htnGoalsPath; }

    Character* findClosestEnemyInSight() const;
protected:
    void updateSpottedActors();

    void drawMovementInfo();

    void drawSight();
    void drawTakenDamage();
    void drawSmellSense();

    virtual mkVec3 getPosForVisualsNode() const;

    virtual mkVec3 getRespawnPos() const;

private:
    mkString m_btTreePath;
    mkString m_htnMethodsPath;
    mkString m_htnOperatorsPath;
    mkString m_htnGoalsPath;

    CharacterVec m_spottedActors;
    float m_health;
    float m_maxHealth;

    float m_lastDamageTime;

    mkString m_characterCtrlName;
    IActorController* m_controller;

    ObjectRef<TeamFlag> m_carriedFlag;

    int m_teamNumber;
};
