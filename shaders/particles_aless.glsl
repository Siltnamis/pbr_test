#VERTEX
#version 430 core
layout(location = 0) in vec3    p_position;
layout(location = 1) in float   rotation;
layout(location = 2) in vec2    scale;
layout(location = 3) in vec2    v_uv1;
layout(location = 4) in vec2    v_uv2;
layout(location = 5) in float   v_blend;

out vec2    uv1;
out vec2    uv2;
out float   blend;

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

uniform mat4 view_mat;
uniform mat4 proj_mat;
uniform float atlass_inv_rows; // 1/num_rows
uniform mat4 model_mat;

void main()
{
    float frametime = 1.0/10.0;
    vec2 uv;
    int v_id = gl_VertexID%4;
    uv = uv_data[v_id];
    uv1 = uv*atlass_inv_rows + v_uv1;
    uv2 = uv*atlass_inv_rows + v_uv2;
	blend = v_blend;
    
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

#FRAGMENT
#version 430 core

in vec2    uv1;
in vec2    uv2;
in float   blend;

out vec4 out_color;

uniform sampler2D textureMap;

void main()
{

    vec4 c1 = texture(textureMap, uv1);
    vec4 c2 = texture(textureMap, uv2);
    //out_color = vec4(1, 0, 0, 1);
    //out_color = gs_in.color * c.r + vec4(0.1, 0.9, 0.2, 0);
    out_color =  mix(c1, c2, blend)*vec4(1, 1, 1, 1);

    //out_color = vec4(1);
    //out_color = vec4(1, 1, 1, 1) * texture2D(textureMap, fs_in.uv1);
}

