#pragma once
#include "IActorController.h"
#include "utils.h"
#include <vector>

class PathfindingActorController : public IActorController
{
public:
	explicit PathfindingActorController(ActorAI* ai);

	virtual void onCreate() { }
    virtual void onTakeDamage(const SDamageInfo& dmg_info);
	virtual void onUpdate(float dt);
	virtual void onDebugDraw();

private:
	void updateDirectionChange(float dt);
	void updateDirectionChangeTestMode(float dt);
	void updateJumpingOnObstacle();

	float m_lastTurnTime;
	mkVec3 m_curBaseDir;
	mkVec3 m_curDestDir;

	int pathNb;
    int targetId;
	bool pathFound;
	bool destReached;
	Ogre::Vector3 targetPos;
	unsigned nextWaypoint;

	bool useReactivePathFollowing;
	float reactivePathFollowingMinTargetDistance;

	static unsigned pathSlotCount;

	unsigned stayCounterTestMode;

	std::vector< Ogre::Vector3 > realPath;
	Ogre::Vector3 lastPos;

	double sumPathFollowDirCalcExecTime;
};
