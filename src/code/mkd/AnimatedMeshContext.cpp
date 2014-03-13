/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "AnimatedMeshContext.h"
#include "utils.h"
#include "contrib/DebugDrawer.h"

TMeshElementId INVALID_MESH_ELEM_ID = std::numeric_limits<TMeshElementId>::max();

AnimatedMeshContext::AnimatedMeshContext()
    : m_mesh(NULL)
    , m_skeleton(NULL)
    , m_animState(NULL)
    , m_numBones(0)
{

}

void AnimatedMeshContext::setMesh( Ogre::Entity* entity )
{
    m_mesh = entity;
    m_skeleton = NULL;

    updateBonesInfo();
    updateClipsInfo();
}

void AnimatedMeshContext::setAnimationClipPlayed( const char* clip_name )
{
    MK_ASSERT(isReady());

    // Just cut old animation and start new one - this is just testing framework not real game engine,
    // so no need to blend animations in any way (however - it can be done easily, OGRE has simple anim blender)

    Ogre::AnimationState* new_state = m_mesh->getAnimationState(clip_name);
    if (!new_state)
    {
        log_error("Mesh '%s' has no animation clip '%s'!", getMeshDebugName(), clip_name);
    }
    else
    {
        if (m_animState && m_animState != new_state)
            stopAnimation();

        m_animState = new_state;
        m_animState->setEnabled(true);
    }
}

void AnimatedMeshContext::update( float dt )
{
    if (m_animState)
    {
        m_animState->addTime(dt);
    }
}

bool AnimatedMeshContext::isReady() const
{
    return m_mesh != NULL && m_mesh->getAllAnimationStates() != NULL;
}

bool AnimatedMeshContext::hasAnimationClip( const char* clip_name ) const
{
    MK_ASSERT(isReady());

    return m_mesh->getAllAnimationStates() && m_mesh->getAllAnimationStates()->hasAnimationState(clip_name);
}

void AnimatedMeshContext::stopAnimation()
{
    MK_ASSERT(isReady());
    MK_ASSERT(m_animState);

    if (m_animState)
    {
        m_animState->setTimePosition(0.f);
        m_animState->setEnabled(false); // TODO will this return animated object to default pose?
        m_animState = NULL;
    }
}

void AnimatedMeshContext::setLooped( bool is_looped )
{
    MK_ASSERT(isReady());
    MK_ASSERT(m_animState);

    if (m_animState)
        m_animState->setLoop(is_looped);
}

bool AnimatedMeshContext::isPlayingAnimation() const
{
    if (!isReady())
        return false;

    if (!m_animState)
        return false;

    if (m_animState->hasEnded())
        return false;

    return true;
}

void AnimatedMeshContext::setDrawElementBoxes( bool show )
{
    m_meshElemsToDrawBoxes.clear();

    if (!show)
        return;

    m_meshElemsToDrawBoxes.resize(m_numBones);
    for (TMeshElementId id = 0; id < m_numBones; ++id)
        m_meshElemsToDrawBoxes[id] = id;
}

void AnimatedMeshContext::drawElementBox( const char* elem_name )
{
    TMeshElementId id = getElementId(elem_name);
    if (id == INVALID_MESH_ELEM_ID)
        return;

    if (std::find(m_meshElemsToDrawBoxes.begin(), m_meshElemsToDrawBoxes.end(), id) == m_meshElemsToDrawBoxes.end())
        m_meshElemsToDrawBoxes.push_back(id);
}

void AnimatedMeshContext::updateBonesInfo()
{
    if (!m_mesh)
        return;

    m_skeleton = m_mesh->getSkeleton();
    if (!m_skeleton)
        return;

    m_numBones = m_skeleton->getNumBones();
}

void AnimatedMeshContext::debugDraw()
{
    const float unit = 2.f;

    mkVec3 orig_bone_cuboid[] =
    {
        mkVec3( unit, -unit, -unit),
        mkVec3(-unit, -unit, -unit),
        mkVec3(-unit,  unit, -unit),
        mkVec3( unit,  unit, -unit),

        mkVec3(-unit,  unit,  unit),
        mkVec3(-unit, -unit,  unit),
        mkVec3( unit, -unit,  unit),
        mkVec3( unit,  unit,  unit)
    };

    mkVec3 transformed_cuboid[8];

    for (size_t i = 0; i < m_meshElemsToDrawBoxes.size(); ++i)
    {
        TMeshElementId id = m_meshElemsToDrawBoxes[i];
        mkMat4 mat = getMeshElementWorldTransform(id);
        mat.setScale(mkVec3(0.01f, 0.01f, 0.01f));

        for (int vert_idx = 0; vert_idx < 8; ++vert_idx)
        {
            transformed_cuboid[vert_idx] = mat.transformAffine(orig_bone_cuboid[vert_idx]);
        }

        DebugDrawer::getSingleton().drawCuboid(&transformed_cuboid[0], Ogre::ColourValue::White);
    }
}

mkVec3 AnimatedMeshContext::getMeshElementWorldPos( TMeshElementId id ) const
{
    return getMeshElementWorldTransform(id).getTrans();
}

mkVec3 AnimatedMeshContext::getMeshElementWorldPos( const char* elem_name ) const
{
    MK_ASSERT(isReady());

    TMeshElementId id = getElementId(elem_name);
    MK_ASSERT_MSG(id != INVALID_MESH_ELEM_ID, "Mesh '%s' has no element with name '%s'",
        getMeshDebugName(), elem_name);

    return getMeshElementWorldPos(id);
}

mkMat4 AnimatedMeshContext::getMeshElementWorldTransform( TMeshElementId id ) const
{
    MK_ASSERT(isReady());
    MK_ASSERT(id != INVALID_MESH_ELEM_ID);

    if (id == INVALID_MESH_ELEM_ID)
        return mkMat4::IDENTITY;

    Ogre::Bone* bone = m_skeleton->getBone(id);
    MK_ASSERT(bone);

    const mkMat4& full_anim_transform = bone->_getFullTransform();
    const mkMat4& parent_node_transform = m_mesh->getParentSceneNode()->_getFullTransform();

    return parent_node_transform * full_anim_transform;
}

mkMat4 AnimatedMeshContext::getMeshElementWorldTransform( const char* elem_name ) const
{
    MK_ASSERT(isReady());

    TMeshElementId id = getElementId(elem_name);
    MK_ASSERT_MSG(id != INVALID_MESH_ELEM_ID, "Mesh '%s' has no element with name '%s'",
        getMeshDebugName(), elem_name);

    return getMeshElementWorldTransform(id);
}

const char* AnimatedMeshContext::getMeshDebugName() const
{
    if (m_mesh == NULL)
        return "(null)";
    else
        return m_mesh->getMesh()->getName().c_str();
}

TMeshElementId AnimatedMeshContext::getElementId( const char* elem_name ) const
{
    MK_ASSERT(elem_name);
    MK_ASSERT(isReady());

    Ogre::Bone* bone = m_skeleton->getBone(elem_name);
    if (bone == NULL)
        return INVALID_MESH_ELEM_ID;

    return bone->getHandle();
}

void AnimatedMeshContext::updateClipsInfo()
{
    m_availableClips.clear();

    if (!m_mesh)
        return;

    m_skeleton = m_mesh->getSkeleton();
    if (!m_skeleton)
        return;

    int anim_num = m_skeleton->getNumAnimations();
    for (int i = 0; i < anim_num; ++i)
    {
        Ogre::Animation* anim = m_skeleton->getAnimation(i);
        m_availableClips.push_back(anim->getName());
    }
}

const AnimatedMeshContext::TClipNamesVec& AnimatedMeshContext::getAvailableClips() const
{
    return m_availableClips;
}

float AnimatedMeshContext::getClipLength( const char* clip_name ) const
{
    MK_ASSERT(clip_name);
    MK_ASSERT(isReady());
    MK_ASSERT(hasAnimationClip(clip_name));

    if (!m_skeleton)
        return 0.f;

    Ogre::Animation* anim = m_skeleton->getAnimation(clip_name);
    if (!anim)
        return 0.f;

    return anim->getLength();
}
