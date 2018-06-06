#VERTEX
#version 430 core

#define MAX_LIGHTS 5

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in vec3 v_tangent;

out vec2 uv;
out vec3 face_normal;
out vec3 to_camera;
out vec4 frag_pos_light_space;
out mat3 tbn_mat;

out vec3 to_lights[MAX_LIGHTS];

uniform mat4 mvp_mat;
uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;

void main()
{
    vec3 v_bitangent = cross(v_normal, v_tangent);
    vec3 T = normalize(vec3(model_mat * vec4(v_tangent, 0.0)));
    vec3 B = normalize(vec3(model_mat * vec4(v_bitangent, 0.0)));
    vec3 N = normalize(vec3(model_mat * vec4(v_normal, 0.0)));
    tbn_mat = mat3(T, B, N);
    uv = v_uv;// * length(cam_pos - model_mat[3].xyz);
    gl_Position = mvp_mat * vec4(v_position, 1);
}

#FRAGMENT
#version 430 core

#define MAX_LIGHTS 5
#define PI 3.14159265358979323846264338327950288

in vec2 uv;
in vec3 face_normal;
in vec3 to_camera;
in vec4 frag_pos_light_space;
in mat3 tbn_mat;

in vec3 to_lights[MAX_LIGHTS];

out vec4 color;

uniform sampler2D t1_map;
uniform sampler2D t2_map;

uniform vec3 light_colors[MAX_LIGHTS];
uniform int  light_count;

void main()
{
    vec3 t1 = texture(t1_map, uv).rgb;
    vec3 t2 = texture(t2_map, uv).rgb;
    //vec3 col = t1 * t2;
    vec3 col = t1/(1.0-t2);
    color = vec4(col, 1.0);
}