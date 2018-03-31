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
out mat3 tbn_mat;

out vec3 to_lights[MAX_LIGHTS];

uniform mat4 mvp_mat;
uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;

uniform vec3 cam_pos;
uniform vec3 light_pos[MAX_LIGHTS];
uniform int  light_count;

void main()
{
    vec3 v_bitangent = cross(v_normal, v_tangent);
    vec3 T = normalize(vec3(model_mat * vec4(v_tangent, 0.0)));
    vec3 B = normalize(vec3(model_mat * vec4(v_bitangent, 0.0)));
    vec3 N = normalize(vec3(model_mat * vec4(v_normal, 0.0)));
    tbn_mat = mat3(T, B, N);

    uv = v_uv*2.0;
    // //normal = (model_mat * vec4(vNormal, 0)).xyz;
    face_normal = mat3(transpose(inverse(model_mat)))*v_normal.xyz;
    to_camera = cam_pos - (model_mat * vec4(v_position, 1)).xyz;

    vec3 vpos = (model_mat*vec4(v_position, 1)).xyz;

    for(int i = 0; i < light_count; ++i)
        to_lights[i] = light_pos[i] - vpos;

    gl_Position = mvp_mat * vec4(v_position, 1);
}

#FRAGMENT
#version 430 core
#define MAX_LIGHTS 5

#define PI 3.14159265358979323846264338327950288

in vec2 uv;
in vec3 face_normal;
in vec3 to_camera;
in mat3 tbn_mat;

in vec3 to_lights[MAX_LIGHTS];

layout(location = 0)out vec4 color;
layout(location = 1)out vec4 bright_color;

uniform sampler2D albedo_map;
uniform sampler2D normal_map;
uniform sampler2D roughness_map;
uniform sampler2D metalic_map;

uniform vec3 light_colors[MAX_LIGHTS];
uniform int  light_count;

vec3 ambient = vec3(0.001, 0.001, 0.001);
float specular_str = 0.5;

uniform float flicker_rate;
uniform float time;
uniform float intensity;

void main()
{
    vec4 lit_color;
    vec3 diffuse = vec3(0, 0, 0);
    vec3 specular = vec3(0, 0, 0);

    vec3 N = texture(normal_map, vec2(uv.x*1.0, uv.y*1.0)).rgb;
    N.y = 1.0 - N.y; //flip y because of data. TODO: decide on normal_map format
    N = normalize(N * 2.0 - 1.0);
    N = normalize(tbn_mat * N);
    //N = normalize(face_normal);

    vec3 to_camera = normalize(to_camera);
    for(int i = 0; i < light_count; ++i)
    {
        vec3 to_light = normalize(to_lights[i]);
        //vec3 lpos = light_pos[i];
        //to_light = normalize(lpos);
        float dist = length(to_lights[i]);
        float attenuation = 1.0/(dist*dist);
        if(i==0)
            attenuation = 0.8;

        diffuse += (max(dot(N, to_light), 0.01) * light_colors[i]) * attenuation;
        vec3 R = reflect(-to_light, N);
        float spec = (pow(max(dot(to_camera, R), 0.0), 32.0)) * attenuation;
        specular += specular_str*spec*light_colors[i];
    }
    vec4 effect_color = vec4(clamp(sin(2.0*PI*time*flicker_rate), 0.0, 1.0), 
            0, sin(2.0*PI*time*flicker_rate + PI), 1)*intensity;
    lit_color = vec4((ambient + diffuse + specular), 1)*effect_color;
    lit_color.a = 1.0;
    color = lit_color * texture(albedo_map, uv);
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722))+0.0;
    if(brightness > 1.0)
        bright_color = vec4(color.rgb, 1.0);
}
