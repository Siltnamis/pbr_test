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

layout(binding = 0, std140) uniform CommonVars
{
    mat4    proj_mat;
    mat4    view_mat;
    vec3    cam_pos;
    float   time;
};

uniform mat4 model_mat;

uniform mat4 light_space_mat;

uniform vec3 light_pos[MAX_LIGHTS];
uniform int  light_count;

void main()
{
    const mat4 mvp_mat = proj_mat*view_mat*model_mat;
    vec3 v_bitangent = cross(v_normal, v_tangent);
    vec3 T = normalize(vec3(model_mat * vec4(v_tangent, 0.0)));
    vec3 B = normalize(vec3(model_mat * vec4(v_bitangent, 0.0)));
    vec3 N = normalize(vec3(model_mat * vec4(v_normal, 0.0)));
    tbn_mat = mat3(T, B, N);
    uv = v_uv;// * length(cam_pos - model_mat[3].xyz);
    //uv = v_uv*2.0;
    //uv = v_uv * length(cam_pos - model_mat[3].xyz);
    // //normal = (model_mat * vec4(vNormal, 0)).xyz;
    face_normal = mat3(transpose(inverse(model_mat)))*v_normal.xyz;
    to_camera = cam_pos - (model_mat * vec4(v_position, 1)).xyz;

    vec3 vpos = (model_mat*vec4(v_position, 1)).xyz;
    frag_pos_light_space = light_space_mat*vec4(vpos, 1.0);

    for(int i = 0; i < light_count; ++i)
        to_lights[i] = light_pos[i] - vpos;

    gl_Position = mvp_mat * vec4(v_position, 1);
    //float vid = float(gl_VertexID%6);
    //gl_Position = mvp_mat * vec4(v_position+sin(vid*time/5.0)/10.0, 1);
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

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 bright_color;
layout(location = 2) out vec4 lscattering;

layout(binding = 0) uniform sampler2D albedo_map;
layout(binding = 1) uniform sampler2D normal_map;
layout(binding = 2) uniform sampler2D roughness_map;
layout(binding = 3) uniform sampler2D metalic_map;
layout(binding = 4) uniform sampler2D shadow_map;
// layout(binding = 4) uniform sampler2DShadow shadow_map2;

uniform vec3 light_colors[MAX_LIGHTS];
uniform int  light_count;

vec3 ambient = vec3(0.05, 0.05, 0.05);
float specular_str = 0.5;

//a = roughness  
float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}
//k = (a+1)^2 /8   for direct light
//k = (a^2)/2  for image based lightining
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// float shadowCalc2(vec4 frag_lspace, vec3 normal)
// {
//     return textureProj(shadow_map2, frag_lspace);
// }

float shadowCalc(vec4 frag_lspace, vec3 normal)
{
    vec2 uv_offsets[16] = vec2[]( 
		vec2( -0.94201624, -0.39906216 ), 
		vec2( +0.94558609, -0.76890725 ), 
		vec2( -0.09418410, -0.92938870 ), 
		vec2( +0.34495938, +0.29387760 ), 
		vec2( -0.91588581, +0.45771432 ), 
		vec2( -0.81544232, -0.87912464 ), 
		vec2( -0.38277543, +0.27676845 ), 
		vec2( +0.97484398, +0.75648379 ), 
		vec2( +0.44323325, -0.97511554 ), 
		vec2( +0.53742981, -0.47373420 ), 
		vec2( -0.26496911, -0.41893023 ), 
		vec2( +0.79197514, +0.19090188 ), 
		vec2( -0.24188840, +0.99706507 ), 
		vec2( -0.81409955, +0.91437590 ), 
		vec2( +0.19984126, +0.78641367 ), 
		vec2( +0.14383161, -0.14100790 ) 
	);

    float shadow = 1.0;
    vec3 proj_coords = frag_lspace.xyz/frag_lspace.w;
    proj_coords = proj_coords*0.5 + 0.5;
    for(int i = 0; i < 16; ++i)
    {
        if(texture(shadow_map, proj_coords.xy + uv_offsets[i]/500.0).r < proj_coords.z - 0.009f)
            shadow -= 0.06;
    }

    //float closest_depth = texture(shadow_map, proj_coords.xy).r;
    //float current_depth = proj_coords.z;
    //float bias = max(0.05 * (1.0 - dot(normal, vec3(0.5, -5.0, 5.0))), 0.005);
    //bias = 0.002;
    //float bias = 0.005*tan(acos(dot(normal, vec3(0.5, -5.0, 5.0))));
    //bias = clamp(bias, 0.0, 0.001);
    //shadow = current_depth - bias < closest_depth ? 1.0 : 0.1;
    //shadow = closest_depth < current_depth - bias ? 0.1 : 1.0;

    //return 1.0;
    return shadow;
}

void main()
{
    //uv *= length(to_camera);
	vec3 N = texture(normal_map, vec2(uv.x*1.0, uv.y*1.0)).rgb;
    N.y = 1.0 - N.y; //flip y because of data. TODO: decide on normal_map format
    N = normalize(N * 2.0 - 1.0);
    N = normalize(tbn_mat * N);
    vec3 NF = normalize(face_normal);

    float mix_factor = 1.0/length(to_camera);
    //N = mix(N, NF, mix_factor);
    //N = normalize(N)

    vec3 V = normalize(to_camera);
    vec3 albedo = vec3(texture(albedo_map, uv));

    float metalic = texture(metalic_map, uv).r;
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metalic);

    vec3 Lo = vec3(0.0);

    //loop through all lights for that
    //light radiance
    for(int i = 0; i < light_count; i++)
    {
        vec3 L = normalize(to_lights[i]);
        vec3 H = normalize(V + L);

        float distance = length(to_lights[i]);
        float attenuation = 1.0/(distance*distance);
        if ( i == 0)
            attenuation = 1.0;
        vec3 radiance = light_colors[i] * attenuation;

        float roughness = length(texture(roughness_map, uv));
        roughness = (texture(roughness_map, uv)).r;
        //cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metalic;

        vec3 nominator = NDF * G * F;
        float denominator = 4.0*max(dot(N, V), 0.0)*max(dot(N, L), 0.0) + 0.001;
        vec3 specular = nominator/denominator;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD*albedo/PI + specular)*radiance*NdotL * shadowCalc(frag_pos_light_space, N);
        //Lo += (kD*albedo/PI + specular)*radiance*NdotL;
    }

    float ao = 1.0;
    vec3 ambient = vec3(0.03)*albedo*ao;
    color = vec4(ambient + Lo, 1.0);
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722))+0.0;
    if(brightness > 1.0)
        bright_color = vec4(color.rgb, 1.0);
    else bright_color = vec4(vec3(0.0), 1.0);
    lscattering = vec4(0.0, 0.0, 0.0, 1.0);
        // bright_color = vec4(, 1.0);
    // bright_color = vec4(color.rgb, 1.0);
        // bright_color = vec4(1.0, 0.0, 1.0, 1.0);
}