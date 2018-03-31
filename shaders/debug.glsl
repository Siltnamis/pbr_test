#VERTEX
#version 430 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 3) in vec3 vTangent;

out VS_OUT{
    vec3 normal;
    vec3 tangent;
}vs_out;

uniform mat4 model_mat;

void main()
{
    vs_out.normal = vNormal;
    vs_out.tangent = vTangent;
    gl_Position = vec4(vPosition, 1);
}

#GEOMETRY
#version 430 core
layout (triangles) in;
layout (line_strip, max_vertices = 18) out;

in VS_OUT
{
    vec3 normal;
    vec3 tangent;
}gs_in[];

out GS_OUT
{
    vec4 color;
}gs_out;

uniform mat4 model_mat;

layout(binding = 0, std140) uniform CommonVars
{
    mat4    proj_mat;
    mat4    view_mat;
    vec3    cam_pos;
    float   time;
};


mat4 mvp_mat;
const float magnitude = 0.2;
mat3 normal_mat;

void genNormalLine(int ind)
{
    const vec4 color = vec4(0, 0, 1, 1);
    gl_Position = mvp_mat * gl_in[ind].gl_Position;
    gs_out.color = color; 
    EmitVertex();
    vec3 normal;
    //normal = normal_mat*gs_in[ind].normal.xyz;
    normal = gs_in[ind].normal.xyz;
    gl_Position = mvp_mat* (gl_in[ind].gl_Position + vec4(normal, 0)*magnitude);
    gs_out.color = color; 
    EmitVertex();
    EndPrimitive();
}

void genTangentLine(int ind)
{
    const vec4 color = vec4(0, 1, 0, 1);
    gl_Position = mvp_mat * gl_in[ind].gl_Position;
    gs_out.color = color; 
    EmitVertex();
    vec3 tangent;
    //normal = normal_mat*gs_in[ind].normal.xyz;
    tangent = gs_in[ind].tangent.xyz;
    gl_Position = mvp_mat* (gl_in[ind].gl_Position + vec4(tangent, 0)*magnitude);
    gs_out.color = color; 
    EmitVertex();
    EndPrimitive();
}

void genBitangentLine(int ind)
{
    const vec4 color = vec4(1, 0, 0, 1);
    gl_Position = mvp_mat * gl_in[ind].gl_Position;
    gs_out.color = color; 
    EmitVertex();
    vec3 bitangent;
    //normal = normal_mat*gs_in[ind].normal.xyz;
    bitangent = cross(gs_in[ind].normal.xyz, gs_in[ind].tangent.xyz);
    gl_Position = mvp_mat* (gl_in[ind].gl_Position + vec4(bitangent, 0)*magnitude);
    gs_out.color = color; 
    EmitVertex();
    EndPrimitive();
}

void main()
{
    mvp_mat = proj_mat*view_mat*model_mat;
    normal_mat = mat3(transpose(inverse(model_mat)));
    genNormalLine(0);
    genNormalLine(1);
    genNormalLine(2);
    
    genTangentLine(0);
    genTangentLine(1);
    genTangentLine(2);

    genBitangentLine(0);
    genBitangentLine(1);
    genBitangentLine(2);
}

#FRAGMENT
#version 430 core

in GS_OUT
{
    vec4 color;
}fs_in;
layout(location = 0) out vec4 color;
layout(location = 1) out vec4 bright_color;
layout(location = 2) out vec4 lscattering;

void main()
{
    //color = vec4(1, 1, 0, 1);
    color = fs_in.color;
    bright_color = fs_in.color;
    lscattering = vec4(vec3(0.0), 1.0);
}
