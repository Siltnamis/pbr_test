#pragma once 

#include "av/av.h"
#include "av/avmath.h"

#include "shader.h"
#include "texture.h"
#include "mesh.h"
#include "text.h"

#include "camera.h"

#define MAX_LIGHTS 10

enum Font_Enum      { Font_Default, Font_Max };

enum DrawFlags_Enum { 
    DFlag_Outlined  = (1 << 0),
    DFlag_Unlit     = (1 << 1), 
    DFlag_Phong     = (1 << 2), 
    DFlag_Normals   = (1 << 3),
    DFlag_Axis      = (1 << 4)
};

//slow way of doing stuff
struct ShaderData
{
    int type;
    mat4 mvp_mat;
    mat4 model_mat;
    mat4 view_mat;
    mat4 proj_mat;
    vec3 cam_pos;

    vec3 size;

    int texture;
    vec4 color = {1, 1, 1, 1};
};

enum Mesh_Enum
{ 
    Mesh_Quad, Mesh_FBO, Mesh_Grid, Mesh_Cube, Mesh_Cylinder, Mesh_Cone, 
    Mesh_Plane,
    Mesh_Sphere,
    Mesh_CylinderAxis, Mesh_ConeAxis,
    Mesh_Skybox,
    Mesh_AxisVectors,
    Mesh_Boxman, Mesh_Boxman_New,
    Mesh_Cerberus,
    Mesh_Buddha,
    Mesh_Dragon,
    Mesh_Lucy,
    Mesh_Max 
};

enum Shader_ID
{
    Shader_Default,
    Shader_Phong,
    Shader_PBR,
    Shader_PBRUntextured,
    Shader_Skybox,
    Shader_PostProcessGamma,
    Shader_ShadowMap,
    Shader_Framebuffer,
    Shader_Depthbuffer,
    Shader_Debug,
    Shader_MultiTexture,
    Shader_Particles,
    Shader_GaussianBlur,
    Shader_Reflection,
    Shader_Text,
    Shader_Light,
    Shader_Max
};

enum Texture
{
    Texture_Skybox,
    Texture_White,
    Texture_Black,
    Texture_Brick,
    Texture_Brick_Normal,
    Texture_Brick_Roughness,
    Texture_Rooftile,
    Texture_Rooftile_Normal,
    Texture_Rooftile_Roughness,
    Texture_Cobblestone,
    Texture_Cobblestone_Normal,
    Texture_Cobblestone_Roughness,
    Texture_Cobblestone_Oclusion,

    Texture_Bentsteel,
    Texture_Bentsteel_Normal,
    Texture_Bentsteel_Roughness,
    
    Texture_Solarpanel,
    Texture_Solarpanel_Normal,
    Texture_Solarpanel_Roughness,
    Texture_Solarpanel_Metalic,

    Texture_Particle_Flame,

    Texture_Max
};

//these cosntants must match shader
enum Texture_Target : int
{
    Target_Default = 0, Target_Albedo = 0,
    Target_Normal,
    Target_Roughness,
    Target_Metalic,
    Target_Shadow,

    Target_MAX
};

enum Material_Enum
{
    //untextured
    Material_test,
    Material_Gold,
    Material_Silver,
    //textured
    Material_Brick,
    Material_Rooftile,
    Material_Cobblestone,
    Material_Bentsteel,
    Material_Solarpanel,
    Material_Cerberus,
    Material_Max
};

struct DrawItem
{
    int model;
    int material;
    int shader;
    mat4 model_mat;
};

struct LightInfo
{
    static const int max_lights = 10;
    vec3 sun_light_color; // sun. only in the scene
    vec3 sun_light_direction;

    vec3 point_light_positions[max_lights];
    vec3 point_light_colors[max_lights];
    int point_light_count = 0;
    mat4 light_space_mat;
};

struct Scene
{
    static const int max_size = 4048;
    DrawItem objects[max_size] = {};
    int object_count = 0;

    LightInfo lights;
    Camera* camera;
    

    void push(const DrawItem& item)
    {
        objects[object_count++] = item;
    }

    void pushPointLight(const vec3& position, const vec3& color)
    {
        lights.point_light_positions[lights.point_light_count] = position;
        lights.point_light_colors[lights.point_light_count] = color;
        ++lights.point_light_count;
    }
};

struct Renderer
{
    //shader hotloading stuff
    const char* vert_shaders[Shader_Max];
    const char* frag_shaders[Shader_Max];
    const char* geom_shaders[Shader_Max];
    uint64      vs_change_times[Shader_Max];
    uint64      fs_change_times[Shader_Max];
    uint64      gs_change_times[Shader_Max];

    const char* shader_file_names[Shader_Max];
    uint64      shader_change_times[Shader_Max]; 

    Shader      shaders[Shader_Max];
    GLuint      textures[Texture_Max];

    //Mesh        grid;
    Mesh        meshes[Mesh_Max];

    //"better" text stuff
    Font        fonts[Font_Max];
    TextBuffer  ui_textbuffer;

    Material    materials[Material_Max];

    //perf queries
    GLuint      query_id[2];
    GLuint      query_back_buff;
    GLuint      query_front_buff;
    GLuint64    last_frame_time;
    GLuint64    curr_frame_time;

    GLuint      fbo_ms;
    GLuint      fbo_ms_color_texture;
    GLuint      fbo_ms_bloom_texture;
    GLuint      fbo_ms_depth_texture;
    GLuint      fbo_ms_light_scattering_texture;
    
    // matches shader uniform block structure
    struct CommonShaderVars
    {
        mat4    proj;
        mat4    view;
        vec3    cam_pos;
        float   time;
    };
    GLuint      uniform_buffer;
    GLuint      uniform_buffer_light_offset;
    GLuint      uniform_buffer_particles_offset;
    GLint       uniform_buffer_alignment;

    GLuint      uniform_buffer_particles;

    // might be a good idea to group fbo and textures into a struct
    GLuint      fbo_extra;
    GLuint      fbo_extra_color_texture;

    // tmp fbo for multipass post-processing effects
    GLuint      fbo_tmp;
    GLuint      fbo_tmp_texture;

    GLuint      fbo_bloom;
    GLuint      fbo_bloom_texture;

    GLuint      fbo_light_scattering;
    GLuint      fbo_light_scattering_texture;

    GLuint      frame_buffer_shadow_dir;
    GLuint      frame_buffer_shadow_dir_depth;
    int         shadow_buffer_w;
    int         shadow_buffer_h;

    int         frame_buffer_w;
    int         frame_buffer_h;

    int         window_w;
    int         window_h;

    int         msaa_level;

    mat4        ortho;
    mat4        perspective;

    GLuint      ui_fbos[8];
    mat4        ui_fbo_mats[8];
    int         ui_fbo_count;
};

int rendererInit(int width, int height);
void checkShaderReload();
void drawFrameBegin();
void drawFrameEnd();
void drawDebugData();
void rendererQuit();
void drawSceneDebug(Scene* scene);
void drawScene(Scene* scene);
void drawSceneToShadowMap(Scene* scene);
void drawSceneToCubeMap(Scene* scene);
void setCommonShaderVars(Renderer::CommonShaderVars* cvars);
void drawFramebuffer(int attachment, mat4 mvp_mat = mat4_identity());
void drawUIFramebuffer(int attachment, mat4 mvp_mat = mat4_identity());
