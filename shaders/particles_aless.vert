#version 430 core

layout(location = 0) in vec3    p_position;
layout(location = 1) in float   rotation;
layout(location = 2) in vec2    scale;
layout(location = 3) in vec2    uv1;
layout(location = 4) in vec2    uv2;
layout(location = 5) in float   blend;

out VS_OUT
{
    vec2    uv1;
    vec2    uv2;
    float   blend;
}vs_out;

vec2 pos_data[4] = vec2[]
(
    vec2(-1, -1),
    vec2(1, -1),
    vec2(-1, 1),
    vec2(1, 1)
);

vec2 uv_data[4] = vec2[]
(
    vec2(0, 1),
    vec2(1, 1),
    vec2(0, 0),
    vec2(1, 0)
);

layout(std140, binding = 0) uniform mats{
    mat4 view_mat;
    mat4 proj_mat;
};

#if 0
//asume max 16k max size
//8*4 = 32 bytes per particle
//16384 / 32 = 512 max particles
#define MAX_PARTICLES 512
layout(std140, binding = 2) uniform particles{
    vec4 positions[MAX_PARTICLES];
    //float rotations[MAX_PARTICLES];
    vec2 scales[MAX_PARTICLES];
    vec2 uvs[MAX_PARTICLES];
};
#endif

uniform vec2 texture_info; //x = 1/num_rows y = blend betweeen 2 textures
uniform mat4 model_mat;

void main()
{
    float frametime = 1.0/10.0;
    vec2 uv;
    int v_id = gl_VertexID%4;
    uv = uv_data[v_id];
    vs_out.uv1 = uv*texture_info.x + uv1;
    vs_out.uv2 = uv*texture_info.x + uv2;
    
    //int particle_id = gl_VertexID/6;
    vec2 pos = vec2(0.15, 0.15)*scale*pos_data[v_id];
    mat4 model_mat2 = mat4(1);
    model_mat2 = model_mat;
    mat4 view_model_mat = view_mat*model_mat;
#if 1
    model_mat2[0][0] = view_mat[0][0];
    model_mat2[0][1] = view_mat[1][0];
    model_mat2[0][2] = view_mat[2][0];

    model_mat2[1][0] = view_mat[0][1];
    model_mat2[1][1] = view_mat[1][1];
    model_mat2[1][2] = view_mat[2][1];

    model_mat2[2][0] = view_mat[0][2];
    model_mat2[2][1] = view_mat[1][2];
    model_mat2[2][2] = view_mat[2][2];
#else
    model_mat2[0][0] = view_model_mat[0][0];
    model_mat2[0][1] = view_model_mat[1][0];
    model_mat2[0][2] = view_model_mat[2][0];

    model_mat2[1][0] = view_model_mat[0][1];
    model_mat2[1][1] = view_model_mat[1][1];
    model_mat2[1][2] = view_model_mat[2][1];

    model_mat2[2][0] = view_model_mat[0][2];
    model_mat2[2][1] = view_model_mat[1][2];
    model_mat2[2][2] = view_model_mat[2][2];
#endif
    model_mat2[3].xyz = (model_mat*vec4(p_position, 1)).xyz;
    //model_mat2[3].z += 0.3;
    //model_mat2[3].xyz = p_position;
    //model_mat2[3].w = 1;
    gl_Position = proj_mat*view_mat*model_mat2*vec4(pos, 0, 1);
}
