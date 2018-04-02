#include "renderer.h"
#include "SDL2/SDL.h"
#include "particles.h"

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

extern Renderer* renderer;

static void myGlMessageCallback(GLenum source,
            GLenum type,
            GLuint id,
            GLenum severity,
            GLsizei length,
            const GLchar *message,
            void *userParam)
{
#if AV_PLATFORM == PLATFORM_WINDOWS
    return;
#endif
    if(severity != GL_DEBUG_SEVERITY_NOTIFICATION){
        printf("%s\n", message);
        const char* s = strstr(message, "<program>");
        if(s){
            SDL_Delay(10);
        }
    };
}

static void setupShader(int shader_enum)
{
    renderer->shaders[shader_enum].use();
    if(shader_enum == Shader_Default){
        renderer->shaders[shader_enum].addUniforms(
            6, "mvp_mat", "model_mat", "view_mat", "proj_mat", "textured", "color");
    }else if(shader_enum == Shader_Skybox){
        renderer->shaders[shader_enum].addUniforms(
            3, "view_mat", "proj_mat", "cube_map");
    }else if(shader_enum == Shader_Phong || 
            shader_enum == Shader_PBR){
        renderer->shaders[shader_enum].addUniforms(
            10, "model_mat", "light_pos", "light_colors", "light_count", "light_space_mat",
            "albedo_map", "normal_map", "roughness_map", "metalic_map", "shadow_map");
    }else if(shader_enum == Shader_PBRUntextured){
        renderer->shaders[shader_enum].addUniforms(
            13, "mvp_mat", "model_mat", "view_mat", "proj_mat", "cam_pos",
            "light_pos", "light_colors", "light_count", "light_space_mat",
            "albedo", "roughness", "metalic", "shadow_map");
    }
    else if(shader_enum == Shader_MultiTexture)
    {
        renderer->shaders[shader_enum].addUniforms(
            6, "mvp_mat", "model_mat", "view_mat", "proj_mat", "t1_map", "t2_map");
    }
    else if (shader_enum == Shader_ShadowMap){
        renderer->shaders[shader_enum].addUniforms(2, "light_space_mat", "model_mat");
    }
    else if (shader_enum == Shader_PostProcessGamma)
    {
        renderer->shaders[shader_enum].addUniforms(3, "textureMap", "light_screen_space", "use_bloom");
    }
    else if(shader_enum == Shader_Particles)
    {
        renderer->shaders[shader_enum].addUniforms(4, "model_mat", "view_mat", "proj_mat", "atlass_inv_rows");
    }
    else if(shader_enum == Shader_GaussianBlur)
    {
        renderer->shaders[shader_enum].addUniforms(1, "horizontal");
    }
    else if (shader_enum == Shader_Reflection)
    {
        renderer->shaders[shader_enum].addUniforms(4, "model_mat", "view_mat", "proj_mat", "cam_pos");
    }
    else if(shader_enum == Shader_Debug){
        renderer->shaders[shader_enum].addUniforms(
            3, "model_mat", "view_mat", "proj_mat");
    }
    else if (shader_enum == Shader_Text){
        renderer->shaders[shader_enum].addUniforms(2, "mvp_mat", "size");
    }
    else if (shader_enum == Shader_Framebuffer || shader_enum == Shader_Depthbuffer)
        renderer->shaders[shader_enum].addUniforms(1, "mvp_mat");
    else if (shader_enum == Shader_Light)
        renderer->shaders[shader_enum].addUniforms(2, "model_mat", "color");
}

void checkShaderReload2()
{
    for(int i = 0; i < Shader_Max; ++i)
    {
        uint64 new_change_time = fileChangeTime(renderer->shader_file_names[i]);
        if(new_change_time > renderer->shader_change_times[i])
        {
            renderer->shaders[i].deleteShaderProgram();
            renderer->shaders[i].loadFromFile(renderer->shader_file_names[i]);
            setupShader(i);
            renderer->shader_change_times[i] = new_change_time;
        }
    }
}

#if 1
int rendererInit(int width, int height)
{
    *renderer = {};
    renderer->frame_buffer_w = width;
    renderer->frame_buffer_h = height;
    //renderer->frame_buffer_w = 1920;
    //renderer->frame_buffer_h = 1080;
    renderer->window_w = width;
    renderer->window_h = height;
    {//setup gl state
        GLCALL(glEnable(GL_DEBUG_OUTPUT));
        GLCALL(glDebugMessageCallback((GLDEBUGPROC)myGlMessageCallback, NULL));
        //glEnable(GL_CULL_FACE); 
        //glCullFace(GL_BACK);
        //glFrontFace(GL_CCW);
        GLCALL(glEnable(GL_DEPTH_TEST));
        //glDepthFunc(GL_LEQUAL);
        GLCALL(glEnable(GL_STENCIL_TEST));
        GLCALL(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
        GLCALL(glEnable(GL_BLEND));
        GLCALL(glEnable(GL_CULL_FACE));
        GLCALL(glEnable(GL_BLEND));

        GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        GLCALL(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS));
    }

    GLint avail_mem_before_load;
    {//get memory info
        GLint total_mem_kb = 0;
        GLCALL(glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX,
              &total_mem_kb));

        GLint cur_avail_mem_kb = 0;
        GLCALL(glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX,
              &cur_avail_mem_kb));
        avail_mem_before_load = cur_avail_mem_kb;

        printf("Total_mem_kb %d\n", total_mem_kb);
        printf("Avail_mem_kb %d\n", cur_avail_mem_kb);
    }

    {//setup framebuffer
        unsigned int color_attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        int max_samples;
        GLCALL(glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &max_samples));
        printf("MAX SAMPLES: %d\n", max_samples);
        GLCALL(glGenFramebuffers(1, &renderer->fbo_ms));
        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer->fbo_ms));

        renderer->msaa_level = MIN(8, max_samples);

        GLCALL(glGenTextures(1, &renderer->fbo_ms_color_texture));
        GLCALL(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderer->fbo_ms_color_texture))
        GLCALL(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, renderer->msaa_level,
                GL_RGBA16F, width, height, GL_TRUE));
        GLCALL(glGenTextures(1, &renderer->fbo_ms_bloom_texture));
        GLCALL(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderer->fbo_ms_bloom_texture));
        GLCALL(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, renderer->msaa_level, 
                GL_RGBA16F, width, height, GL_TRUE));

        GLCALL(glGenTextures(1, &renderer->fbo_ms_depth_texture));
        GLCALL(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderer->fbo_ms_depth_texture));
        GLCALL(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, renderer->msaa_level,
                GL_DEPTH24_STENCIL8, width, height, GL_TRUE));

        GLCALL(glGenTextures(1, &renderer->fbo_ms_light_scattering_texture));
        GLCALL(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderer->fbo_ms_light_scattering_texture));
        // GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr));
        GLCALL(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, renderer->msaa_level, 
                GL_RGBA16F, width, height, GL_TRUE));

        GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D_MULTISAMPLE, renderer->fbo_ms_color_texture, 0));
        GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                               GL_TEXTURE_2D_MULTISAMPLE, renderer->fbo_ms_bloom_texture, 0));
        GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,
                               GL_TEXTURE_2D_MULTISAMPLE, renderer->fbo_ms_light_scattering_texture, 0));

        GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                               GL_TEXTURE_2D_MULTISAMPLE, renderer->fbo_ms_depth_texture, 0));

        glDrawBuffers(3, color_attachments);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            printf("frame MS buff is incomplete %X \n", glGetError());
            return -1;
        };

        //extra
        GLCALL(glGenFramebuffers(1, &renderer->fbo_extra));
        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer->fbo_extra));

        GLCALL(glGenTextures(1, &renderer->fbo_extra_color_texture));
        GLCALL(glBindTexture(GL_TEXTURE_2D, renderer->fbo_extra_color_texture));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height,
                     0, GL_RGBA, GL_FLOAT, nullptr));

        GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, renderer->fbo_extra_color_texture, 0));

        glDrawBuffers(1, color_attachments);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            printf("frame buff is incomplete \n");
            return -1;
        }

        GLCALL(glGenFramebuffers(1, &renderer->fbo_bloom));
        GLCALL(glGenTextures(1, &renderer->fbo_bloom_texture));
        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer->fbo_bloom));
        GLCALL(glBindTexture(GL_TEXTURE_2D, renderer->fbo_bloom_texture));
        GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
            renderer->fbo_bloom_texture, 0));
        GLCALL(glDrawBuffers(1, color_attachments));

        GLCALL(glGenFramebuffers(1, &renderer->fbo_light_scattering));
        GLCALL(glGenTextures(1, &renderer->fbo_light_scattering_texture));
        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer->fbo_light_scattering));
        GLCALL(glBindTexture(GL_TEXTURE_2D, renderer->fbo_light_scattering_texture));
        GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
            renderer->fbo_light_scattering_texture, 0));
        GLCALL(glDrawBuffers(1, color_attachments));

        GLCALL(glGenFramebuffers(1, &renderer->fbo_tmp);)
        GLCALL(glGenTextures(1, &renderer->fbo_tmp_texture));
        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer->fbo_tmp));
        GLCALL(glBindTexture(GL_TEXTURE_2D, renderer->fbo_tmp_texture));
        GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLCALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
                renderer->fbo_tmp_texture, 0));
        GLCALL(glDrawBuffers(1, color_attachments));

        //dir shadow map fb
        glGenFramebuffers(1, &renderer->frame_buffer_shadow_dir);
        glBindFramebuffer(GL_FRAMEBUFFER, renderer->frame_buffer_shadow_dir);

        glGenTextures(1, &renderer->frame_buffer_shadow_dir_depth);
        glBindTexture(GL_TEXTURE_2D, renderer->frame_buffer_shadow_dir_depth);
        renderer->shadow_buffer_h = 2048;
        renderer->shadow_buffer_w = 2048;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 
            renderer->shadow_buffer_w, renderer->shadow_buffer_h, 0,
            GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 
            renderer->frame_buffer_shadow_dir_depth, 0);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            printf("frame buff is incomplete \n");
            return -1;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    /*
     * UNIFORM BUFFER BINDINGS!
     * 0 - view and projection matrices, total time passed
     *      range: 0 - uniform_buffer_alignment
     * 1 - 10 point lights closest to the camera
     *      range: n*uniform_buffer_alignment - m*uniform_buffer_alignment
     * 
     * //2 - particles 
     *      range: light_offset - n*uniform_buffer_alignment
     *
     */
    {//create uniform buffers
        GLint max_uniform_buffer_size;
        glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &max_uniform_buffer_size);
        glGenBuffers(1, &renderer->uniform_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, renderer->uniform_buffer);
        glBufferData(GL_UNIFORM_BUFFER, Megabytes(1), NULL, GL_DYNAMIC_DRAW);

        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT,
                &renderer->uniform_buffer_alignment);

        int alignment = renderer->uniform_buffer_alignment;
        printf("alignment %d\n", alignment);
        int off = 0;
        int size = ceil((float)(sizeof(mat4)*2+8)/alignment)*alignment;
        renderer->uniform_buffer_light_offset = size;
        renderer->uniform_buffer_particles_offset =
            3*renderer->uniform_buffer_alignment;

        glBindBufferRange(GL_UNIFORM_BUFFER, 0, renderer->uniform_buffer,
                0, renderer->uniform_buffer_light_offset);
        glBindBufferRange(GL_UNIFORM_BUFFER, 1, renderer->uniform_buffer,
                renderer->uniform_buffer_light_offset,
                renderer->uniform_buffer_particles_offset);
        glBindBufferRange(GL_UNIFORM_BUFFER, 2, renderer->uniform_buffer,
                renderer->uniform_buffer_particles_offset,
                Megabytes(1)-renderer->uniform_buffer_particles_offset);
    }


    {// gen performance queries
        renderer->query_back_buff = 0;
        renderer->query_front_buff = 1;
        GLCALL(glGenQueries(1, &renderer->query_id[0]));
        GLCALL(glGenQueries(1, &renderer->query_id[1]));
        // to avoid errors on first frame
        GLCALL(glBeginQuery(GL_TIME_ELAPSED, renderer->query_id[1]));
        GLCALL(glEndQuery(GL_TIME_ELAPSED));
    }


    {//setup shader stuff
        renderer->shader_file_names[Shader_PBR] = "shaders/pbr.glsl";
        renderer->shader_file_names[Shader_Skybox] = "shaders/skybox.glsl";
        renderer->shader_file_names[Shader_PostProcessGamma] = "shaders/post_tonemap_gamma.glsl";
        renderer->shader_file_names[Shader_Phong] = "shaders/phong.glsl";
        renderer->shader_file_names[Shader_ShadowMap] = "shaders/shadow_map.glsl";
        renderer->shader_file_names[Shader_Framebuffer] = "shaders/framebuffer.glsl";
        renderer->shader_file_names[Shader_Depthbuffer] = "shaders/depthbuffer.glsl";
        renderer->shader_file_names[Shader_PBRUntextured] = "shaders/pbr_textureless.glsl";
        renderer->shader_file_names[Shader_MultiTexture] = "shaders/multitexture.glsl";
        renderer->shader_file_names[Shader_Particles] = "shaders/particles_aless.glsl";
        renderer->shader_file_names[Shader_GaussianBlur] = "shaders/gaussian_blur.glsl";
        renderer->shader_file_names[Shader_Reflection] = "shaders/reflection.glsl";
        renderer->shader_file_names[Shader_Debug] = "shaders/debug.glsl";
        renderer->shader_file_names[Shader_Text] = "shaders/text.glsl";
        renderer->shader_file_names[Shader_Light] = "shaders/light.glsl";
        checkShaderReload2();
    }

    {//load meshes
        MeshData mesh_data;
        //generateFlatGrid(&mesh_data, 10, 10, 5, 5);
        float x = 4;
        generatePlane(&mesh_data, {-x, x, 0}, {-x, -x, 0},
                {x, -x, 0}, {x, x, 0}, 8.f);
        loadMesh(&renderer->meshes[Mesh_Grid], &mesh_data);
        freeMeshData(&mesh_data);
        
        x = 0.5f;
        generatePlane(&mesh_data, {-x, x, 0}, {-x, -x, 0},
                {x, -x, 0}, {x, x, 0}, 1.f);
        loadMesh(&renderer->meshes[Mesh_Plane], &mesh_data);
        freeMeshData(&mesh_data);

        // generateTerrain(&mesh_data, 200, 200, 50, 50);
        // loadMesh(&renderer->meshes[Mesh_Terrain], &mesh_data);
        // freeMeshData(&mesh_data);

        generateCube(&mesh_data, 1);
        loadMesh(&renderer->meshes[Mesh_Cube], &mesh_data);
        freeMeshData(&mesh_data);

        generateSkybox(&mesh_data, 1);
        loadMesh(&renderer->meshes[Mesh_Skybox], &mesh_data);
        freeMeshData(&mesh_data);

        generateAxisVectors(&mesh_data, 3);
        //generateFlatGrid(&mesh_data, 10, 10, 5, 5);
        loadMesh(&renderer->meshes[Mesh_AxisVectors], &mesh_data);
        freeMeshData(&mesh_data);

        generateCylinder(&mesh_data, 0.5f, 1.f, 32);
        loadMesh(&renderer->meshes[Mesh_Cylinder], &mesh_data);
        freeMeshData(&mesh_data);

        generateCylinder(&mesh_data, 0.5f, 1.f, 8);
        loadMesh(&renderer->meshes[Mesh_CylinderAxis], &mesh_data);
        freeMeshData(&mesh_data);

        generateCone(&mesh_data, 0.5f, 1.f, 32);
        loadMesh(&renderer->meshes[Mesh_Cone], &mesh_data);
        freeMeshData(&mesh_data);

        generateCone(&mesh_data, 0.5f, 1.f, 8);
        loadMesh(&renderer->meshes[Mesh_ConeAxis], &mesh_data);
        freeMeshData(&mesh_data);

        generatePlane(&mesh_data, {-1, 1, 0}, {-1, -1, 0}, {1, -1, 0}, {1, 1, 0});
        loadMesh(&renderer->meshes[Mesh_FBO], &mesh_data);
        freeMeshData(&mesh_data);

        loadFromFile(&mesh_data, "assets/cerberus.avmesh");
        loadMesh(&renderer->meshes[Mesh_Cerberus], &mesh_data);
        freeMeshData(&mesh_data);

        loadFromFile(&mesh_data, "assets/sphere.avmesh");
        loadMesh(&renderer->meshes[Mesh_Sphere], &mesh_data);
        freeMeshData(&mesh_data);

        loadFromFile(&mesh_data, "assets/buddha.avmesh");
        loadMesh(&renderer->meshes[Mesh_Buddha], &mesh_data);
        freeMeshData(&mesh_data);

        loadFromFile(&mesh_data, "assets/dragon.avmesh");
        loadMesh(&renderer->meshes[Mesh_Dragon], &mesh_data);
        freeMeshData(&mesh_data);
        // generateSphere(&mesh_data, 32, 32, 1.f);
        // loadMesh(&renderer->meshes[Mesh_Sphere], &mesh_data);
        //freeMeshData(&mesh_data);

        MeshData2D mesh_data2d;
        generateQuad(&mesh_data2d, 1.f);
        loadMesh(&renderer->meshes[Mesh_Quad], &mesh_data2d);
        freeMeshData(&mesh_data2d);
    }

    loadMaterial(&renderer->materials[Material_Rooftile], 
        "assets/rooftile.tga", "assets/rooftile_normal.tga",
        "assets/rooftile_roughness.tga", nullptr);

    loadMaterial(&renderer->materials[Material_Brick], 
        "assets/wallbrick.tga", "assets/wallbrick_normal.tga",
        "assets/wallbrick_roughness.tga", nullptr);

    loadMaterial(&renderer->materials[Material_Cobblestone], 
        "assets/floor/cobblestone.tga", "assets/floor/cobblestone_normal.tga",
        "assets/floor/cobblestone_roughness.tga", nullptr);

    loadMaterial(&renderer->materials[Material_Bentsteel], 
        "assets/bentsteel.tga", "assets/bentsteel_normal.tga",
        "assets/bentsteel_roughness.tga", nullptr, true);

    loadMaterial(&renderer->materials[Material_Solarpanel], 
        "assets/solarpanel.tga", "assets/solarpanel_normal.tga",
        "assets/solarpanel_roughness.tga", "assets/solarpanel_metalic.tga");
    
    loadMaterial(&renderer->materials[Material_Cerberus],
        "assets/cerberus/cerberus_a.tga", "assets/cerberus/cerberus_n.tga",
        "assets/cerberus/cerberus_r.tga", "assets/cerberus/cerberus_m.tga");


    {//texture load stuff
        const char* skybox_texture_files[6] = {
#if 1
            "assets/skybox/tropicalsunny/tropicalsunny_rt.tga",
            "assets/skybox/tropicalsunny/tropicalsunny_lf.tga",
            "assets/skybox/tropicalsunny/tropicalsunny_up.tga",
            "assets/skybox/tropicalsunny/tropicalsunny_dn.tga",
            "assets/skybox/tropicalsunny/tropicalsunny_bk.tga",
            "assets/skybox/tropicalsunny/tropicalsunny_ft.tga"
#else

            "assets/skybox/corona_rt.tga", "assets/skybox/corona_lf.tga",
            "assets/skybox/corona_up.tga", "assets/skybox/corona_dn.tga",
            "assets/skybox/corona_bk.tga", "assets/skybox/corona_ft.tga"
#endif
        };
        renderer->textures[Texture_Skybox] = textureCubeMapLoad(skybox_texture_files);
        assert(renderer->textures[Texture_Skybox] != 0);

        renderer->textures[Texture_White] = texture2DLoadWhiteTexture();
        renderer->textures[Texture_Black] = texture2DLoadBlackTexture();
        // renderer->textures[Texture_Particle_Flame] = texture2DLoad("assets/fire.tga");
    }


    {//font laod stuff
        fontLoad(&renderer->fonts[Font_Default], "assets/liberationmono.ttf", 32);
        textBufferCreate(&renderer->ui_textbuffer, 1024); 
    }


    {//setup matrices
        renderer->ortho = mat4_ortho(0, width, height, 0, 1, -1);
        //renderer->perspective = perspective(radians(45.f), (float)width/height,
                    //0.0001, 10000.f);
        // renderer->width = width;
        // renderer->height = height;
    }

    printf("after load\n");
        GLint total_mem_kb = 0;
        GLCALL(glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX,
              &total_mem_kb));

        GLint cur_avail_mem_kb = 0;
        GLCALL(glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX,
              &cur_avail_mem_kb));

        printf("Total_mem_kb %d\n", total_mem_kb);
        printf("Avail_mem_kb %d\n", cur_avail_mem_kb);

        printf("USED: %d\n", avail_mem_before_load - cur_avail_mem_kb);
    return 0;
}
#endif

static void swapQueryBuffers()
{
    if(renderer->query_back_buff){
        renderer->query_back_buff = 0;
        renderer->query_front_buff = 1;
    } else {
        renderer->query_back_buff = 1;
        renderer->query_front_buff = 0;
    }
}

void drawFrameBegin()
{
    GLuint query_back_buff = renderer->query_back_buff;
    GLCALL(glBeginQuery(GL_TIME_ELAPSED, renderer->query_id[query_back_buff]));
    checkShaderReload2();
    GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer->fbo_ms));
    unsigned int draw_buffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    // GLCALL(glDrawBuffers(2, draw_buffers));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    textBufferBegin(&renderer->ui_textbuffer);
}
static uint64_t gputime = 0;
static int gpu_frame = 0;

void blurBloomTexture(Renderer* renderer, int fbo);
void drawTextbuffer(TextBuffer* tbuffer, Font* font);
void drawFrameEnd()
{
    unsigned int draw_buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer->fbo_extra));
    GLCALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer->fbo_ms));
    GLCALL(glReadBuffer(GL_COLOR_ATTACHMENT0));
    GLCALL(glBlitFramebuffer(0, 0, renderer->frame_buffer_w, renderer->frame_buffer_h,
             0, 0, renderer->frame_buffer_w, renderer->frame_buffer_h,
             GL_COLOR_BUFFER_BIT, GL_LINEAR));

    GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer->fbo_bloom));
    GLCALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer->fbo_ms));
    GLCALL(glReadBuffer(GL_COLOR_ATTACHMENT1));
    GLCALL(glBlitFramebuffer(0, 0, renderer->frame_buffer_w, renderer->frame_buffer_h,
             0, 0, renderer->frame_buffer_w, renderer->frame_buffer_h,
             GL_COLOR_BUFFER_BIT, GL_LINEAR));

    GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer->fbo_light_scattering));
    GLCALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer->fbo_ms));
    GLCALL(glReadBuffer(GL_COLOR_ATTACHMENT2));
    GLCALL(glBlitFramebuffer(0, 0, renderer->frame_buffer_w, renderer->frame_buffer_h,
             0, 0, renderer->frame_buffer_w, renderer->frame_buffer_h,
             GL_COLOR_BUFFER_BIT, GL_LINEAR));

    //blur bloom texture
    Shader* bshader = &renderer->shaders[Shader_GaussianBlur];
    bshader->use();
    GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer->fbo_tmp));
    bshader->setUniform("horizontal", 1);
    texture2DBind(renderer->fbo_bloom_texture, 0);
    GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer->fbo_bloom));
    bshader->setUniform("horizontal", 0);
    texture2DBind(renderer->fbo_tmp_texture, 0);
    GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer->fbo_tmp));
    bshader->setUniform("horizontal", 1);
    texture2DBind(renderer->fbo_light_scattering_texture, 0);
    GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer->fbo_light_scattering));
    bshader->setUniform("horizontal", 0);
    texture2DBind(renderer->fbo_tmp_texture, 0);
    GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    // draw ui fbos
    for(int i = 0; i < renderer->ui_fbo_count; ++i)
    {
        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer->fbo_extra));
        drawFramebuffer(renderer->ui_fbos[i], renderer->ui_fbo_mats[i]);
    }
    for(int i = 0; i < renderer->ui_fbo_count; ++i)
    {
        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer->fbo_bloom));
        drawFramebuffer(renderer->textures[Texture_Black], renderer->ui_fbo_mats[i]);
        GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer->fbo_light_scattering));
        drawFramebuffer(renderer->textures[Texture_Black], renderer->ui_fbo_mats[i]);
    }
    renderer->ui_fbo_count = 0;
    GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, renderer->fbo_extra));

    GLCALL(glDisable(GL_DEPTH_TEST));
    //draw ui text
    textBufferEnd(&renderer->ui_textbuffer);
    drawTextbuffer(&renderer->ui_textbuffer, &renderer->fonts[Font_Default]);

    GLCALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GLCALL(glViewport(0, 0, renderer->window_w, renderer->window_h));

    Shader* shader = &renderer->shaders[Shader_PostProcessGamma];
    shader->use();
    texture2DBind(renderer->fbo_extra_color_texture, 0);
    texture2DBind(renderer->fbo_bloom_texture, 1);
    texture2DBind(renderer->fbo_light_scattering_texture, 2);
    GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    GLCALL(glViewport(0, 0, renderer->frame_buffer_w, renderer->frame_buffer_h));
    GLCALL(glEnable(GL_DEPTH_TEST));

    GLuint64 frame_time;
    GLuint query_front_buff = renderer->query_front_buff;
    GLCALL(glEndQuery(GL_TIME_ELAPSED));
    GLCALL(glGetQueryObjectui64v(renderer->query_id[query_front_buff],
            GL_QUERY_RESULT, &frame_time));
    swapQueryBuffers();

    renderer->last_frame_time = (frame_time);

    gputime += frame_time;
    ++gpu_frame;
    const int frames = 60*5;
    if(gpu_frame >= frames){
        printf("AVG frame time per %d frames: %.3fus\n",
                frames, (gputime/gpu_frame)/1000.f);
        gpu_frame = 0;
        gputime = 0;
    }
}

void drawText(const char* str, vec2 pos, float size, vec4 color)
{
    Text text;
    text.font = &renderer->fonts[Font_Default];
    text.pos = pos;
    text.str = str;
    text.size = size;
    text.color = color;
    textBufferPush(&renderer->ui_textbuffer, &text);
}

void drawTextbuffer(TextBuffer* tbuffer, Font* font)
{
    GLCALL(glBindVertexArray(renderer->ui_textbuffer.vao));
    Shader* shader = &renderer->shaders[Shader_Text];
    shader->use();
    shader->setUniform("mvp_mat", renderer->ortho);
    GLCALL(glActiveTexture(GL_TEXTURE0));
    GLCALL(glBindTexture(GL_TEXTURE_2D, font->texture));
    GLCALL(glDrawArrays(GL_TRIANGLES, 0, renderer->ui_textbuffer.vertex_count));
}

static void drawUpVector(mat4 model, mat4 view_proj, vec4 color)
{
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //glDisable(GL_CULL_FACE);
    float scale = 0.05f;
    //mat4 m1 = mat4_scale(vec3{0.3, 0.3, 12}*scale);
    mat4 m1 = mat4_scale(vec3{0.4f, 0.4f, 12}*scale);
    mat4 m1t = mat4_translation(vec3{0, 0, 0.5f});
    m1 = m1*m1t;

    glBindVertexArray(renderer->meshes[Mesh_CylinderAxis].vao);
    glUseProgram(renderer->shaders[Shader_Default].program);

    glUniformMatrix4fv(renderer->shaders[Shader_Default].getUniform("mvp_mat"),
            1, GL_FALSE, (view_proj*model*m1).e);
    glUniform4f(renderer->shaders[Shader_Default].getUniform("color"),
                color.r, color.g, color.b, color.a);
    glUniform1i(renderer->shaders[Shader_Default].getUniform("textured"), 0);
    glDrawElements(renderer->meshes[Mesh_CylinderAxis].mode,
            renderer->meshes[Mesh_CylinderAxis].index_count,
            GL_UNSIGNED_INT, 0);

    //mat4 m2 = mat4_translation(vec3{0, 0, m1.m22});
    mat4 m2 = mat4_scale({scale, scale, scale});
    m2.w.xyz = {0, 0, m1.m22+0.5f*scale};

    glBindVertexArray(renderer->meshes[Mesh_ConeAxis].vao);
    glUseProgram(renderer->shaders[Shader_Default].program);

    glUniformMatrix4fv(renderer->shaders[Shader_Default].getUniform("mvp_mat"),
            1, GL_FALSE, (view_proj*model*m2).e);
    glUniform4f(renderer->shaders[Shader_Default].getUniform("color"),
                color.r, color.g, color.b, color.a);
    glUniform1i(renderer->shaders[Shader_Default].getUniform("textured"), 0);
    glDrawElements(renderer->meshes[Mesh_ConeAxis].mode,
            renderer->meshes[Mesh_ConeAxis].index_count,
            GL_UNSIGNED_INT, 0);


    glUniform1i(renderer->shaders[Shader_Default].getUniform("textured"), 1);
    glUniform4f(renderer->shaders[Shader_Default].getUniform("color"),
                1.f, 1.f, 1.f, 1.f);

    //glEnable(GL_CULL_FACE);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

void drawAxisVectors(mat4 model, mat4 view_proj)
{
    drawUpVector(model, view_proj, vec4{0, 0, 1, 1});
    mat4 modelx = model * mat4_rotationy(radians(90));
    drawUpVector(modelx, view_proj, vec4{1, 0, 0, 1});
    mat4 modely = model * mat4_rotationx(radians(-90));
    drawUpVector(modely, view_proj, vec4{0, 1, 0, 1});
}

static void printmat(float* e)
{
    for(int i = 0; i < 4; ++i){
        printf("%.3f %.3f %.3f %.3f\n",
                e[i*4+0], e[i*4+1],
                e[i*4+2], e[i*4+3]);
    }
    printf("\n");
}

void drawMesh(Mesh* mesh, Material* material, Shader* shader, mat4 model_mat,
    LightInfo* lights)
{
    GLCALL(glBindVertexArray(mesh->vao));
    shader->use();
    if(shader == &renderer->shaders[Shader_PBRUntextured])
    {
    shader->setUniform("albedo", material->albedo);
    shader->setUniform("roughness", material->roughness);
    shader->setUniform("metalic", material->metalic);
    }
    else
    {
    texture2DBind(material->albedo_map, Target_Albedo);
    texture2DBind(material->normal_map, Target_Normal);
    texture2DBind(material->roughness_map, Target_Roughness);
    texture2DBind(material->metalic_map, Target_Metalic);
    }
    texture2DBind(renderer->frame_buffer_shadow_dir_depth, Target_Shadow);

    shader->setUniform("model_mat", model_mat);

    //TODO: move to UBO
    shader->setUniform("light_pos", lights->point_light_count, lights->point_light_positions);
    shader->setUniform("light_colors", lights->point_light_count, lights->point_light_colors);
    shader->setUniform("light_count", lights->point_light_count);
    shader->setUniform("light_space_mat", lights->light_space_mat);

    if(shader == &renderer->shaders[Shader_Reflection])
    {
        textureCubeMapBind(renderer->textures[Texture_Skybox]);
    }

    GLCALL(glDrawElements(mesh->mode, mesh->index_count, GL_UNSIGNED_INT, 0));
}

void drawMesh(int mesh, int material, int shader,
        mat4 model_mat, LightInfo* lights)
{
    drawMesh(&renderer->meshes[mesh],
        &renderer->materials[material],
        &renderer->shaders[shader],
        model_mat, lights);
}

void drawScene(Scene* scene)
{
    vec4 light_screen_space = (scene->camera->getViewProjectionMatrix()*
        mat4_translation(-scene->lights.sun_light_direction*2000.f)*vec4{0, 0, 0, 1});
        // mat4_translation({-0.5, -1.5, 0.5})*vec4{0, 0, 0, 1});

    light_screen_space.xyz /= light_screen_space.w;
    light_screen_space.xy += {1, 1};
    light_screen_space.xy *= 0.5;
    // light_screen_space.y = 1-light_screen_space.y;
    renderer->shaders[Shader_PostProcessGamma].use();
    renderer->shaders[Shader_PostProcessGamma].setUniform("light_screen_space", light_screen_space.xyz);
    // printf("%.3f %.3f %.3f %.3f\n", light_screen_space.x, light_screen_space.y, 
    //                             light_screen_space.z, light_screen_space.w);

    for(DrawItem* obj = scene->objects; obj != &scene->objects[scene->object_count]; ++obj)
    {
        drawMesh(obj->model, obj->material, obj->shader, obj->model_mat, &scene->lights);
    }
}

void drawSceneDebug(Scene* scene)
{
    for(DrawItem* obj = scene->objects; obj != &scene->objects[scene->object_count]; ++obj)
    {
        drawMesh(obj->model, obj->material, Shader_Debug, obj->model_mat, &scene->lights);
    }
}

void drawLights(LightInfo* lights)
{
    Shader* shader = &renderer->shaders[Shader_Light];
    shader->use();
    Mesh* mesh = &renderer->meshes[Mesh_Sphere];
    GLCALL(glBindVertexArray(mesh->vao));
    for(int i = 0; i < lights->point_light_count; ++i)
    {
        shader->setUniform("model_mat", mat4_translation(lights->point_light_positions[i]));
        shader->setUniform("color", lights->point_light_colors[i]);
        GLCALL(glDrawElements(mesh->mode, mesh->index_count, GL_UNSIGNED_INT, 0));
    }
    vec3 pos = -normalize(lights->sun_light_direction)*10.f;
    shader->setUniform("model_mat", mat4_translation(pos));
    shader->setUniform("color", lights->sun_light_color);
    GLCALL(glDrawElements(mesh->mode, mesh->index_count, GL_UNSIGNED_INT, 0));
}

void drawSceneToShadowMap(Scene* scene)
{
    //draw scene from sun light perspective
    //do not support pointlight shadows yet.
    float near_plane = 0.10f;
    float far_plane = 10.0f;
    mat4 light_proj = mat4_ortho(-5.0f, 5.f, -5.f, 5.f, near_plane, far_plane);
    vec3 pos = {0.9f, -3, 3};
    // pos = {0.9, -3, 0.5};
    pos = -scene->lights.sun_light_direction*5.0;
    vec3 look = normalize(vec3{0, 0.0, 0});
    mat4 light_view = look_at(pos, look, {0, 0, 1});
    mat4 light_space_mat = light_proj*light_view;

    scene->lights.light_space_mat = light_space_mat;

    Shader* shader = &renderer->shaders[Shader_ShadowMap];
    shader->use();
    shader->setUniform("light_space_mat", light_space_mat);

    glViewport(0, 0, renderer->shadow_buffer_w, renderer->shadow_buffer_h);
    glBindFramebuffer(GL_FRAMEBUFFER, renderer->frame_buffer_shadow_dir);
    glClear(GL_DEPTH_BUFFER_BIT);
    //glCullFace(GL_FRONT);
    for(DrawItem* obj = scene->objects; obj != &scene->objects[scene->object_count]; ++obj)
    {
        glBindVertexArray(renderer->meshes[obj->model].vao);
        shader->setUniform("model_mat", obj->model_mat);
        glDrawElements(renderer->meshes[obj->model].mode, 
            renderer->meshes[obj->model].index_count, GL_UNSIGNED_INT, 0);
    }
    glCullFace(GL_BACK);
    glViewport(0, 0, renderer->frame_buffer_w, renderer->frame_buffer_h);
    glBindFramebuffer(GL_FRAMEBUFFER, renderer->fbo_ms);
}

void drawFramebuffer(int attachment, mat4 mvp_mat)
{
    glDisable(GL_DEPTH_TEST);
    Shader* shader = &renderer->shaders[Shader_Framebuffer];
    if(attachment == renderer->frame_buffer_shadow_dir_depth)
        shader = &renderer->shaders[Shader_Depthbuffer];
    shader->use();
    shader->setUniform("mvp_mat", mvp_mat);
    texture2DBind(attachment);
    GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    glEnable(GL_DEPTH_TEST);
}

void drawUIFramebuffer(int attachment, mat4 mvp_mat)
{
    renderer->ui_fbos[renderer->ui_fbo_count] = attachment;
    renderer->ui_fbo_mats[renderer->ui_fbo_count] = mvp_mat;
    ++renderer->ui_fbo_count;
}

void drawSkybox(int texture, Camera* camera)
{
    Mesh* mesh = &renderer->meshes[Mesh_Skybox];
    Shader* shader = &renderer->shaders[Shader_Skybox];
    GLCALL(glDisable(GL_CULL_FACE));
    GLCALL(glBindVertexArray(mesh->vao));
    shader->use();
    textureCubeMapBind(renderer->textures[texture]);
    GLCALL(glDrawElements(mesh->mode, mesh->index_count, GL_UNSIGNED_INT, 0));
    GLCALL(glEnable(GL_CULL_FACE));
}

void setCommonShaderVars(Renderer::CommonShaderVars* cvars)
{
    GLCALL(glBindBuffer(GL_UNIFORM_BUFFER, renderer->uniform_buffer));
    GLCALL(glBufferSubData(GL_UNIFORM_BUFFER, 0, 
            sizeof(Renderer::CommonShaderVars), cvars));
}

void rendererQuit()
{
    for(int i = 0; i < Shader_Max; ++i){
        renderer->shaders[i].deleteShaderProgram();
    }
    for(int i = 0; i < Texture_Max; ++i){
        GLCALL(glDeleteTextures(1, &renderer->textures[i]));
    }
    for(int i = 0; i < Mesh_Max; ++i){
        freeMesh(&renderer->meshes[i]);
    }

    GLCALL(glDeleteFramebuffers(1, &renderer->fbo_ms));
    GLCALL(glDeleteTextures(1, &renderer->fbo_ms_color_texture));
    GLCALL(glDeleteTextures(1, &renderer->fbo_ms_depth_texture));

    GLCALL(glDeleteFramebuffers(1, &renderer->fbo_extra));
    GLCALL(glDeleteTextures(1, &renderer->fbo_extra_color_texture));
}
