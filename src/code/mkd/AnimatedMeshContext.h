/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once

typedef unsigned short TMeshElementId;
extern TMeshElementId INVALID_MESH_ELEM_ID;

// Helper class for retrieving data and controlling animated OGRE mesh
// Necessary due to poor support for animation control in OGRE
// Can be freely copied, it doesn't hold any resources, but should
// never outlive owner of mesh it points to
// Note: OGRE has virtually no support for helpers/dummys in mesh,
// so we just use bones. 
class AnimatedMeshContext
{
public:
    // Initialization & update
    ////////////////////////////////////////////////////////
    AnimatedMeshContext();

    // Returns whether animated mesh was set to this context
    bool isReady() const;
    void setMesh(Ogre::Entity* entity);

    void update(float dt);
    void debugDraw();

    // Animation control
    ////////////////////////////////////////////////////////
    void setAnimationClipPlayed(const char* clip_name);
    void stopAnimation();
    void setLooped(bool is_looped);

    bool hasAnimationClip(const char* clip_name) const;
    bool isPlayingAnimation() const;

    // Retrieving information from animation
    ////////////////////////////////////////////////////////
    TMeshElementId getElementId(const char* elem_name) const;

    mkMat4 getMeshElementWorldTransform(TMeshElementId id) const;
    mkMat4 getMeshElementWorldTransform(const char* elem_name) const;

    mkVec3 getMeshElementWorldPos(TMeshElementId id) const;
    mkVec3 getMeshElementWorldPos(const char* elem_name) const;
    
    float getClipLength(const char* clip_name) const;

    typedef std::vector<mkString> TClipNamesVec;
    const TClipNamesVec& getAvailableClips() const;

    // Debugging tools
    ////////////////////////////////////////////////////////
    void setDrawElementBoxes(bool show);
    void drawElementBox(const char* elem_name);

private:
    void updateBonesInfo();
    void updateClipsInfo();

    const char* getMeshDebugName() const;

private:
    Ogre::Entity* m_mesh;
    Ogre::SkeletonInstance* m_skeleton;
    Ogre::AnimationState* m_animState;

    typedef std::vector<TMeshElementId> TMeshElementsVec;
    TMeshElementsVec m_meshElemsToDrawBoxes;
    unsigned short m_numBones;

    TClipNamesVec m_availableClips;
};
