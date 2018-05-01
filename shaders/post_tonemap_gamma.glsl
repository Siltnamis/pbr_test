#VERTEX
#version 430 core

out vec2 uv;
vec2 v_positions[4] = vec2[]
(
    vec2(-1, -1),
    vec2(1, -1),
    vec2(-1, 1),
    vec2(1, 1)
);

vec2 v_uvs[4] = vec2[]
(
    vec2(0, 0),
    vec2(1, 0),
    vec2(0, 1),
    vec2(1, 1)
);

void main()
{
    uv = v_uvs[gl_VertexID];
    vec2 pos = v_positions[gl_VertexID];
    gl_Position = vec4(pos, 0, 1);
}

#FRAGMENT
#version 430 core

in vec2 uv;
layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D textureMap;
layout(binding = 1) uniform sampler2D bloom_map;
layout(binding = 2) uniform sampler2D light_map; //for light scattering
uniform vec3 position;
const float exposure = 1.5;
const float gamma = 2.2;
uniform bool use_bloom;
uniform vec3 light_screen_space;

vec4 scattering()
{
    const int NUM_SAMPLES = 128;
    float exposure2 = 0.5;
    float decay = 1.0;
    float density = 0.99;
    float weight = 0.03;
    float illumination_decay = 0.4;

    // decay = 0.97815;
    // exposure2 = 0.92;
    // density = 0.966;
    // weight = 0.58767;
    // illumination_decay = 1.0;

    vec2 luv = uv;
    vec2 lsp = light_screen_space.xy;
    vec2 uv_delta = luv - lsp;
    uv_delta *= 1.0f/NUM_SAMPLES * density;
    vec3 color = texture(light_map, luv).rgb;
    for(int i = 0; i < NUM_SAMPLES; ++i)
    {
       luv -= uv_delta; 
       vec3 c2 = texture(light_map, luv).rgb;
       c2 *= illumination_decay * weight;
       color += c2;
       illumination_decay *= decay;
    }
    vec3 rcolor = texture(textureMap, uv).rgb;
    return vec4(color * exposure2, 1.0);
}

void main()
{
    if(false){
        vec4 color = texture(textureMap, uv);
        color.rgb = pow(color.rgb, vec3(1.0/gamma));
        out_color = color;
    }else if(false){
        vec3 hdrcolor = texture(textureMap, uv).rgb;
        vec3 mapped = hdrcolor / (hdrcolor + vec3(1.0));
        mapped = pow(mapped, vec3(1.0 / gamma));
        out_color = vec4(mapped, 1);
    }else if(true){
        vec3 hdrcolor = texture(textureMap, uv).rgb;
        // hdrcolor = texture(bloom_map, uv).rgb;
        //if(use_bloom)
        if(false)
        {
        vec3 bloom_color = texture(bloom_map, uv).rgb;
        hdrcolor += bloom_color*0.85 + scattering().rgb;
        }
        vec3 mapped = vec3(1) - exp(-hdrcolor * exposure);
        mapped = pow(mapped, vec3(1.0 / gamma));
        out_color = vec4(mapped, 1);
    }
}