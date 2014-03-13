#include "pch.h"
#include "ModelObject.h"
#include "Game.h"
#include "Level.h"
#include "PrefabMgr.h"
#include "scripting/LuaSimpleBinding.h"

IMPLEMENT_RTTI_SCRIPTED(ModelObject, GameObject);

START_RTTI_INIT(ModelObject);
{
    FIELD_STRING(m_prefabName);
    FIELD_STRING(m_meshName);
    FIELD_VEC3(m_worldTransform.position);
    FIELD_QUAT(m_worldTransform.orientation);
    FIELD_BOOL(m_castsShadows);
}
END_RTTI_INIT();

EXPORT_NOARG_METHOD_SCRIPT(ModelObject, getWorldPosition, mkVec3);
EXPORT_VOID_ARG_METHOD_SCRIPT(ModelObject, setWorldPosition, const mkVec3&);
EXPORT_NOARG_METHOD_SCRIPT(ModelObject, getOrientation, mkQuat);
EXPORT_VOID_ARG_METHOD_SCRIPT(ModelObject, setOrientation, const mkQuat&);

START_SCRIPT_REGISTRATION(ModelObject, ctx);
{
    ctx->registerFunc("GetWorldPosition", VOID_METHOD_SCRIPT(ModelObject, getWorldPosition));
    ctx->registerFunc("SetWorldPosition", VOID_METHOD_SCRIPT(ModelObject, setWorldPosition));
    ctx->registerFunc("GetOrientation", VOID_METHOD_SCRIPT(ModelObject, getOrientation));
    ctx->registerFunc("SetOrientation", VOID_METHOD_SCRIPT(ModelObject, setOrientation));
}
END_SCRIPT_REGISTRATION(ctx);

ModelObject::ModelObject()
    : m_visSceneNode(NULL)
    , m_visMesh(NULL)
    , m_castsShadows(false)
{
    m_worldTransform.position = mkVec3::ZERO;
    m_worldTransform.orientation = mkQuat::IDENTITY;
}

void ModelObject::onCreate()
{
    __super::onCreate();

    createVis();
}

void ModelObject::onDestroy()
{
    __super::onDestroy();

    destroyVis();
}

void ModelObject::onUpdate()
{
    __super::onUpdate();

    updateVisSceneNodeTransform();
}

void ModelObject::createVis()
{
    Ogre::SceneManager* smgr = getLevel()->getOgreSceneMgr();
    m_visSceneNode = smgr->getRootSceneNode()->createChildSceneNode(m_worldTransform.position, m_worldTransform.orientation);

    Prefab* prefab = getPrefab();
    if (prefab && prefab->mesh_name != "none")
    {
        m_visMesh = smgr->createEntity(prefab->mesh_name.c_str());

        if (prefab->material_name != "default")
            m_visMesh->setMaterialName(prefab->material_name.c_str());

        m_visSceneNode->attachObject(m_visMesh);
        m_visSceneNode->setScale(prefab->vis_scale, prefab->vis_scale, prefab->vis_scale);
    }
    else if (!m_meshName.empty())
    {
        m_visMesh = smgr->createEntity(m_meshName);
        m_visSceneNode->attachObject(m_visMesh);
    }
    else
    {
        log_error("Neither prefab name nor mesh name of ModelObject named '%s' is set!" 
            " Hint: one can use GameObject class for objects with no visualization", getName().c_str());
    }

    if (getVisMesh())
        getVisMesh()->setCastShadows(m_castsShadows);
}

void ModelObject::destroyVis()
{
    if (!m_visSceneNode)
        return;

    if (m_visMesh)
    {
        m_visMesh->detachFromParent();
        getLevel()->getOgreSceneMgr()->destroyEntity(m_visMesh);
        m_visMesh = NULL;
    }

    getLevel()->getOgreSceneMgr()->destroySceneNode(m_visSceneNode);
}

void ModelObject::updateVisSceneNodeTransform()
{
    if (!m_visSceneNode)
        return;

    m_worldTransform.position = m_visSceneNode->getPosition();
    m_worldTransform.orientation = m_visSceneNode->getOrientation();
}

const Transform& ModelObject::getWorldTransform() const
{
    return m_worldTransform;
}

const mkVec3& ModelObject::getWorldPosition() const
{
    return m_worldTransform.position;
}

const mkQuat& ModelObject::getOrientation() const
{
    return m_worldTransform.orientation;
}

void ModelObject::setWorldTransform( const Transform& new_transform )
{
    m_worldTransform = new_transform;

    if (m_visSceneNode)
    {
        m_visSceneNode->setPosition(new_transform.position);
        m_visSceneNode->setOrientation(new_transform.orientation);
    }
}

void ModelObject::setWorldPosition( const mkVec3& new_pos )
{
    m_worldTransform.position = new_pos;

    if (m_visSceneNode)
        m_visSceneNode->setPosition(new_pos);
}

void ModelObject::setOrientation( const mkQuat& new_orientation )
{
    m_worldTransform.orientation = new_orientation;

    if (m_visSceneNode)
        m_visSceneNode->setOrientation(new_orientation);
}

Ogre::SceneNode* ModelObject::getVisSceneNode() const
{
    return m_visSceneNode;
}

Ogre::Entity* ModelObject::getVisMesh() const
{
    return m_visMesh;
}

Prefab* ModelObject::getPrefab() const
{
    if (m_prefabName.empty())
        return NULL;

    return getLevel()->getPrefabMgr()->get(m_prefabName.c_str());
}

void ModelObject::setPrefabName( const mkString& prefab_name )
{
    m_prefabName = prefab_name;
}

mkVec3 ModelObject::pointWorldToLocal( const mkVec3& world_pt ) const
{
    mkMat4 mat;
    mat.makeInverseTransform(getWorldPosition(), mkVec3::UNIT_SCALE, getOrientation());

    return mat.transformAffine(world_pt);
}

mkVec3 ModelObject::vecWorldToLocal( const mkVec3& world_vec ) const
{
    mkMat4 mat;
    mat.makeInverseTransform(getWorldPosition(), mkVec3::UNIT_SCALE, getOrientation());

    mkVec4 vec4_world(world_vec.x, world_vec.y, world_vec.z, 0.f);
    mkVec4 vec4_local = mat.transformAffine(vec4_world);

    return mkVec3(vec4_local.x, vec4_local.y, vec4_local.z);
}

mkVec3 ModelObject::vecLocalToWorld( const mkVec3& local_vec ) const
{
    mkMat4 mat;
    mat.makeTransform(getWorldPosition(), mkVec3::UNIT_SCALE, getOrientation());

    mkVec4 vec4_local(local_vec.x, local_vec.y, local_vec.z, 0.f);
    mkVec4 vec4_world = mat.transformAffine(vec4_local);

    return mkVec3(vec4_world.x, vec4_world.y, vec4_world.z);
}
