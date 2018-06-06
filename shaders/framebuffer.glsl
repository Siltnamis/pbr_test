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
layout(location = 1) out vec4 bright_color;

uniform sampler2D textureMap;

void main()
{
	float depth = texture(textureMap, uv).r;
	color = vec4(vec3(depth), 1);
    color = texture(textureMap, uv);
    // bright_color = vec4(0.0);
}