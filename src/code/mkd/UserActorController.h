/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "IActorController.h"
#include "utils.h"

class UserActorController : public IActorController
{
public:
    explicit UserActorController(ActorAI* ai);

    virtual void onCreate() { }
    virtual void onTakeDamage(const SDamageInfo& dmg_info) { }
    virtual void onUpdate(float dt);
    virtual void onDebugDraw();

    virtual void onDbgKeyDown(OIS::KeyCode key);

private:
    bool m_inputPending;
    bool m_shift;

    static const int DIR_COUNT = 4;
    static OIS::KeyCode s_inputCodes [DIR_COUNT];
    static mkVec3        s_dirValues  [DIR_COUNT];

    bool m_hadInput[DIR_COUNT];

    void processInput();
    void updateDirectionAndSpeed();
    void clearInputs();
};
