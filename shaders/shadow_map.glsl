#VERTEX
#version 430 core

layout(location = 0) in vec3 v_position;

uniform mat4 light_space_mat;
uniform mat4 model_mat;

void main()
{
	gl_Position = light_space_mat*model_mat*(vec4(v_position, 1));
}

#FRAGMENT
#version 430 core

void main()
{
	gl_FragDepth = gl_FragCoord.z;
	//gl_FragDepth = 0.0;
}