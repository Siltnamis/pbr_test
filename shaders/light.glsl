#VERTEX
#version 430 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in vec3 v_tangent;

out vec2 uv;

layout(binding = 0, std140) uniform CommonVars
{
    mat4    proj_mat;
    mat4    view_mat;
    vec3    cam_pos;
    float   time;
};

uniform mat4 model_mat;

void main()
{
    const mat4 mvp_mat = proj_mat*view_mat*model_mat;
    uv = v_uv;
    gl_Position = mvp_mat*vec4(v_position, 1);
}

#FRAGMENT
#version 430 core

in vec2 uv;
layout(location = 0) out vec4 o_color;
layout(location = 1) out vec4 b_color;

uniform vec3 color;

void main()
{
    o_color = vec4(color, 1.0);
    b_color = vec4(color, 1.0);
}