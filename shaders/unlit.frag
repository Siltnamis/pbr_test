#version 430 core

in vec2 t_coord;
out vec4 out_color;

uniform sampler2D textureMap;
uniform sampler2D normalMap;
uniform bool textured;
uniform vec4 color;

void main()
{
    if(textured)
        out_color = texture(textureMap, vec2(t_coord.x, t_coord.y));
    else
        out_color = color;
    //color = vec4(1, 0, 1, 1.0);
}
