#VERTEX
#version 430 core

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in vec3 v_tangent;

layout(binding = 0, std140) uniform CommonVars
{
    mat4    proj_mat;
    mat4    view_mat;
    vec3    cam_pos;
    float   time;
};
uniform mat4 model_mat;

out vec3 reflected;
out vec3 refracted;
out vec3 face_normal;
out float refractive_factor;

void main()
{
    const mat4 mvp_mat = proj_mat*view_mat*model_mat;
	vec3 world_position = (model_mat*vec4(v_position, 1.0)).xyz;
    gl_Position = proj_mat*view_mat*vec4(world_position, 1.0);

	face_normal = v_normal;
	vec3 N = normalize(v_normal);
	vec3 view_vector = normalize(world_position - cam_pos);
	reflected = reflect(view_vector, N);
	reflected = reflected.xzy;
	reflected.z = -reflected.z;
	refracted = refract(view_vector, N, 1.0/1.33);
	refracted = refracted.xzy;
	refracted.z = -refracted.z;

	refractive_factor = abs(pow(dot(view_vector, N), 2.0));
	refractive_factor = abs(pow(dot(N, view_vector), 3.0));
	//refractive_factor = dot(N, view_vector) + 0.0;
}

#FRAGMENT
#version 430 core

in vec3 reflected;
in vec3 refracted;
in vec3 face_normal;
in float refractive_factor;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 bright_color;

uniform samplerCube cube_map;
const float levels = 10.0;
void main()
{
	vec4 reflected_color = texture(cube_map, reflected);
	vec4 refracted_color = texture(cube_map, refracted);
	color = mix(reflected_color, refracted_color, refractive_factor);
	float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722))+0.0;
    if(brightness > 1.0)
        bright_color = vec4(color.rgb, 1.0);
    else bright_color = vec4(vec3(0.0), 1.0);
}