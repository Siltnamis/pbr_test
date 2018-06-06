#VERTEX
#version 430 core

layout(location = 0) in vec3 vPosition;

layout(binding = 0, std140) uniform CommonVars
{
    mat4    proj_mat;
    mat4    view_mat;
    vec3    cam_pos;
    float   time;
};

out vec3 t_coord;
void main()
{
    t_coord = vPosition.xzy;
    t_coord.z = -t_coord.z;
    mat4 mvp_mat = proj_mat * view_mat;
    gl_Position = mvp_mat * vec4(vPosition*1000.0, 1.0);
}

#FRAGMENT
#version 430 core

in vec3 t_coord;
layout(location = 0) out vec4 color;
layout(location = 1) out vec4 bright_color;
//use skybox brightest colors as sun for light scattering
//it might limit some skyboxes (if it has multiple bright spots)
layout(location = 2) out vec4 sun_color; 

uniform samplerCube cube_map;
const float levels = 10.0;
void main()
{
    vec4 tmp_color = texture(cube_map, vec3(t_coord.x, t_coord.y, t_coord.z));
    //float amaunt = (tmp_color.r + tmp_color.g + tmp_color.b)/3.0;
    //amaunt = floor(amaunt * levels)/levels;
    color = texture(cube_map, vec3(t_coord.x, t_coord.y, t_coord.z));
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722))+0.015;
    if(brightness > 1.0)
        bright_color = vec4(color.rgb, 1.0);
    else bright_color = vec4(vec3(0.0), 1.0);
    sun_color = bright_color;
    //color = vec4(1, 0, 1, 1);
}