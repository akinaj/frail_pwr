/***********************************
 * author: Marcin Kunert 'qmar'    *
 ***********************************/
#include "pch.h"
#include "CameraSettingsSetter.h"
#include "Game.h"

IMPLEMENT_RTTI_NOSCRIPT(CameraSettingsSetter, GameObject);

START_RTTI_INIT(CameraSettingsSetter);
{
	FIELD_BOOL(m_freelookEnabled);
	FIELD_VEC3(m_camStartPos);
	FIELD_VEC3(m_camStartAngle);
}
END_RTTI_INIT();


// When object of this class is created, it changes position of the camera and destroys itself
// It's meant to be used in serialized levels to set the starting position of the camera
CameraSettingsSetter::CameraSettingsSetter()
{
	m_camStartAngle = mkVec3::UNIT_SCALE;
	m_camStartPos = mkVec3::UNIT_SCALE;
}

void CameraSettingsSetter::onCreate()
{
	__super::onCreate();
	if(m_freelookEnabled) 
	{
		g_game->setFreelookCamera(m_camStartPos, m_camStartAngle);
	}
	destroy();
}