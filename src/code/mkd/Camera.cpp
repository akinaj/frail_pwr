/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "Camera.h"
#include "contrib/DebugDrawer.h"

CameraFPP::CameraFPP(Ogre::Camera* cam, ICameraTarget* target)
    : m_cam(cam)
    , m_pos(0, 0, 0)
    , m_yaw(0)
    , m_roll(0)
    , m_forwardUnit(0, 0, -1)
    , m_upUnit(0, 1, 0)
    , m_target(target)
{

}

void CameraFPP::update( float dt, float mouse_dx, float mouse_dy )
{
    if (m_target)
        m_pos = m_target->CameraTarget_getWorldPosition();

    const float kNaturalMovementStr = 0.0f;

    if (m_lastPos == m_pos)
    {
        m_naturalMovementPt = 0;
    }
    else
    {
        m_naturalMovementPt += dt;
    }

    const float xoffs = cosf(m_naturalMovementPt * 3) * kNaturalMovementStr;
    const float yoffs = sinf(m_naturalMovementPt * 6) * kNaturalMovementStr;


    const float kMaxAbsRoll = (float)M_PI_2 * 0.99f;

    const float kMouseSensitivity = 0.005f;
    const float y_rot_speed = -0.5f * kMouseSensitivity;
    const float x_rot_speed = -0.5f * kMouseSensitivity;

    m_yaw += x_rot_speed * mouse_dx;
    m_roll += y_rot_speed * mouse_dy; // todo roll czy pitch? :)

    m_roll = clamp(m_roll, -kMaxAbsRoll, kMaxAbsRoll);

    // recalc look-at vec
    const Ogre::Quaternion yaw_rot = Ogre::Quaternion(Ogre::Radian(m_yaw), mkVec3(0, 1, 0));
    const Ogre::Quaternion roll_rot = Ogre::Quaternion(Ogre::Radian(m_roll), mkVec3(1, 0, 0));
    m_forward =  yaw_rot * roll_rot * m_forwardUnit;
    m_forward.normalise();

    const mkVec3 natural_movement_offset = getRightVec().normalisedCopy() * xoffs * 2 + mkVec3(0, yoffs, 0);

    const mkVec3 lookat = m_pos + natural_movement_offset + m_forward;

    m_cam->setPosition(m_pos + natural_movement_offset);
    m_cam->lookAt(lookat);

    m_lastLookat = lookat;

    static float fovy = 70.f;
    m_cam->setFOVy(Ogre::Degree(fovy));

    m_lastPos = m_pos;
}

void CameraFPP::move( const mkVec3& translation )
{
    m_pos += translation;
}

const mkVec3 CameraFPP::getForwardVec() const
{
    return m_forward;
}

const mkVec3 CameraFPP::getRightVec() const
{
    return m_forward.crossProduct(m_upUnit);
}

const mkVec3 CameraFPP::getPosition() const
{
    return m_pos;
}

void CameraFPP::setTarget( ICameraTarget* target )
{
    m_target = target;
    
    if (m_target)
        m_pos = m_target->CameraTarget_getWorldPosition();
}

const mkVec3 CameraFPP::getLookatPos() const
{
    return m_lastLookat;
}

CameraTPP::CameraTPP( Ogre::Camera* cam, ICameraTarget* target )
    : m_cam(cam)
    , m_target(target)
    , m_lookat(mkVec3::ZERO)
    , m_pos(mkVec3::ZERO)
    , m_yaw(0.f)
    , m_pitch(0.f)
{

}

void CameraTPP::update( float dt, float mouse_dx, float mouse_dy )
{
    mkVec3 target_pos = mkVec3::ZERO;

    if (m_target)
        target_pos = m_target->CameraTarget_getWorldPosition();

    static float BASE_OFFSET = 5.5f;
    mkVec3 cam_offset = -mkVec3::UNIT_Z * BASE_OFFSET;

    static float SCALE = 0.005f;
    
    const float y_rot_speed = 0.5f * SCALE;
    const float x_rot_speed = -0.5f * SCALE;

    m_yaw += x_rot_speed * mouse_dx;
    m_pitch += y_rot_speed * mouse_dy;

    m_pitch = clamp(m_pitch, -1.f, 1.f);

    const Ogre::Quaternion yaw_rot = Ogre::Quaternion(Ogre::Radian(m_yaw), mkVec3(0, 1, 0));
    const Ogre::Quaternion pitch_rot = Ogre::Quaternion(Ogre::Radian(m_pitch), mkVec3(1, 0, 0));
    cam_offset =  yaw_rot * pitch_rot * cam_offset;

    static float BASE_TARGET_XOFFSET = -1.f;
    static float BASE_TARGET_ZOFFSET = 1.f;
    static float BASE_TARGET_YOFFSET = 2.f;

    target_pos += yaw_rot * pitch_rot * (BASE_TARGET_XOFFSET * mkVec3::UNIT_X + BASE_TARGET_ZOFFSET * mkVec3::UNIT_Z + BASE_TARGET_YOFFSET * mkVec3::UNIT_Y);

    m_pos = target_pos + cam_offset;

    mkVec3 lookat_offset = -cam_offset;
    lookat_offset.normalise();

    static float LOOKAT_OFFSET = 10.f;
    static float LOOKAT_VERT_OFFSET = 0.f;
    m_lookat = target_pos + lookat_offset * LOOKAT_OFFSET; + mkVec3::UNIT_Y * LOOKAT_VERT_OFFSET;

    m_cam->setPosition(m_pos);
    m_cam->lookAt(m_lookat);

    static float fovy = 70.f;
    m_cam->setFOVy(Ogre::Degree(fovy));

    //DebugDrawer::getSingleton().drawSphere(m_lookat, 0.1f, Ogre::ColourValue::White);
}

void CameraTPP::setTarget( ICameraTarget* target )
{
    m_target = target;
}

const mkVec3 CameraTPP::getForwardVec() const
{
    return (m_lookat - m_pos).normalisedCopy();
}

const mkVec3 CameraTPP::getRightVec() const
{
    return getForwardVec().crossProduct(mkVec3::UNIT_Y);
}

const mkVec3 CameraTPP::getPosition() const
{
    return m_pos;
}

const mkVec3 CameraTPP::getLookatPos() const
{
    return m_lookat;
}
