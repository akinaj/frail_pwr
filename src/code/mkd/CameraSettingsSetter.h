/***********************************
 * author: Marcin Kunert 'qmar'    *
 ***********************************/
#pragma once
#include "GameObject.h"
class CameraSettingsSetter : public GameObject
{
    DECLARE_RTTI(CameraSettingsSetter);

public:
	CameraSettingsSetter();
	virtual void onCreate();

private: 
	bool m_freelookEnabled;
	mkVec3 m_camStartAngle;
	mkVec3 m_camStartPos;
};