#include "mesh.h"

void loadMesh(Mesh* mesh, const MeshData* data)
{
    mesh->mode = data->mode;
    mesh->index_count = data->index_count;
    mesh->vertex_count = data->vertex_count;
    GLCALL(glGenVertexArrays(1, &mesh->vao));
    GLCALL(glBindVertexArray(mesh->vao));

    GLCALL(glGenBuffers(1, &mesh->vertex_buff));
    GLCALL(glGenBuffers(1, &mesh->index_buff));

    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buff));
    GLCALL(glBufferData(GL_ARRAY_BUFFER, mesh->vertex_count*sizeof(Vertex3D),
            data->vertices, GL_STATIC_DRAW));

    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buff));
    GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->index_count*sizeof(uint32),
            data->indices, GL_STATIC_DRAW));

    GLCALL(glEnableVertexAttribArray(0));   //positions
    GLCALL(glEnableVertexAttribArray(1));   //normals
    GLCALL(glEnableVertexAttribArray(2));   //uvs
    GLCALL(glEnableVertexAttribArray(3));   //tangents
    GLCALL(glEnableVertexAttribArray(4));   //weights
    GLCALL(glEnableVertexAttribArray(5));   //ids

    GLCALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), 0));
    GLCALL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), 
            (const void*)offsetof(Vertex3D, normal)));
    GLCALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), 
            (const void*)offsetof(Vertex3D, uv)));
    GLCALL(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), 
            (const void*)offsetof(Vertex3D, tangent)));
    GLCALL(glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), 
            (const void*)offsetof(Vertex3D, weights)));
    GLCALL(glVertexAttribPointer(5, 4, GL_INT, GL_FALSE, sizeof(Vertex3D), 
            (const void*)offsetof(Vertex3D, bone_ids)));
}

void loadMesh(Mesh *mesh, const MeshData2D *data)
{
    mesh->mode = data->mode;
    mesh->index_count = data->index_count;
    mesh->vertex_count = data->vertex_count;
    GLCALL(glGenVertexArrays(1, &mesh->vao));
    GLCALL(glBindVertexArray(mesh->vao));

    GLCALL(glGenBuffers(1, &mesh->vertex_buff));
    GLCALL(glGenBuffers(1, &mesh->index_buff));

    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buff));
    GLCALL(glBufferData(GL_ARRAY_BUFFER, mesh->vertex_count*sizeof(Vertex2D),
            data->vertices, GL_STATIC_DRAW));

    GLCALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buff));
    GLCALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->index_count*sizeof(uint32),
            data->indices, GL_STATIC_DRAW));

    GLCALL(glEnableVertexAttribArray(0));   //positions
    GLCALL(glEnableVertexAttribArray(1));   //uvs
    GLCALL(glEnableVertexAttribArray(2));   //colors

    GLCALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), 0));
    GLCALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D),
            (const void*)offsetof(Vertex2D, uv)));
    GLCALL(glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D),
            (const void*)offsetof(Vertex2D, color)));

}

void freeMesh(Mesh* mesh)
{
    GLCALL(glDeleteVertexArrays(1, &mesh->vao));
    GLCALL(glDeleteBuffers(1, &mesh->index_buff));
    GLCALL(glDeleteBuffers(1, &mesh->vertex_buff));
}

void calcTangents(MeshData* data)
{
    for(int i = 0; i < data->index_count; i += 3){
        Vertex3D& v0 = data->vertices[data->indices[i+0]];
        Vertex3D& v1 = data->vertices[data->indices[i+1]];
        Vertex3D& v2 = data->vertices[data->indices[i+2]];

        vec3 delta_pos1 = v1.pos - v0.pos;
        vec3 delta_pos2 = v2.pos - v0.pos;

        vec2 delta_uv1 = v1.uv - v0.uv;
        vec2 delta_uv2 = v2.uv - v0.uv;

        float r = 1.0f/(delta_uv1.x*delta_uv2.y - delta_uv1.y*delta_uv2.x);
        vec3 tangent = (delta_pos1*delta_uv2.y - delta_pos2*delta_uv1.y)*r;
        vec3 bitangent = (delta_pos2*delta_uv1.x - delta_pos1*delta_uv2.x)*r;

        vec3 smooth_bitangent = normalize(cross(v0.normal, tangent));
        vec3 smooth_tangent = normalize(cross(smooth_bitangent, v0.normal));
        v0.tangent = normalize(smooth_tangent);

        smooth_bitangent = normalize(cross(v1.normal, tangent));
        smooth_tangent = normalize(cross(smooth_bitangent, v1.normal));
        v1.tangent = normalize(smooth_tangent);

        smooth_bitangent = normalize(cross(v2.normal, tangent));
        smooth_tangent = normalize(cross(smooth_bitangent, v2.normal));
        v2.tangent = normalize(smooth_tangent);
    }
}

void generatePlane(MeshData* data, vec3 a, vec3 b, vec3 c, vec3 d, float uv)
{
    data->vertex_count = 4;
    data->vertices = (Vertex3D*)malloc(sizeof(Vertex3D)*4);      
    vec3 normal = normalize(cross(b-a, d-a));

    data->vertices[0].normal = normal;
    data->vertices[0].pos = a;
    data->vertices[0].uv = {0, 0};

    data->vertices[1].normal = normal;
    data->vertices[1].pos = b;
    data->vertices[1].uv = {0, uv};

    data->vertices[2].normal = normal;
    data->vertices[2].pos = c;
    data->vertices[2].uv = {uv, uv};

    data->vertices[3].normal = normal;
    data->vertices[3].pos = d;
    data->vertices[3].uv = {uv, 0};

    data->index_count = 6;
    data->indices = (uint32*)malloc(sizeof(uint32*)*6);

    data->indices[0] = 0;
    data->indices[1] = 1;
    data->indices[2] = 2;
    data->indices[3] = 0;
    data->indices[4] = 2;
    data->indices[5] = 3;

    data->mode = GL_TRIANGLES;

    calcTangents(data);
}

void generateCube(MeshData* data, float sz)
{
    MeshData sides[6];

    float size = sz/2.f;
#if 0 // y up
    //front
    generatePlane(&sides[0], {-size, size, size},    {-size,-size, size},
                             {size, -size, size},    {size, size, size});
    //back
    generatePlane(&sides[1], {size, size, -size},    {size, -size, -size},
                             {-size, -size, -size,}, {-size, size, -size});
    //left
    generatePlane(&sides[2], {-size, size, -size},   {-size, -size, -size,}, 
                             {-size, -size, size},   {-size, size, size});
    //right
    generatePlane(&sides[3], {size, size, size},     {size, -size, size}, 
                             {size, -size, -size},   {size, size, -size});
    //top
    generatePlane(&sides[4], {-size, size, -size},   {-size, size, size}, 
                             {size, size, size},     {size, size, -size});
    //bottom
    generatePlane(&sides[5], {-size, -size, size},   {-size, -size, -size}, 
                             {size, -size, -size},   {size, -size, size});
#else // z up
    //front
    generatePlane(&sides[0], {-size, -size, size},    {-size,-size, -size},
                             {size, -size, -size},    {size, -size, size});
    //back
    generatePlane(&sides[1], {size, size, size},    {size, size, -size},
                             {-size, size, -size,}, {-size, size, size});
    //left
    generatePlane(&sides[2], {-size, size, size},   {-size, size, -size,}, 
                             {-size, -size, -size},   {-size, -size, size});
    //right
    generatePlane(&sides[3], {size, -size, size},     {size, -size, -size}, 
                             {size, size, -size},   {size, size, size});
    //top
    generatePlane(&sides[4], {-size, size, size},   {-size, -size, size}, 
                             {size, -size, size},     {size, size, size});
    //bottom
    generatePlane(&sides[5], {-size, -size, -size},   {-size, size, -size}, 
                             {size, size, -size},   {size, -size, -size});

#endif

    data->vertex_count = 4*6;
    data->vertices = (Vertex3D*)malloc(sizeof(Vertex3D)*4*6);
    for(int i = 0; i < 6; ++i){
        memcpy(&data->vertices[i*4], sides[i].vertices, sizeof(Vertex3D)*4);
    } 

    data->index_count = 6*6;
    data->indices = (uint32*)malloc(sizeof(uint32)*6*6);
    for(int i = 0, j = 0; i < 36; i += 6, j += 4){
        data->indices[i + 0] = 0 + j;
        data->indices[i + 1] = 1 + j;
        data->indices[i + 2] = 2 + j;
        data->indices[i + 3] = 0 + j;
        data->indices[i + 4] = 2 + j;
        data->indices[i + 5] = 3 + j;
    }
    data->mode = GL_TRIANGLES;
   
    for(int i = 0; i < 6; ++i){
        freeMeshData(&sides[i]);
    }
}

void generateSkybox(MeshData* data, float sz)
{
    MeshData sides[6];

    float size = sz/2.f;

    //front
    generatePlane(&sides[0], {-size, -size, size},    {-size,-size, -size},
                             {size, -size, -size},    {size, -size, size});
    //back
    generatePlane(&sides[1], {size, size, size},    {size, size, -size},
                             {-size, size, -size,}, {-size, size, size});
    //left
    generatePlane(&sides[2], {-size, size, size},   {-size, size, -size,}, 
                             {-size, -size, -size},   {-size, -size, size});
    //right
    generatePlane(&sides[3], {size, -size, size},     {size, -size, -size}, 
                             {size, size, -size},   {size, size, size});
    //top
    generatePlane(&sides[4], {-size, size, size},   {-size, -size, size}, 
                             {size, -size, size},     {size, size, size});
    //bottom
    generatePlane(&sides[5], {-size, -size, -size},   {-size, size, -size}, 
                             {size, size, -size},   {size, -size, -size});

    data->vertex_count = 4*6;
    data->vertices = (Vertex3D*)malloc(sizeof(Vertex3D)*4*6);
    for(int i = 0; i < 6; ++i){
        memcpy(&data->vertices[i*4], sides[i].vertices, sizeof(Vertex3D)*4);
    } 

    data->index_count = 6*6;
    data->indices = (uint32*)malloc(sizeof(uint32)*6*6);
    for(int i = 0, j = 0; i < 36; i += 6, j += 4){
        data->indices[i + 0] = 3 + j;
        data->indices[i + 1] = 2 + j;
        data->indices[i + 2] = 1 + j;
        data->indices[i + 3] = 3 + j;
        data->indices[i + 4] = 1 + j;
        data->indices[i + 5] = 0 + j;
    }
    data->mode = GL_TRIANGLES;
   
    for(int i = 0; i < 6; ++i){
        freeMeshData(&sides[i]);
    }

}

void generateFlatGrid(MeshData* data, float sizex, float sizez, int numx, int numz)
{
    int vertex_count = (numx + numz + 2)*2;
    data->vertices = (Vertex3D*)malloc(sizeof(Vertex3D)*vertex_count);
    int v_index = 0;

    for(int i = 0; i <= numz; ++i){
        data->vertices[v_index++].pos= {0 - sizex/2, 0, i*sizez/numz - sizez/2};
        data->vertices[v_index++].pos= {sizex - sizex/2, 0, i*sizez/numz - sizez/2};
    }
    for(int i = 0; i <= numz; ++i){
        data->vertices[v_index++].pos = {i*sizex/numx - sizex/2, 0, 0 - sizez/2};
        data->vertices[v_index++].pos = {i*sizex/numx - sizex/2, 0, sizez - sizez/2};
    }
    data->vertex_count = vertex_count;

    data->indices = (uint32*)malloc(sizeof(uint32)*vertex_count);
    data->index_count = vertex_count;
    for(int i = 0; i < vertex_count; ++i){
        data->indices[i] = i;
    }
    data->mode = GL_LINES;
}

#include <random>

static float cosinterp(float a, float b, float blend)
{
    float theta = blend * PI;
    float f = (1.0f - cosf(theta)) * 0.5f;
    return a * (1.0f - f) + b*f;
}

static float genTerrainHeight(int x, int z)
{
    float amplitude = 20.0f;
    std::mt19937 rng(x * 6969 + z * 420420 + 6969420247); 
    float h = (float(rng())/rng.max() * 2.0f - 1.0f + 0.4)*amplitude;
    return h;
}

static float getSmoothTerrainHeight(int x, int z)
{
    float corners = genTerrainHeight(x - 1, z - 1) +
                    genTerrainHeight(x + 1, z - 1) +
                    genTerrainHeight(x - 1, z + 1) +
                    genTerrainHeight(x + 1, z + 1);
    corners /= 16.0f;

    float sides = genTerrainHeight(x - 1, z    ) +
                  genTerrainHeight(x + 1, z    ) +
                  genTerrainHeight(x    , z - 1) +
                  genTerrainHeight(x    , z + 1);
    sides /= 8.0f;

    float center = genTerrainHeight(x, z) / 4;
    return corners + sides + center;
}

static float getInterpNoise(float x, float z)
{
    int int_x = (int)x;
    int int_z = (int)z;
    float frac_x = x - int_x;
    float frac_z = z - int_z;

    float v1 = getSmoothTerrainHeight(int_x, int_z);
    float v2 = getSmoothTerrainHeight(int_x + 1, int_z);
    float v3 = getSmoothTerrainHeight(int_x, int_z + 1);
    float v4 = getSmoothTerrainHeight(int_x + 1, int_z + 1);

    float i1 = cosinterp(v1, v2, frac_x);
    float i2 = cosinterp(v3, v4, frac_x);
    return cosinterp(i1, i2, frac_z);
}

void generateTerrain(MeshData* data, float sizex, float sizez, int numx, int numz)
{
    float h_sizex = sizex/2;
    float h_sizez = sizez/2;
    int vertex_count = (numx + 1)*(numz + 1);
    data->vertices = (Vertex3D*)malloc(sizeof(Vertex3D)*vertex_count);
    int v_index = 0;

    Vertex3D* vert = data->vertices;

    for(int i = 0; i <= numz; ++i){
        for(int j = 0; j <= numx; ++j){
            int mid_x = numx/2;
            int mid_z = numz/2;

            vec3 pos = { ((float(i)/(numx-1))*2-1)*h_sizex, 
                ((float(j)/(numz-1))*2-1)*h_sizez, getInterpNoise(i/6.0f, j/6.0f)};

            if(pos.x > -14.0f && pos.y < 14.0f && pos.x < 14.0f && pos.y > -14.0f)
                pos.z = -0.f;

            vert->pos = pos;
            vert->uv = {float(j)/numx, float(i)/numz};
            //vert->uv *= 10.0f;
            ++vert;
            //data->vertices[v_index++].pos = pos;
        }
    }
    data->vertex_count = vertex_count;

    int index_count = numx*numz*6;
    data->index_count = index_count;
    data->indices = (uint32*)malloc(sizeof(uint32)*index_count);
    uint32* id = data->indices;
    for(int i = 0; i < numx; ++i){
        for(int j = 0; j < numz; ++j){
            int i0 = i * (numx+1) + j;
            int i1 = i0 + 1;
            int i2 = i0 + (numx+1);
            int i3 = i2 + 1;
            if ((j+i)%2) {
                *id++ = i0; *id++ = i2; *id++ = i1;
                *id++ = i1; *id++ = i2; *id++ = i3;
            } else {
                *id++ = i0; *id++ = i2; *id++ = i3;
                *id++ = i0; *id++ = i3; *id++ = i1;
            }
        }
    }
    data->mode = GL_TRIANGLES;
}


void generateFlatTerrain(MeshData* data, float sizex, float sizey, int numx, int numy)
{
    int vertex_count = (numx + numy + 2)*2;
    data->vertices = (Vertex3D*)malloc(sizeof(Vertex3D)*vertex_count);
}

void generateCone(MeshData* data, float radius, float height, int ctriangles)
{
    data->mode = GL_TRIANGLES;
    data->vertex_count = 3*ctriangles + 4;
    data->index_count = ctriangles*3 + ctriangles*3;

    data->vertices = (Vertex3D*)malloc(sizeof(Vertex3D)*data->vertex_count);
    data->indices = (uint32*)malloc(sizeof(uint32)*data->index_count);

    int vcnt = 0;
    int icnt = 0;

    float halfh = height/2.f;
    float degree_step = 2*PI/ctriangles;
    float degrees = 0.f;

    uint32* ind = data->indices;
    Vertex3D* vert = data->vertices;
    //vert->pos = {0.f, 0.f, 0.f};//center
    vert->pos = {0.f, 0.f, -halfh};//center
    vert->normal = {0.f, 0.f, -1.f};
    vert->uv = {0.5f, 0.5f};
    ++vert;

    ++vcnt;

    //bottom
    for(int i = 0; i <= ctriangles; ++i){
        //vert->pos = {cos(degrees)*radius, sin(degrees)*radius, 0.f};
        vert->pos = {cos(degrees)*radius, sin(degrees)*radius, -halfh};
        vert->normal = {0.f, 0.f, -1.f};
        vert->uv = vert->pos.xy/2.f + vec2{0.5f, 0.5f};
        degrees += degree_step;
        ++vert;

        ++vcnt;
    }


    for(int i = 1; i <= ctriangles; ++i){
        *ind = i+1;
        ++ind;

        *ind = i;
        ++ind;

        *ind = 0;
        ++ind;

        icnt += 3;
    }


    int indoffset = ctriangles + 2;
    //bottom/side verts
    degrees = 0;
    for(int i = 0; i <= ctriangles; ++i){
        vert->pos = {cos(degrees)*radius, sin(degrees)*radius, -halfh};
        vert->normal = normalize(vec3{vert->pos.x, vert->pos.y, radius}); //B - A = pos - {0, 0, 0} = pos
        //vert->uv = {1.f/ctriangles*i, 1.f};
        vert->uv = vert->pos.xy/2.f + vec2{0.5f, 0.5f};
        degrees += degree_step;
        ++vert;

        ++vcnt;
    }

    //top/side verts
    degrees = 0;
    for(int i = 0; i <= ctriangles; ++i){
        vert->pos = vec3{0, 0, halfh};
        vert->normal = normalize(vec3{cos(degrees+degree_step/2.f)*radius, sin(degrees+degree_step/2.f)*radius, radius});
        //vert->normal = normalize(vec3{vert->pos.x, vert->pos.y, height}); //B - A = pos - {0, 0, 0} = pos
        //vert->uv = {1.f/ctriangles*i, 0.f};
        vert->uv = {0.5f, 0.5f};
        degrees += degree_step;
        ++vert;

        ++vcnt;
    }

    int c2off = ctriangles + 1;
    for(int i = indoffset; i < indoffset + ctriangles; ++i){
        *ind = i;
        ++ind;

        *ind = i+1;
        ++ind;

        *ind = c2off + i;
        ++ind;

        icnt += 3;
    }

    assert(icnt == data->index_count);
    assert(vcnt == data->vertex_count);
    calcTangents(data);
}

void generateCylinder(MeshData* data, float radius, float height, int ctriangles)
{
    data->mode = GL_TRIANGLES;
    data->vertex_count = 4*ctriangles + 6;
    data->index_count = ctriangles*3*2 + ctriangles*6;

    data->vertices = (Vertex3D*)malloc(sizeof(Vertex3D)*data->vertex_count);
    data->indices = (uint32*)malloc(sizeof(uint32)*data->index_count);

    float halfh = height/2.f;
    float degree_step = 2*PI/ctriangles;
    float degrees = 0.f;

    uint32* ind = data->indices;
    Vertex3D* vert = data->vertices;
    //vert->pos = {0.f, 0.f, 0.f};//center
    vert->pos = {0.f, 0.f, -halfh};//center
    vert->normal = {0.f, 0.f, -1.f};
    vert->uv = {0.5f, 0.5f};
    ++vert;

    //bottom
    for(int i = 0; i <= ctriangles; ++i){
        //vert->pos = {cos(degrees)*radius, sin(degrees)*radius, 0.f};
        vert->pos = {cos(degrees)*radius, sin(degrees)*radius, -halfh};
        vert->normal = {0.f, 0.f, -1.f};
        vert->uv = vert->pos.xy/2.f + vec2{0.5f, 0.5f};
        degrees += degree_step;
        ++vert;
    }


    for(int i = 1; i <= ctriangles; ++i){
        *ind = i+1;
        ++ind;

        *ind = i;
        ++ind;

        *ind = 0;
        ++ind;
    }


    //top
    int indoffset = ctriangles + 2;
    //vert->pos = {0.f, 0.f, height};//center
    vert->pos = {0.f, 0.f, halfh};//center
    vert->normal = {0.f, 0.f, 1.f};
    vert->uv = {0.5f, 0.5f};
    ++vert;

    degrees = 0;
    for(int i = 0; i <= ctriangles; ++i){
        //vert->pos = {cos(degrees)*radius, sin(degrees)*radius, height};
        vert->pos = {cos(degrees)*radius, sin(degrees)*radius, halfh};
        vert->normal = {0.f, 0.f, 1.f};
        vert->uv = vert->pos.xy/2.f + vec2{0.5f, 0.5f};
        degrees += degree_step;
        ++vert;
    }

    for(int i = indoffset + 1; i <= indoffset + ctriangles; ++i){
        *ind = indoffset;
        ++ind;

        *ind = i;
        ++ind;

        *ind = i+1;
        ++ind;
    }

    indoffset *= 2; //+= ctriangles + 2
    //bottom/side verts
    degrees = 0;
    for(int i = 0; i <= ctriangles; ++i){
        //vert->pos = {cos(degrees)*radius, sin(degrees)*radius, 0.f};
        vert->pos = {cos(degrees)*radius, sin(degrees)*radius, -halfh};
        //vert->pos += {1.1f, 0, 0};
        vert->normal = normalize(vec3{vert->pos.x, vert->pos.y, 0}); //B - A = pos - {0, 0, 0} = pos
        //vert->normal = {cos(degrees)*radius, sin(degrees)*radius, -halfh};
        vert->uv = {1.f/ctriangles*i, 1.f};
        degrees += degree_step;
        ++vert;
    }

    //top/side verts
    degrees = 0;
    for(int i = 0; i <= ctriangles; ++i){
        //vert->pos = {cos(degrees)*radius, sin(degrees)*radius, height};
        vert->pos = {cos(degrees)*radius, sin(degrees)*radius, halfh};
        //vert->pos += {1.1f, 0, 0};
        //vert->normal = normalize(vert->pos - vec3{0.f, 0.f, height}); //pos.xy, 0 ?
        vert->normal = normalize(vec3{vert->pos.x, vert->pos.y, 0}); //B - A = pos - {0, 0, 0} = pos
        vert->uv = {1.f/ctriangles*i, 0.f};
        degrees += degree_step;
        ++vert;
    }

    int c2off = ctriangles + 1;
    for(int i = indoffset; i < indoffset + ctriangles; ++i){
        *ind = i;
        ++ind;

        *ind = i+1;
        ++ind;

        *ind = c2off + i;
        ++ind;

        *ind = c2off + i;
        ++ind;

        *ind = i+1;
        ++ind;

        *ind = c2off + i + 1;
        ++ind;
    }
    calcTangents(data);
}


static vec3 sphericalToCartesian(float radius, float inclination, float azimuth)
{
    vec3 result;
    result.x = radius*sinf(inclination)*cosf(azimuth);
    result.y = radius*sinf(inclination)*sinf(azimuth);
    result.z = radius*cosf(inclination);
    return result;
}

void generateSphere(MeshData* data, int slices, int rings, float radius)
{

}

void generateAxisVectors(MeshData* data, float len)
{
    data->mode = GL_LINES;     
    data->vertex_count = 4;
    data->index_count = 6;
    
    data->vertices = (Vertex3D*)malloc(sizeof(Vertex3D)*data->vertex_count);
    data->indices = (uint32*)malloc(sizeof(uint32)*data->index_count);

    data->vertices[0].pos = {0, 0, 0};
    data->vertices[1].pos = {len, 0, 0};
    data->vertices[2].pos = {0, len, 0};
    data->vertices[3].pos = {0, 0, len};

    data->indices[0] = 0;
    data->indices[1] = 1;
    data->indices[2] = 0;
    data->indices[3] = 2;
    data->indices[4] = 0;
    data->indices[5] = 3;
}

void generateQuad(MeshData2D* data, float size)
{
    data->mode = GL_TRIANGLES;
    data->vertex_count = 4;
    data->index_count = 6;

    data->vertices = (Vertex2D*)malloc(sizeof(Vertex2D)*data->vertex_count);
    data->indices = (uint32*)malloc(sizeof(uint32)*data->index_count);

    data->vertices[0].pos = {-size, size};
    data->vertices[1].pos = {-size, -size};
    data->vertices[2].pos = {size, -size};
    data->vertices[3].pos = {size, size};

    data->vertices[0].uv = {0, size};
    data->vertices[1].uv = {0, 0};
    data->vertices[2].uv = {size, 0};
    data->vertices[3].uv = {size, size};

    data->indices[0] = 0;
    data->indices[1] = 1;
    data->indices[2] = 2;
    data->indices[3] = 0;
    data->indices[4] = 2;
    data->indices[5] = 3;
}

void freeMeshData(MeshData2D *data)
{
    free(data->vertices);
    free(data->indices);
}

void freeMeshData(MeshData* data)
{
    free(data->vertices);
    free(data->indices);
}

//unstable file format for models/meshes
void loadFromFile(MeshData* data, const char* file_name)
{
    FILE* file = fopen(file_name, "rb");
    //assert(file && "TODO: implement proper error handling");
    if(!file)
        return;
    fread(&data->mode, sizeof(data->mode), 1, file);
    fread(&data->vertex_count, sizeof(data->vertex_count), 1, file);
    data->vertices = (Vertex3D*)malloc(sizeof(Vertex3D)*data->vertex_count);
    fread(data->vertices, sizeof(Vertex3D), data->vertex_count, file);

    fread(&data->index_count, sizeof(data->index_count), 1, file);
    data->indices = (uint32*)malloc(sizeof(uint32)*data->index_count);
    fread(data->indices, sizeof(uint32), data->index_count, file);
    
    fclose(file);
}

void writeToFile(const MeshData* data, const char* file_name)
{
    FILE* file = fopen(file_name, "wb");
    assert(file && "TODO: implement proper error handling");

    fwrite(&data->mode, sizeof(data->mode), 1, file);
    fwrite(&data->vertex_count, sizeof(data->vertex_count), 1, file);
    fwrite(data->vertices, sizeof(Vertex3D), data->vertex_count, file);
    fwrite(&data->index_count, sizeof(data->index_count), 1, file);
    fwrite(data->indices, sizeof(uint32), data->index_count, file);

    fclose(file);
}

int loadMaterial(Material* material, const char* albedo_map, const char* normal_map,
    const char* roughness_map, const char* metalic_map, bool metalic)
{
    *material = {0};
    material->albedo_map = texture2DLoad(albedo_map);
    material->normal_map = texture2DLoad(normal_map, 0b101);
    material->roughness_map = texture2DLoad(roughness_map, 0b101);

    if(metalic_map)
        material->metalic_map = texture2DLoad(metalic_map, 0b101);
    else
        material->metalic_map = metalic ? texture2DLoadWhiteTexture() : texture2DLoadBlackTexture();

    return 0;
}

void loadMaterial(Material* material, vec3 albedo, float roughness, float metalic)
{
    *material = {0};
    material->albedo = albedo;
    material->roughness = roughness;
    material->metalic = metalic;
}
