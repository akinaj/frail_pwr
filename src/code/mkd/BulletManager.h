#pragma once
#include "GameObject.h"

struct SBulletInfo
{
    mkVec3 start;
    mkVec3 end;
    bool hit;
    float creation_time;
};

class BulletManager : public GameObject
{
    DECLARE_RTTI(BulletManager);

public:
    BulletManager();

    virtual void onUpdate();
    virtual void onRender();

    void addBullet(const mkVec3& start_point, const mkVec3& end_point, bool hit);

private:
    typedef std::vector<SBulletInfo> TBulletInfoVec;
    TBulletInfoVec m_bullets;
};
