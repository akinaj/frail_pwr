/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "utils.h"

class ICameraTarget
{
public:
    virtual mkVec3 CameraTarget_getWorldPosition() const = 0;

    // E.g. for tpp player
    virtual void CameraTarget_setOrientationFromCamera(const mkVec3& camera_world_pos, const mkVec3& camera_lookat_pos) { }
};

class ICamera
{
public:
    virtual void update(float dt, float mouse_dx, float mouse_dy) = 0;

    virtual const mkVec3 getForwardVec() const = 0;
    virtual const mkVec3 getRightVec() const = 0;

    virtual const mkVec3 getPosition() const = 0;
    virtual const mkVec3 getLookatPos() const = 0;
    
    // for freelook only, TODO remove
    virtual void move(const mkVec3& translation) = 0;

    virtual void setTarget(ICameraTarget* target) = 0;
};

class CameraFPP : public ICamera
{
public:
    CameraFPP(Ogre::Camera* cam, ICameraTarget* target);
    
    void update(float dt, float mouse_dx, float mouse_dy);
    void move(const mkVec3& translation);

    const mkVec3 getForwardVec() const;
    const mkVec3 getRightVec() const;

    const mkVec3 getPosition() const;

    const mkVec3 getLookatPos() const;

    virtual void setTarget(ICameraTarget* target);

private:
    const mkVec3 m_forwardUnit;
    const mkVec3 m_upUnit;
    Ogre::Camera* m_cam;
    ICameraTarget* m_target;

    mkVec3 m_lastLookat;

    mkVec3 m_lastPos;
    mkVec3 m_pos;
    float m_yaw;
    float m_roll;

    float m_naturalMovementPt;
    float m_standingTime;

    mkVec3 m_forward;
};

class CameraTPP : public ICamera
{
public:
    CameraTPP(Ogre::Camera* cam, ICameraTarget* target);

    void update(float dt, float mouse_dx, float mouse_dy);
    virtual void setTarget(ICameraTarget* target);

    const mkVec3 getForwardVec() const;
    const mkVec3 getRightVec() const;

    const mkVec3 getPosition() const;

    void move(const mkVec3& translation) { }

    const mkVec3 getLookatPos() const;

private:
    Ogre::Camera* m_cam;
    ICameraTarget* m_target;

    mkVec3 m_lookat;
    mkVec3 m_pos;

    float m_yaw;
    float m_pitch;
};
