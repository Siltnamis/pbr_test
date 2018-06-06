#version 430 core

in VS_OUT
{
    vec2    uv1;
    vec2    uv2;
    float   blend;
}fs_in;

out vec4 out_color;

uniform sampler2D textureMap;

void main()
{

    vec4 c1 = texture2D(textureMap, fs_in.uv1);
    vec4 c2 = texture2D(textureMap, fs_in.uv2);
    //out_color = vec4(1, 0, 0, 1);
    //out_color = gs_in.color * c.r + vec4(0.1, 0.9, 0.2, 0);
    out_color =  mix(c1, c2, fs_in.blend)*vec4(1, 1, 1, 1);

    //out_color = vec4(1);
    //out_color = vec4(1, 1, 1, 1) * texture2D(textureMap, fs_in.uv1);
}
