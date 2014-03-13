#define _CRT_SECURE_NO_WARNINGS
#include <cassert>
#include <cstdio>
#include <vector>
#include <btBulletDynamicsCommon.h>
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "btBulletWorldImporter.h"

#include "FixedString.h"
#include "Prefab.h"

#pragma warning(error: 4820) // hidden padding

typedef unsigned __int32 uint32;
typedef unsigned char uint8;

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)									\
			((uint32)(uint8)(ch0)	| ((uint32)(uint8)(ch1) << 8) |		\
			((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))
#endif

struct Header
{
	uint32 fourcc;
	uint32 num_prefabs;
};

typedef std::vector<Prefab> PrefabContainer;

void processSourceFile(const char* src_filename, PrefabContainer& output);
void saveOutput(FILE* output_file, PrefabContainer& output);

int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		puts("Usage:\n\n\tprefab_compiler output_filename prefab_src_file_1 prefab_src_file_2 ...\n");
		return 1;
	}

	const char* output_filename = argv[1];
	FILE* output_file = fopen(output_filename, "wb");

	if(output_filename == 0)
	{
		fprintf(stderr, "Could not open output file \"%s\" for writing", output_filename);
		return 2;
	}

	PrefabContainer prefabs;

	for(int i = 2; i < argc; ++i)
	{
		processSourceFile(argv[i], prefabs);
	}

	saveOutput(output_file, prefabs);
	fclose(output_file);

	return 0;
}

const char* skipLeadingWhitespace(const char* input)
{
	while(*input != 0 && (*input == ' ' || *input == '\t' || *input == '\n' || *input == 'r'))
		++input;

	return input;
}

bool isPrefabComplete(const Prefab& prfb)
{
	return (prfb.name != "" && prfb.physics_shape != 0);
}

btCollisionShape* parsePhysicsShape(const char* str)
{
	// capsule-shape width height
	float capsule_width, capsule_height;
	if(sscanf(str, "capsule-shape %f %f", &capsule_width, &capsule_height) == 2)
	{
		return new btCapsuleShape(capsule_width, capsule_height);
	}

	// sphere-shape radius
	float sphere_radius;
	if(sscanf(str, "sphere-shape %f", &sphere_radius) == 1)
	{
		return new btSphereShape(sphere_radius);
	}

	// cylinder-shape radius height
	float cylinder_radius, cylinder_height;
	if(sscanf(str, "cylinder-shape %f %f", &cylinder_radius, &cylinder_height) == 2)
	{
		return new btCylinderShape(btVector3(cylinder_radius, cylinder_height, cylinder_radius));
	}

	return 0;
}

void processSourceFile( const char* src_filename, PrefabContainer& output )
{
	FILE* file = fopen(src_filename, "rt");

	if(file == 0)
	{
		fprintf(stderr, "Could not open input file \"%s\", skipping", src_filename);
		return;
	}

	char buff[512] = {0};
	char str_arg_buff[256] = {0};

	Prefab prfb;
	prfb.mass = 1.0f;
	prfb.mesh_name = "none";
	prfb.physics_shape = 0;
	prfb.material_name = "default";
	prfb.vis_scale = 1.0f;
	prfb.vis_forward_vec.x = prfb.vis_forward_vec.y = 0.f;
	prfb.vis_forward_vec.z = 1.f;
	prfb.vis_offset.x = prfb.vis_offset.y = prfb.vis_offset.z = 0.f;

	while(fgets(buff, 512-1, file))
	{
		const char* line = skipLeadingWhitespace(buff);
		
		// ignore comments and empty lines
		if(*line == 0 || line[0] == '#')
			continue;

		xyz vec;
		float mass, vis_scale;
		if(sscanf(line, "mass: %f", &mass) == 1)
		{
			prfb.mass = mass;
		}
		else if(sscanf(line, "vis_scale: %f", &vis_scale) == 1)
		{
			prfb.vis_scale = vis_scale;
		}
		else if(sscanf(line, "mesh_name: %255s", str_arg_buff) == 1)
		{
			prfb.mesh_name.assign(str_arg_buff);
		}
		else if(sscanf(line, "name: %255s", str_arg_buff) == 1)
		{
			prfb.name.assign(str_arg_buff);
		}
		else if(strstr(line, "physics_shape: ") == line)
		{
			prfb.physics_shape = parsePhysicsShape(line + strlen("physics_shape: "));
		}
		else if(sscanf(line, "material: %255s", str_arg_buff) == 1)
		{
			prfb.material_name.assign(str_arg_buff);
		}
		else if(sscanf(line, "forward_vec: %f, %f, %f", &vec.x, &vec.y, &vec.z) == 3)
		{
			prfb.vis_forward_vec = vec;
		}
		else if(sscanf(line, "vis_offset: %f, %f, %f", &vec.x, &vec.y, &vec.z) == 3)
		{
			prfb.vis_offset = vec;
		}
		else
		{
			fprintf(stderr, "[%s] warning: no match found for line:\n\t%s\n",
				src_filename, line);
		}
	}

	if(!isPrefabComplete(prfb))
	{
		fprintf(stderr, "File \"%s\" is incomplete, skipping\n", src_filename);
	}
	else
	{
		output.push_back(prfb);
	}
}

void saveOutput( FILE* output_file, PrefabContainer& output )
{
	Header header;
	header.fourcc = MAKEFOURCC('P', 'R', 'F', 'B');
	header.num_prefabs = output.size();

	fwrite(&header, sizeof(header), 1, output_file);

	btDefaultSerializer* bullet_serializer = new btDefaultSerializer(1024 * 1024 * 5);
	bullet_serializer->startSerialization();

	for(uint32 i = 0; i < output.size(); ++i)
	{
		fprintf(stdout, "[%s] mesh: %s, mass: %g, shape: %#x, vis_scale: %g, forward_vec: [%g, %g, %g]\n",
			output[i].name.c_str(), output[i].mesh_name.c_str(), output[i].mass, output[i].physics_shape, output[i].vis_scale,
			output[i].vis_forward_vec.x, output[i].vis_forward_vec.y, output[i].vis_forward_vec.z);

		// serialize physics shape and reset ptr to magic value (it will be restored at deserialization)
		output[i].physics_shape->serializeSingleShape(bullet_serializer);
		output[i].physics_shape = (btCollisionShape*)0xdeadf00d;

		// write basic def of prefab
		fwrite(&output[i], sizeof(Prefab), 1, output_file);
	}

	// write serialized bullet collision shapes
	bullet_serializer->finishSerialization();
	fwrite(bullet_serializer->getBufferPointer(), bullet_serializer->getCurrentBufferSize(), 1, output_file);
	fclose(output_file);

	delete bullet_serializer;
}
