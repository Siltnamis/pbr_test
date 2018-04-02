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

uniform mat4 mvp_mat = mat4(1);

void main()
{
	uv = v_uvs[gl_VertexID];
	vec2 pos = v_positions[gl_VertexID];
	gl_Position = mvp_mat*vec4(pos, 0, 1);
}


#FRAGMENT
#version 430 core

in vec2 uv;
layout(location = 0) out vec4 color;

uniform sampler2D depthMap;

float getLinearDepth()
{
    float z_near = 0.1;    
    float z_far  = 10.0; 
    float depth = texture2D(depthMap, uv).r;
    return (2.0 * z_near) / (z_far + z_near - depth * (z_far - z_near));
}

void main()
{
	float depth = getLinearDepth();
	color = vec4(vec3(depth), 1);
}