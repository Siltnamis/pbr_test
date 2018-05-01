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
	gl_Position = mvp_mat*vec4(pos, 0.99999, 1);
}


#FRAGMENT
#version 430 core

in vec2 uv;
layout(location = 0) out vec4 color;
layout(location = 1) out vec4 bright_color;

uniform sampler2D textureMap;
uniform float brightness;

void main()
{
    vec4 col = texture(textureMap, vec2(uv.s, 1.0 - uv.t));
    col.a *= brightness;
    color = col; 
    // color.a = 0.01;
    bright_color = col;
}