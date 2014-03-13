#include "pch.h"
#include "StaticCollidableMesh.h"
#include "Level.h"

IMPLEMENT_RTTI_NOSCRIPT(StaticCollidableMesh, ModelObject);

START_RTTI_INIT(StaticCollidableMesh);
{

}
END_RTTI_INIT();

StaticCollidableMesh::StaticCollidableMesh()
    : m_physicsBody(NULL)
{
    // Default value of castsShadows for static collidable meshes is false.
    // However, this can be overriden from preset or level file - that's why its not set to false in onCreate
    m_castsShadows = false;
}

void StaticCollidableMesh::onCreate()
{
    __super::onCreate();

    btBvhTriangleMeshShape* trimesh_shape = createCollisionFromRenderingMesh();

    btTransform ground_transform;
    ground_transform.setIdentity();
    ground_transform.setOrigin(ogre_to_bullet(getWorldPosition()));
    ground_transform.setRotation(ogre_to_bullet(getOrientation()));

    btScalar mass = 0;
    btVector3 local_inertia(0, 0, 0);
    btDefaultMotionState* ground_motion_state = new btDefaultMotionState(ground_transform);
    btRigidBody::btRigidBodyConstructionInfo rb_info(mass, ground_motion_state, trimesh_shape, local_inertia);
    m_physicsBody = new btRigidBody(rb_info);
    m_physicsBody->setCollisionFlags(m_physicsBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);

    getLevel()->getPhysicsWorld()->addRigidBody(m_physicsBody);
}

void StaticCollidableMesh::onDestroy()
{
    __super::onDestroy();
}

btBvhTriangleMeshShape* StaticCollidableMesh::createCollisionFromRenderingMesh()
{
    Ogre::MeshPtr mesh = getVisMesh()->getMesh();

    // Extracting rendering mesh (from Ogre wiki: www.ogre3d.org/wiki/index.php/RetrieveVertexData
    bool added_shared = false;
    size_t current_offset = 0;
    size_t shared_offset = 0;
    size_t next_offset = 0;
    size_t index_offset = 0;

    size_t vertex_count = 0;
    size_t index_count = 0;

    // Calculate how many vertices and indices we're going to need
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh( i );

        // We only need to add the shared vertices once
        if (submesh->useSharedVertices)
        {
            if ( !added_shared )
            {
                vertex_count += mesh->sharedVertexData->vertexCount;
                added_shared = true;
            }
        }
        else
        {
            vertex_count += submesh->vertexData->vertexCount;
        }

        // Add the indices
        index_count += submesh->indexData->indexCount;
    }

    // Allocate space for the vertices and indices
    std::vector<Ogre::Vector3> vertices;
    std::vector<unsigned long> indices;

    vertices.reserve(vertex_count);
    indices.reserve(index_count);

    added_shared = false;

    // Run through the submeshes again, adding the data into the arrays
    for ( unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);

        Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

        if ((!submesh->useSharedVertices) || (submesh->useSharedVertices && !added_shared))
        {
            if (submesh->useSharedVertices)
            {
                added_shared = true;
                shared_offset = current_offset;
            }

            const Ogre::VertexElement* posElem =
                vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

            Ogre::HardwareVertexBufferSharedPtr vbuf =
                vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

            unsigned char* vertex =
                static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

            // There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
            //  as second argument. So make it float, to avoid trouble when Ogre::Real will
            //  be comiled/typedefed as double:
            //      Ogre::Real* pReal;
            float* pReal;

            for ( size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
            {
                posElem->baseVertexPointerToElement(vertex, &pReal);

                Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);

                vertices.push_back(pt);
            }

            vbuf->unlock();
            next_offset += vertex_data->vertexCount;
        }

        Ogre::IndexData* index_data = submesh->indexData;

        size_t num_tris = index_data->indexCount / 3;
        Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

        unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);

        size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;

        if ( use32bitindexes )
        {
            for ( size_t k = 0; k < num_tris*3; ++k)
            {
                indices.push_back(pLong[k] + static_cast<unsigned long>(offset));
            }
        }
        else
        {
            for ( size_t k = 0; k < num_tris*3; ++k)
            {
                indices.push_back(static_cast<unsigned long>(pShort[k]) +
                    static_cast<unsigned long>(offset));
            }
        }

        ibuf->unlock();
        current_offset = next_offset;
    }

    btVector3* btvertices = new btVector3[ vertex_count ];
    int* btindices = new int[ index_count ];

    for (unsigned int i = 0; i < vertex_count; i++)
    {
        btvertices[i].setValue( vertices[i].x, vertices[i].y, vertices[i].z);
    }

    for (unsigned int i = 0; i < index_count; i++)
    {
        btindices[i] = indices[i];
    }

    int indexStride = 3 * sizeof(int);
    int vertStride = sizeof(btVector3);

    btTriangleIndexVertexArray* iva = new btTriangleIndexVertexArray( indices.size() / 3,
        btindices,
        indexStride,
        vertex_count,
        (btScalar*) &btvertices[0].x(),
        vertStride );

    bool useQuantizedAabbCompression = true;
    return new btBvhTriangleMeshShape( iva, useQuantizedAabbCompression );
}