#VERTEX
#version 430 core

layout(location = 0)    in vec2 vPosition;
layout(location = 1)    in vec2 vTCoord;
layout(location = 2)    in vec4 in_color;

out     vec2    t_coord;
out     vec4    out_color;

uniform mat4    mvp_mat;
uniform vec2    size = vec2(1, 1);

void main()
{
    t_coord = vTCoord;
    out_color = in_color;
    gl_Position = mvp_mat * vec4(vPosition*size, 0, 1);
}


#FRAGMENT
#version 430 core

in vec2 t_coord;
in vec4 out_color;
out vec4 color;

uniform sampler2D textureMap;

void main()
{
    vec3 scolor = out_color.xyz;
    float val = texture2D(textureMap, vec2(t_coord.x, t_coord.y)).r;
    color = vec4(scolor, val);
    //color = vec4(1.0, 0.0, 1.0, 1.0);
    //gl_FragColor = out_color * vec4(1.0, 1.0, 1.0, val);
    //gl_FragColor = vec4(val, val, val, val);
    //gl_FragColor = vec4(1, 1, 1, 1.0);
    //gl_FragColor = texture2D(textureMap, vec2(t_coord.x, t_coord.y));
}