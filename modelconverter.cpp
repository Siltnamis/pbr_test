#define AV_GL_GLAD
#define AV_LIB_IMPLEMENTATION
#include "src/mesh.cpp"
#include "src/texture.cpp"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static void processaiMesh(MeshData* data, aiMesh* aimesh)
{
    data->vertex_count = aimesh->mNumVertices;
    data->vertices = (Vertex3D*)malloc(sizeof(Vertex3D)*aimesh->mNumVertices);
    Vertex3D* vertex = data->vertices;
    printf("Vertex count: %d\n", aimesh->mNumVertices);
    for(uint i = 0; i < aimesh->mNumVertices; ++i)
    {
        vertex->pos = { aimesh->mVertices[i].x,
                        aimesh->mVertices[i].y,
                        aimesh->mVertices[i].z };
        vertex->normal = { aimesh->mNormals[i].x,
                           aimesh->mNormals[i].y,
                           aimesh->mNormals[i].z };
        if(aimesh->HasTextureCoords(i))
        {
        vertex->tangent = { aimesh->mTangents[i].x,
                            aimesh->mTangents[i].y,
                            aimesh->mTangents[i].z };
        vertex->uv = { aimesh->mTextureCoords[0][i].x, aimesh->mTextureCoords[0][i].y };
        }else{
            vertex->tangent = {};
            vertex->uv = {};
        }
        ++vertex;
    }

    data->index_count = aimesh->mNumFaces*3;
    data->indices = (uint32*)malloc(sizeof(uint32)*data->index_count);
    uint* index = data->indices;
    for(int i = 0; i < aimesh->mNumFaces; ++i)
    {
        aiFace face = aimesh->mFaces[i];
        for(int j = 0; j < face.mNumIndices; ++j)
        {
            *index = face.mIndices[j];
            ++index;
        }
    }
    data->mode = GL_TRIANGLES;
}

void loadMeshData(MeshData* data, const char* file_name)
{
    const aiScene* scene = aiImportFile(file_name, aiProcess_Triangulate | 
        aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace 
        // | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph);
    );

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        printf("Failed to load model %s\n%s\n", file_name, aiGetErrorString());
        return;
    }
    printf("Meshes: %d\n", scene->mNumMeshes);
    processaiMesh(data, scene->mMeshes[0]);
}

void applyTransformation(MeshData* data, mat4 transform)
{
    for(int i = 0; i < data->vertex_count; ++i)
    {
        vec3 vert = data->vertices[i].pos;
        vec3 normal = data->vertices[i].normal;
        vec3 tangent = data->vertices[i].tangent;
        data->vertices[i].pos = (transform*vec4{vert.x, vert.y, vert.z, 1}).xyz;
        data->vertices[i].normal = (transform*vec4{normal.x, normal.y, normal.z, 0}).xyz;
        data->vertices[i].tangent = 
            (transform*vec4{tangent.x, tangent.y, tangent.z, 0}).xyz;
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
        printf("Usage: %s model outmodel\n", argv[0]);    
    mat4 transform = mat4_identity();
    int arg_index = checkArg(argc, argv, "--makeyup");
    if(arg_index != -1){
        transform = mat4_scale(0.5)*mat4_rotationx(radians(90.0));
        printf("Rotating by 90 on x\n");
    }

    MeshData data;
    loadMeshData(&data, argv[1]);
    applyTransformation(&data, transform);
    writeToFile(&data, argv[2]);

    return 0;
}

