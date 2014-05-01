/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "ActorControllerFactory.h"

//Other/////////////////////////////////////////////////////////////////////
#include "TestActorController.h"
#include "StateMachineActorController.h"
#include "UserActorController.h"
#include "BoidActorController.h"
//FSM///////////////////////////////////////////////////////////////////////
#include "CtfSMActorController.h"
#include "BossFSMActorController.h"
#include "SampleFSMActorController.h"
//HTN///////////////////////////////////////////////////////////////////////
#include "HTNActorController.h"
#include "SampleHTNActorController.h"
//BT////////////////////////////////////////////////////////////////////////
#include "BTActorController.h"
#include "SampleBTActorController.h"
//Experiments///////////////////////////////////////////////////////////////
#include "Experiment1BTActorController.h"
#include "Experiment1HTNActorController.h"
#include "Experiment3HTNActorController.h"
#include "Experiment3BTActorController.h"

//Pathfinding
#include "PathfindingActorController.h"

// TODO: remove this factory, make actor controllers ordinary GameObjects and use RTTI for instantiating controllers dynamically with their class name

#define CREATE_CONTROLLER(class_prefix) if (are_strings_equal_case_insensitive(controller_id, #class_prefix)) { \
                                            IActorController* result = new class_prefix##ActorController(ai);   \
                                            m_createdControllers.push_back(result);                             \
                                            result->onCreate();                                                 \
                                            return result;                                                      \
                                        }

IActorController* ActorControllerFactory::create( const mkString& controller_id, ActorAI* ai )
{
    CREATE_CONTROLLER(Test);
    CREATE_CONTROLLER(StateMachine);
    CREATE_CONTROLLER(User);
    CREATE_CONTROLLER(CtfSM);
    CREATE_CONTROLLER(Boid);
    CREATE_CONTROLLER(HTN);
    CREATE_CONTROLLER(SampleHTN);
    CREATE_CONTROLLER(BT);
    CREATE_CONTROLLER(SampleBT);
    CREATE_CONTROLLER(Experiment1HTN);
    CREATE_CONTROLLER(Experiment1BT);
    CREATE_CONTROLLER(Experiment3HTN);
    CREATE_CONTROLLER(Experiment3BT);
    CREATE_CONTROLLER(SampleFSM);
    CREATE_CONTROLLER(BossFSM);
	CREATE_CONTROLLER(Pathfinding);

    MK_ASSERT_MSG(false, "Unrecognized actor controller '%s'", controller_id.c_str());
    return NULL;
}

#undef CREATE_CONTROLLER

void ActorControllerFactory::release( IActorController* controller )
{
    MK_ASSERT(!controller->getAI());

    delete controller;
    m_createdControllers.erase(std::remove(m_createdControllers.begin(), m_createdControllers.end(), controller), m_createdControllers.end());
}

void ActorControllerFactory::releaseAll()
{
    for (size_t i = 0; i < m_createdControllers.size(); ++i)
        delete m_createdControllers[i];

    m_createdControllers.clear();
}
