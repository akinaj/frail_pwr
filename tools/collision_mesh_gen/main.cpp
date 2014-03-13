// Create .bullet static collision mesh from OBJ file

#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <vector>
#include <btBulletDynamicsCommon.h>

struct xyz
{
	float x, y, z;
};

class FileGuard
{
	FileGuard(const FileGuard&); void operator=(const FileGuard&); // noncopyable
	FILE* f;

public:
	explicit FileGuard(FILE* file) : f(file) { }
	~FileGuard()
	{
		if(f != stdin) { fclose(f); }
	}
};

bool openFiles(int argc, char* argv[], FILE** out_input_file, FILE** out_output_file);
bool loadObj(FILE* input_file, std::vector<xyz>& out_vertices, std::vector<int>& out_indices);
void createCollisionMesh(std::vector<xyz>& out_vertices, std::vector<int>& out_indices, FILE* output_file);

int main(int argc, char* argv[])
{
	FILE* input_file = 0;
	FILE* output_file = 0;

	if(!openFiles(argc, argv, &input_file, &output_file))
	{
		return 1;
	}

	FileGuard input_guard(input_file), output_guard(output_file);

	std::vector<xyz> vertices;
	std::vector<int> indices;

	if(!loadObj(input_file, vertices, indices))
	{
		fprintf(stderr, "Could not load OBJ mesh from input file\n");
		return 2;
	}

	createCollisionMesh(vertices, indices, output_file);
}

bool openFiles(int argc, char* argv[], FILE** out_input_file, FILE** out_output_file)
{
	if(argc < 2 || argc > 3)
	{
		puts("Usage:\n\tcollision_mesh_gen [input_file] output_file\n");
		return false;
	}

	const char* output_file_name = 0;

	// only output_file given, use stdin as input
	if(argc == 2)
	{
		output_file_name = argv[1];
		*out_input_file = stdin;

		puts("Input file: stdin\n");
	}
	else
	{
		output_file_name = argv[2];

		printf("Input file: \"%s\"\n", argv[1]);
		*out_input_file = fopen(argv[1], "rt");
		if(*out_input_file == 0)
		{
			fprintf(stderr, "Could not open input file \"%s\" for reading\n", argv[1]);
			return false;
		}
	}

	printf("Output file: \"%s\"\n", output_file_name);
	*out_output_file = fopen(output_file_name, "wb");
	if(*out_output_file == 0)
	{
		fprintf(stderr, "Could not open output file \"%s\" for writing\n", output_file_name);

		if(*out_input_file != stdin) fclose(*out_input_file);
		return false;
	}

	return true;
}

int max(int a, int b, int c, int d)
{
	return std::max(d, std::max(c, std::max(a, b)));
}

bool loadObj( FILE* input_file, std::vector<xyz>& out_vertices, std::vector<int>& out_indices )
{
	// Simple loader of OBJ subset (handle only triangles, load vertex positions and indices, ignore everything else)
	unsigned num_triangles = 0;

	int max_idx = -1;

	char buff[512] = {0};
	while(fgets(buff, 512-1, input_file))
	{
		int ignore, idx1, idx2, idx3;
		xyz vertex;

		if(sscanf(buff, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z) == 3)
		{
			out_vertices.push_back(vertex);
		}
		else if(sscanf(buff, "f %d/%d/%d %d/%d/%d %d", &idx1, &ignore, &ignore, &idx2, &ignore, &ignore, &idx3) == 7)
		{
			out_indices.push_back(idx1 - 1);
			out_indices.push_back(idx2 - 1);
			out_indices.push_back(idx3 - 1);

			max_idx = max(max_idx, idx1-1, idx2-1, idx3-1);

			++num_triangles;
		}
	}

	if(max_idx >= (int)out_vertices.size())
	{
		fprintf(stderr, "Input file contains faces referencing non-existent vertex on idx %d", max_idx + 1);
		return false;
	}

	if(out_indices.empty())
	{
		fprintf(stderr, "Input file is empty");
		return false;
	}

	return true;
}

void createCollisionMesh(std::vector<xyz>& out_vertices, std::vector<int>& out_indices,
						 FILE* output_file)
{
	btTriangleIndexVertexArray* iva = new btTriangleIndexVertexArray(
		out_indices.size() / 3,
		&out_indices[0],
		sizeof(int) * 3,
		out_vertices.size(),
		(btScalar*)&out_vertices[0].x,
		sizeof(xyz));

	bool use_quantized_aabb_compression = true;
	btBvhTriangleMeshShape* trimesh_shape = new btBvhTriangleMeshShape(iva, use_quantized_aabb_compression);

	int max_serialize_buff_size = 1024 * 1024 * 5;
	btDefaultSerializer* serializer = new btDefaultSerializer(max_serialize_buff_size);
	
	serializer->startSerialization();
	trimesh_shape->serializeSingleBvh(serializer);
	trimesh_shape->serializeSingleShape(serializer);
	serializer->finishSerialization();

	fwrite(serializer->getBufferPointer(), serializer->getCurrentBufferSize(), 1, output_file);

	delete serializer;
	delete trimesh_shape;
	delete iva;
}
