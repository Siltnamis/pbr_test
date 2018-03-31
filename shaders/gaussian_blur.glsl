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
layout(location = 0) out vec4 color;

layout(binding = 0) uniform sampler2D textureMap;
uniform bool horizontal;
// float weight[] = float[](0.099654,0.096603,0.088001,0.075333,0.0606,0.04581,
// 0.032542,0.021724,0.013627,0.008033,0.00445,0.002316,0.001133);

// float weight[] = float[](0.4026, 0.2442, 0.0545);
float weight[] = float[](0.399, 0.242, 0.053, 0.004335, 0.000124);

void main()
{
	vec2 uv_offset = 1.0 / vec2(textureSize(textureMap, 0));
	vec3 result = texture(textureMap, uv).rgb * weight[0];
	if(horizontal)
	{
		for(int i = 1; i < weight.length(); ++i)
		{
			result += texture(textureMap, uv + vec2(uv_offset.x*float(i), 0.0)).rgb * weight[i];
			result += texture(textureMap, uv - vec2(uv_offset.x*float(i), 0.0)).rgb * weight[i];
		}
		color = vec4(result, 1.0);
	}
	else
	{
		for(int i = 1; i < weight.length(); ++i)
		{
			result += texture(textureMap, uv + vec2(0.0, uv_offset.y*float(i))).rgb * weight[i];
			result += texture(textureMap, uv - vec2(0.0, uv_offset.y*float(i))).rgb * weight[i];
		}
		color = vec4(result, 1.0);
	}
}


// #VERTEX
// #version 430 core

// out vec2 uv;
// vec2 v_positions[4] = vec2[]
// (
//     vec2(-1, -1),
//     vec2(1, -1),
//     vec2(-1, 1),
//     vec2(1, 1)
// );

// vec2 v_uvs[4] = vec2[]
// (
//     vec2(0, 0),
//     vec2(1, 0),
//     vec2(0, 1),
//     vec2(1, 1)
// );

// void main()
// {
//     uv = v_uvs[gl_VertexID];
//     vec2 pos = v_positions[gl_VertexID];
//     gl_Position = vec4(pos, 0, 1);
// }

// #FRAGMENT
// #version 430 core

// in vec2 uv;
// layout(location = 2) out vec4 color;

// layout(binding = 0) uniform sampler2D texture_map;

// const float weights[] = float[](
// 	0.0024499299678342, 0.0043538453346397,0.0073599963704157, 0.0118349786570722,
// 	0.0181026699707781, 0.0263392293891488, 0.0364543006660986, 0.0479932050577658,
// 	0.0601029809166942, 0.0715974486241365, 0.0811305381519717, 0.0874493212267511,
// 	0.0896631113333857, 0.0874493212267511, 0.0811305381519717, 0.0715974486241365,
// 	0.0601029809166942, 0.0479932050577658, 0.0364543006660986, 0.0263392293891488,
// 	0.0181026699707781, 0.0118349786570722, 0.0073599963704157, 0.0043538453346397,
// 	0.0024499299678342);


// void main()
// {
// 	vec4 c = vec4(0.0);
// 	ivec2 p = ivec2(gl_FragCoord.yx) - ivec2(0, weights.length() >> 1);
// 	for(int i = 0; i < weights.length(); ++i){
// 		c += texelFetch(texture_map, p + ivec2(0, i), 0) * weights[i];
// 	}
// 	color = c + vec4(0.4, 0.0, 0.4, 1.0);
// 	color = vec4(1.4, 0.0, 1.4, 1.0);
// 	color = texelFetch(texture_map, ivec2(gl_FragCoord.xy), 0);
// 	color = vec4(0.0, 0.0, 0.0, 1.0);
// 	color = texture(texture_map, gl_FragCoord.xy)*vec4(1.0, 0.5, 1.0, 1.0);
// 	color = vec4(1.0);
// 	// color = c;
// }