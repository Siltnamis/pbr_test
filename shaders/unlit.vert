#version 430 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTCoord;

out vec2 t_coord;

uniform mat4 mvp_mat;
uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;

void main()
{
    t_coord = vTCoord;
    gl_Position = mvp_mat * vec4(vPosition.xyz, 1);
}
