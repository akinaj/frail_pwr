/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "IActorController.h"
#include "utils.h"

class ActorControllerFactory
{
public:
    IActorController* create(const mkString& controller_id, ActorAI* ai);
    void release(IActorController* controller);
    void releaseAll();

private:
    std::vector<IActorController*> m_createdControllers;
};
