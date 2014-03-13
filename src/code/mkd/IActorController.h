/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "ActorAI.h"
#include "ObjectRef.h"

class IActorController
{
public:
    explicit IActorController(ActorAI* ai) : m_ai(ai) { }
    virtual ~IActorController() { }

    virtual void onCreate() = 0;
    virtual void onTakeDamage(const SDamageInfo& dmg_info) = 0;
    virtual void onUpdate(float dt) = 0;
    virtual void onDebugDraw() = 0;

    virtual void onDbgKeyDown(OIS::KeyCode key) {}
    virtual void onDie() {}

    ActorAI* getAI() const { return m_ai.fetchPtr(); }
    void setAI(ActorAI* ai) { m_ai.setPtr(ai); }

private:
    ObjectRef<ActorAI> m_ai;
};
