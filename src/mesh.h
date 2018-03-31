#pragma once

#include "av/av.h"
#include "av/avmath.h"

#include "opengl.h"
#include "texture.h"

struct Vertex2D
{
    vec2    pos;
    vec2    uv;
    vec4    color;
};

struct Vertex3D
{
    vec3    pos;
    vec3    normal; 
    vec2    uv;
    vec3    tangent;

    float   weights[4];
    int     bone_ids[4];
};

struct MeshData
{
    int32       mode;
    Vertex3D*   vertices;
    uint32*     indices;
    int32       vertex_count;
    int32       index_count;
};

struct MeshData2D
{
    GLenum      mode;
    Vertex2D*   vertices;
    uint32*     indices;
    int32       vertex_count;
    int32       index_count;
};

struct Material
{
    //if textured  | assume always textured
    int     albedo_map;
    int     normal_map;
    int     roughness_map; 
    int     metalic_map;

    //else
    vec3    albedo;
    float   roughness;
    float   metalic;
};

int loadMaterial(Material* material, const char* albedo_map, const char* normal_map,
    const char* roughness_map, const char* metalic_map, bool metalic = false);

void loadMaterial(Material* material, vec3 albedo, float roughness, float metalic);

struct Mesh
{
    GLenum  mode;
    GLuint  vao;
    GLuint  vertex_buff;
    GLuint  index_buff;
    int32   index_count;
    int32   vertex_count;
};

#define MAX_MESHES_PER_MODEL 32
struct Model
{
    int32       mesh_count; 
    Mesh        meshes[MAX_MESHES_PER_MODEL];
};

void loadMesh(Mesh* mesh, const MeshData* data);
void loadMesh(Mesh* mesh, const MeshData2D *data);
void freeMesh(Mesh* mesh);

//requires assimp
// void loadMeshData(MeshData* data, const char* file_name);
// void loadMesh(Mesh* mesh, const char* file_name);

void loadFromFile(MeshData* data, const char* file_name);
void writeToFile(const MeshData* data, const char* file_name);

void generateTerrain(MeshData* data, float sizex, float sizez, int numx, int numz);
void generateFlatGrid(MeshData* data, float sizex, float sizez, int numx, int numz);
void generateFlatTerrain(MeshData* data, float sizex, float sizez, int numx, int numz);
void generatePlane(MeshData* data, vec3 a, vec3 b, vec3 c, vec3 d, float uv = 1.f);
void generateCube(MeshData* data, float sz);
void generateCylinder(MeshData* data, float radius, float height, int ctriangles);
void generateCone(MeshData* data, float radius, float height, int ctriangles);
void generateSphere(MeshData* data, int slices, int rings, float radius = 1.0f);
void generateSkybox(MeshData* data, float sz);
void generateAxisVectors(MeshData* data, float len);
void generateQuad(MeshData2D* data, float size);
void freeMeshData(MeshData* data);
void freeMeshData(MeshData2D *data);
