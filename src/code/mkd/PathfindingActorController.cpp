#include "pch.h"
#include "PathfindingActorController.h"
#include "Game.h"
#include "Contrib/OgreRecast/include/OgreRecast.h"
#include "Level.h"

#include <time.h>

unsigned PathfindingActorController::pathSlotCount = 0;


PathfindingActorController::PathfindingActorController( ActorAI* ai ) : IActorController(ai) , m_lastTurnTime(-1.f)
{

	useReactivePathFollowing = true;
	reactivePathFollowingMinTargetDistance = 1.0f;

	destReached = false;
	pathFound = false;

	pathNb = pathSlotCount;
	targetId = pathSlotCount;

	pathSlotCount++;

	stayCounterTestMode = 0;

	lastPos = getAI()->getSimPos();
	lastPos.y = 0;

	sumPathFollowDirCalcExecTime = 0;
}

void PathfindingActorController::onTakeDamage(const SDamageInfo& dmg_info){
    
}

void PathfindingActorController::onUpdate( float dt )
{
	updateDirectionChangeTestMode(dt);

	//updateDirectionChange(dt);

	updateJumpingOnObstacle();

}

void PathfindingActorController::onDebugDraw()
{
	getAI()->drawSensesInfo();
}

void PathfindingActorController::updateDirectionChange(float dt)
{
	std::vector<Ogre::Vector3> pathWaypoints;
	Ogre::Vector3 nextTargetPoint;

	Ogre::Vector3 myPos = getAI()->getSimPos();

    if( pathFound )
	{
            g_game->getCurrentLevel()->ogreRecast->CreateRecastPathLine(pathNb) ; 

			pathWaypoints = g_game->getCurrentLevel()->ogreRecast->getPath(pathNb);

			nextTargetPoint = pathWaypoints[nextWaypoint];

			myPos[1] = nextTargetPoint[1];

			float ds = getAI()->getRealSpeed() * dt;
			Ogre::Real distFromTarget = myPos.distance(nextTargetPoint);

			if(useReactivePathFollowing)
			{
				if(ds >= distFromTarget)
				{
					nextWaypoint++;
				}
				else if( distFromTarget <= reactivePathFollowingMinTargetDistance )
				{
					if( pathWaypoints.size() > (nextWaypoint+1) )
					{
						/*if(getAI()->isPositionVisible(pathWaypoints[nextWaypoint+1], 25.0f))
							nextWaypoint++;*/
						
						RayCastResult ray_feet = getAI()->raycast(getAI()->getSimDir(), 0.25f, myPos.distance(pathWaypoints[nextWaypoint+1]));
						if(!ray_feet.hit)
							nextWaypoint++;
						
					}

				}
			}
			else //classic way
			{
				if(distFromTarget < 0.2f || ds >= distFromTarget)
					nextWaypoint++;
			}


			if(nextWaypoint > pathWaypoints.size()-1)
				destReached = true;
	}
	else
	{
		destReached = true;
	}

	if( !destReached )
	{
		Ogre::Vector3 dir;

		dir[0] = nextTargetPoint[0] - myPos[0];
		dir[1] = 0;
		dir[2] = nextTargetPoint[2] - myPos[2];
		dir.normalise();

		getAI()->setDirection(dir);
		getAI()->setSpeed(0.6f);
	}
	else
	{
		getAI()->setSpeed(0.0f);

		targetPos = g_game->getCurrentLevel()->ogreRecast->getRandomNavMeshPoint();
		Ogre::Vector3 beginPos = getAI()->getSimPos();

		beginPos.y = targetPos.y;

		pathFound = ( g_game->getCurrentLevel()->ogreRecast->FindPath(beginPos, targetPos, pathNb, targetId) >=0 );

		destReached = false;
		nextWaypoint = 0;
	}

}

void PathfindingActorController::updateDirectionChangeTestMode(float dt)
{
	stayCounterTestMode++;

	if(stayCounterTestMode < 1000)
	{
		getAI()->setSpeed(0.0f);
		return;
	}
	else
	{
		stayCounterTestMode = 9999;
		getAI()->setSpeed(0.6f);
	}

	std::vector<Ogre::Vector3> pathWaypoints;
	Ogre::Vector3 nextTargetPoint;

	Ogre::Vector3 myPos = getAI()->getSimPos();
	myPos.y = 0;

	if(myPos.distance(lastPos) > 0.0f)
	{
		realPath.push_back(lastPos);

		lastPos = myPos;
	}



    if( pathFound )
	{
            g_game->getCurrentLevel()->ogreRecast->CreateRecastPathLine(pathNb) ; 

			pathWaypoints = g_game->getCurrentLevel()->ogreRecast->getPath(pathNb);

			nextTargetPoint = pathWaypoints[nextWaypoint];

			myPos[1] = nextTargetPoint[1];

			float ds = getAI()->getRealSpeed() * dt;
			Ogre::Real distFromTarget = myPos.distance(nextTargetPoint);


			if(useReactivePathFollowing)
			{
				if(ds >= distFromTarget)
				{
					nextWaypoint++;
				}
				else if( distFromTarget <= reactivePathFollowingMinTargetDistance )
				{
					if( pathWaypoints.size() > (nextWaypoint+1) )
					{
						if(getAI()->isPositionVisible(pathWaypoints[nextWaypoint+1], 25.0f))
							nextWaypoint++;
						
						/*RayCastResult ray_feet = getAI()->raycast(getAI()->getSimDir(), 0.25f, myPos.distance(pathWaypoints[nextWaypoint+1]));
						if(!ray_feet.hit)
							nextWaypoint++;*/
					}

				}
			}
			else //classic way
			{
				if(distFromTarget < 0.25f || ds >= distFromTarget)
					nextWaypoint++;
			}


			if(nextWaypoint > pathWaypoints.size()-1)
			{
				getAI()->setSpeed(0.0f);
				stayCounterTestMode=0;
				destReached = true;

				// draw real path
				Ogre::ManualObject *MOPath = g_game->getOgreSceneMgr()->createManualObject("RealPath");
				MOPath->begin("recastdebug", Ogre::RenderOperation::OT_LINE_STRIP) ;

				Ogre::Real realPathLength = 0;
				Ogre::Vector3 prevPoint = realPath[0];
				MOPath->position(realPath[0].x, realPath[0].y+5.0f, realPath[0].z) ;
				MOPath->colour(Ogre::ColourValue(1,0,0));

				Ogre::Real dot;
				Ogre::Real cross;
				Ogre::Real angle;
				Ogre::Vector2 tmp1, tmp2;
				Ogre::Real angleOverall = 0;

				Ogre::String realPathAngles;

				for(int i=1; i<realPath.size();i++)
				{
					MOPath->position(realPath[i].x, realPath[i].y+5.0f, realPath[i].z) ;
					MOPath->colour(Ogre::ColourValue(1,0,0));
					
					tmp1 = Ogre::Vector2(prevPoint.x, prevPoint.z);
					tmp2 = Ogre::Vector2(realPath[i].x, realPath[i].z);
					
					Ogre::Real a = (tmp1.y - tmp2.y) / (tmp1.x - tmp2.x);

					angle = atan(a) * 180.0f / M_PI;

					angleOverall = angle;

					realPathAngles = "angle;"+ Ogre::StringConverter::toString(angleOverall);
					Ogre::LogManager::getSingletonPtr()->logMessage(realPathAngles);

					realPathLength += prevPoint.distance(realPath[i]);
					prevPoint = realPath[i];
				}

				Ogre::String realPathPoints = "realPathPoints: " + Ogre::StringConverter::toString(realPath.size());
				Ogre::String realPathLen = "realPathLen: " + Ogre::StringConverter::toString(realPathLength);

				Ogre::LogManager::getSingletonPtr()->logMessage(realPathPoints);
				Ogre::LogManager::getSingletonPtr()->logMessage(realPathLen);

				MOPath->end() ;
				g_game->getOgreSceneMgr()->getRootSceneNode()->createChildSceneNode("RealPathSN")->attachObject(MOPath);

				return;
			}
	}
	else
		destReached = true;

	if( !destReached )
	{
		Ogre::Vector3 dir;

		dir[0] = nextTargetPoint[0] - myPos[0];
		dir[1] = 0;
		dir[2] = nextTargetPoint[2] - myPos[2];
		
		dir.normalise();

		getAI()->setDirection(dir);
		getAI()->setSpeed(0.6f);
	}
	else
	{
		// s3:
		//targetPos = Ogre::Vector3(7.0f, 5.0f, 35.5f);
		//Ogre::Vector3 beginPos = Ogre::Vector3(-19.5f, 0.0f, 59.0f);

		//s4:
		//targetPos = Ogre::Vector3(-13.5f, 5.0f, 10.0f);
		//Ogre::Vector3 beginPos = Ogre::Vector3(8.0f, 0.0f, -21.0f);

		//s5:
		//targetPos = Ogre::Vector3(33.0f, 5.0f, -5.0f);
		//Ogre::Vector3 beginPos = Ogre::Vector3(22.0f, 0.0f, 14.0f);

		//s6:
		//targetPos = Ogre::Vector3(-38.0f, 0.0f, -46.0f);
		//Ogre::Vector3 beginPos = Ogre::Vector3(-44.0f, 0.0f, -46.0f);

		//s7:
		targetPos = Ogre::Vector3(8.0f, 1.0f, 35.0f);
		Ogre::Vector3 beginPos = Ogre::Vector3(-21.0f, 1.0f, 59.0f);


		pathFound = ( g_game->getCurrentLevel()->ogreRecast->FindPath(beginPos, targetPos, pathNb, targetId) >=0 );
		destReached = false;
		nextWaypoint = 0;

	}
}

void PathfindingActorController::updateJumpingOnObstacle()
{
	if (getAI()->canJump())
	{
		RayCastResult ray_feet = getAI()->raycast(getAI()->getSimDir(), 0.25f, 1.0f);
		if (ray_feet.hit && ray_feet.collision_type == RayCastResult::Environment)
		{
			RayCastResult ray_eyes = getAI()->raycast(getAI()->getSimDir(), 1.f, 2.f);
			if (!ray_eyes.hit)
			{
				getAI()->setSpeed(1.0f);
				getAI()->jump();
			}
		}
	}
}