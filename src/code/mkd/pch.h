/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>

#include <Ogre.h>
#include <OIS.h>

#include <btBulletDynamicsCommon.h>

#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "btBulletWorldImporter.h"

#include "mkAssert.h"

#include "settings.h"
#include "Types.h"

#include <boost\shared_ptr.hpp>
#include <boost\lexical_cast.hpp>
#include <boost\variant.hpp>
#include <boost\variant\get.hpp>
#include <boost\ptr_container\ptr_vector.hpp>
#include <pugixml.hpp>

#include "sys/stat.h"
#include "time.h"