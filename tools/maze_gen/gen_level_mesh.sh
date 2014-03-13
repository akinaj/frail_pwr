#!/bin/sh

python maze_gen.py | python gen_geometry.py | python obj_to_ogre.py > temp.xml
../OgreXmlConverter.exe temp.xml temp.mesh
cp temp.mesh ../../build/data/render_meshes/level.mesh
collision_mesh_gen level.obj collision.bullet
cp collision.bullet ../../build/data/collision.bullet
#cp level.obj ../../build/data/collision_mesh.obj


