#include <stdio.h>
#include <chrono>

#define AV_LIB_IMPLEMENTATION
#include "av/av.h"

#include "opengl.h"

#include "mesh.cpp"
#include "particles.cpp"
#include "text.cpp"
#include "shader.h"
#include "camera.h"
#include "texture.cpp"
#include "renderer.cpp"

#include "SDL2/SDL.h"

static const char default_title[] = "PBR Tests";
static int height = 900;
static int width = 1200;
static int tick_rate = 60;
float dt = 1.f/tick_rate;
static bool quit = false;
static int use_pbr = 0;

float global_time = 0.0f;
float flicker_rate = 1.0;
float intensity = 2.0;

static Renderer g_renderer;
Renderer* renderer = &g_renderer;

static SDL_Window* window;
static SDL_GLContext context;
char avg_fps_str[32];
float last_frame_time;

static Camera camera;

struct Input
{
    enum Control : uint64_t
    {
        MoveForward         = (1 << 0), 
        MoveBackward        = (1 << 1), 
        MoveRight           = (1 << 2),
        MoveLeft            = (1 << 3),
        MoveUp              = (1 << 4),
        MoveDown            = (1 << 5),
        IncreaseAnimSpeed   = (1 << 6),
        DecreaseAnimSpeed   = (1 << 7),
        RestoreAnimSpeed    = (1 << 8),
		ToggleAnimation     = (1 << 9),
		TogglePBR			= (1 << 10),
		ShowFramebuffer		= (1 << 11),
		ShowProfiling		= (1 << 12),
		DisableMSAA			= (1 << 13),
		DrawDebug			= (1 << 14),
    };
    uint64_t    control;
    int32_t     mouse_pos_x;
    int32_t     mouse_pos_y; 
    int32_t     mouse_move_x;
    int32_t     mouse_move_y;

    bool isSet(Control c)
    {
        return control & c;
    }
    void set(Control c)
    {
        control |= c;
    }
    void clear(Control c)
    {
        control &= ~c;
    }
    void toggle(Control c)
    {
        control ^= c;
    }
};
static Input input = {};
static float move_speed = 1.5f;

struct CLK
{
	uint64 counter;
	uint64 frequency;

	CLK()
	{
		frequency =  SDL_GetPerformanceFrequency();
	}
	inline void start()
	{
		counter = SDL_GetPerformanceCounter();
	}
	inline float time()
	{
		return (float)(SDL_GetPerformanceCounter() - counter)/frequency;
	}
	inline float restart()
	{
		float t = time();
		start();
		return t;
	}
};

void handleEvents()
{
	SDL_Event e;
    float dx = 0;
    float dy = 0;
    input.mouse_move_x = 0;
    input.mouse_move_y = 0;
    while(SDL_PollEvent(&e)){
        switch(e.type)
        {
            case SDL_QUIT:
            {
                quit = true;
            }break;

            case SDL_KEYDOWN:
            {
                if(e.key.keysym.sym == 'a'){
                    input.set(Input::MoveLeft);
                }
                else if(e.key.keysym.sym == 'd'){
					if(e.key.keysym.mod & KMOD_LALT && e.key.repeat == 0){
						input.toggle(Input::ShowProfiling);
						break;
					}else if (e.key.keysym.mod & KMOD_LCTRL && e.key.repeat == 0){
						input.toggle(Input::DrawDebug);
						break;
					}
                    input.set(Input::MoveRight);
                }
				else if(e.key.keysym.sym == 'f'){
					if(e.key.keysym.mod & KMOD_LALT && e.key.repeat == 0)
						input.toggle(Input::DisableMSAA);
                }
                else if(e.key.keysym.sym == 'w'){
                    input.set(Input::MoveForward);
                }
                else if(e.key.keysym.sym == 's'){
                    input.set(Input::MoveBackward);
                }
                else if(e.key.keysym.sym == SDLK_LSHIFT){
                    input.set(Input::MoveDown);
                }
                else if(e.key.keysym.sym == SDLK_SPACE){
                    input.set(Input::MoveUp);
                }
                else if(e.key.keysym.sym == 'p'){
					//input.set(Input::ToggleAnimation);
					input.toggle(Input::TogglePBR);
                }
                else if(e.key.keysym.sym == SDLK_KP_PLUS){
                    input.set(Input::IncreaseAnimSpeed);
                }
                else if(e.key.keysym.sym == SDLK_KP_MINUS){
                    input.set(Input::DecreaseAnimSpeed);
                }
                else if(e.key.keysym.sym == SDLK_KP_ENTER){
                    input.set(Input::RestoreAnimSpeed);
				}
				else if (e.key.keysym.sym == SDLK_TAB){
					input.set(Input::ShowFramebuffer);
				}

                else if(e.key.repeat == 0 && e.key.keysym.sym == 'g'){
                    if(SDL_GetRelativeMouseMode() == SDL_FALSE)
                        SDL_SetRelativeMouseMode(SDL_TRUE);
                    else
                        SDL_SetRelativeMouseMode(SDL_FALSE);
                }
            }break;

            case SDL_KEYUP:
            {
                if(e.key.keysym.sym == 'a'){
                    input.clear(Input::MoveLeft);
                }
                else if(e.key.keysym.sym == 'd'){
                    input.clear(Input::MoveRight);
                }
                else if(e.key.keysym.sym == 'w'){
                    input.clear(Input::MoveForward);
                }
                else if(e.key.keysym.sym == 's'){
                    input.clear(Input::MoveBackward);
                }
                else if(e.key.keysym.sym == SDLK_LSHIFT){
                    input.clear(Input::MoveDown);
                }
                else if(e.key.keysym.sym == SDLK_SPACE){
                    input.clear(Input::MoveUp);
                }
                else if(e.key.keysym.sym == 'p'){
                    //input.clear(Input::ToggleAnimation);
                }
                else if(e.key.keysym.sym == SDLK_KP_PLUS){
                    input.clear(Input::IncreaseAnimSpeed);
                }
                else if(e.key.keysym.sym == SDLK_KP_MINUS){
                    input.clear(Input::DecreaseAnimSpeed);
                }
                else if(e.key.keysym.sym == SDLK_KP_ENTER){
                    input.clear(Input::RestoreAnimSpeed);
				}
				else if (e.key.keysym.sym == SDLK_TAB){
					input.clear(Input::ShowFramebuffer);
				}
            }break;


            case SDL_MOUSEMOTION:
            {
                if(SDL_GetRelativeMouseMode() == SDL_TRUE){
                    input.mouse_move_x += e.motion.xrel;
                    input.mouse_move_y += e.motion.yrel;
                    input.mouse_pos_x = e.motion.x;
                    input.mouse_pos_x = e.motion.y;
                    dx += e.motion.xrel;
                    dy += e.motion.yrel;
                }
            }break;
			
			case SDL_WINDOWEVENT:
			{
				if(e.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					renderer->window_w = e.window.data1;
					renderer->window_h = e.window.data2;
					width = renderer->window_w;
					height = renderer->window_h;
					camera.setupProjection(radians(50.f), (float)width/height, 0.001f, 1000.f);
				}
			}

            default: break;
        }
    }
    float mouse_sens = 2.0f*0.022f;
    camera.yaw += radians(-dx*mouse_sens);
    camera.pitch += radians(-dy*mouse_sens);
    if(camera.pitch > 1.57079f)
        camera.pitch = 1.57079f;
    if(camera.pitch < -1.57079f)
        camera.pitch = -1.57079f;
}
void initState()
{
	int status = rendererInit(width, height);
	assert(status == 0);

	camera.setupProjection(radians(65.f), (float)width/height, 0.001f, 1000.f);
	camera.position = {0, -2, 0.5f};
	camera.look = normalize(vec3{0, 1, 0});
    camera.up = normalize(vec3{0, 0, 1});
    camera.right = {1, 0, 0};
    camera.yaw = radians(90.f);
    camera.pitch = radians(0.f);
    camera.update();
}

void createDBGScene(Scene* scene)
{
	mat4 model = mat4_rotationz(global_time*0.3f)*mat4_scale({0.51f, 0.51f, 0.51f});
	for(int i = 0; i < 20; ++i)
	{
		for(int j = 0; j < 20; ++j)
		{
			model.w.xyz = {-10.f + 0.55f*i, -10.f + 0.55f*j, 1.5f};
			// scene->push({Mesh_Cerberus, Material_Cerberus, Shader_PBR, model});
			// scene->push({Mesh_Cube, Material_Bentsteel, Shader_PBR, model});
		}
	}
}

void draw()
{
	Renderer::CommonShaderVars cvars;
	cvars.proj = camera.projection;
	cvars.view = camera.view;
	cvars.cam_pos = camera.position;
	cvars.time = global_time;
	setCommonShaderVars(&cvars);
	drawFrameBegin();

	mat4 model_mat = mat4_identity();
	model_mat.w.xyz = {0.0, 0.0, 0.0};

	renderer->shaders[Shader_PostProcessGamma].use();
	renderer->shaders[Shader_PostProcessGamma].setUniform("position", camera.position);

	int shader = Shader_PBR;

	renderer->materials[Material_test].albedo = {3, 4.5, 0.5};
	renderer->materials[Material_test].roughness = 0.1;
	renderer->materials[Material_test].metalic = 1.0;

	renderer->materials[Material_Gold].albedo = {1.0f, 0.766f, 0.336f};
	renderer->materials[Material_Gold].roughness = 0.2;
	renderer->materials[Material_Gold].metalic = 1.0;

	renderer->materials[Material_Silver].albedo = {0.972f, 0.960f, 0.915f};
	renderer->materials[Material_Silver].roughness = 0.2;
	renderer->materials[Material_Silver].metalic = 1.0;



	//TODO: put to some render queue
	Scene scene;
	scene.camera = &camera;

	// model_mat.w.xyz = {0.0, 0.0, -1};
	// scene.push({Mesh_Cube, Material_Rooftile, shader, model_mat});

	model_mat = mat4_identity();
	model_mat.w.xyz = {0, 0, -1};
	scene.push({Mesh_Grid, Material_Cobblestone, shader, model_mat});

	model_mat = mat4_rotationx(radians(30.f));
	model_mat.w.xyz = {-2, 0, 0};
	scene.push({Mesh_Cube, Material_Bentsteel, shader, model_mat});
	
	model_mat = mat4_rotationx(radians(20.f)) * mat4_rotationz(radians(20.f));
	model_mat.w.xyz = {2, 0, 0};
	scene.push({Mesh_Cube, Material_Brick, shader, model_mat});

	model_mat = mat4_rotationx(radians(55.f));
	model_mat.w.xyz = {0, 2, 1};
	scene.push({Mesh_Cube, Material_Solarpanel, shader, model_mat});

	model_mat = mat4_rotationz(global_time*0.15f)*mat4_scale({0.01f, 0.01f, 0.01f});
	model_mat.w.xyz = {1, -2, 1};
	scene.push({Mesh_Cerberus, Material_Cerberus, shader, model_mat});
	model_mat.w.xyz = {-3, -2, 1};
	scene.push({Mesh_Cerberus, Material_Cerberus, Shader_Reflection, model_mat});

	model_mat = mat4_translation({-1, -2, -0.5})*mat4_rotationx(global_time*0.01f);
	scene.push({Mesh_Sphere, Material_Solarpanel, shader, model_mat});

	model_mat = mat4_translation({10, -100, 100})*mat4_scale(0.1f);
	scene.push({Mesh_Sphere, Material_Solarpanel, shader, model_mat});

	model_mat = mat4_translation({0, 0, -0.5})*mat4_scale(1.0f);
	scene.push({Mesh_Cube, Material_Rooftile, Shader_PBR, model_mat});

	model_mat = mat4_translation({-3, -1, 1})*mat4_scale(0.1f);
	scene.push({Mesh_Sphere, Material_Solarpanel, shader, model_mat});

	model_mat = mat4_translation({1.5, -3.0, 0.25})*mat4_scale(0.5f);
	scene.push({Mesh_Sphere, Material_Bentsteel, shader, model_mat});

	model_mat = mat4_translation({0, 0, 3.0})*mat4_rotationz(-global_time*0.3f)*mat4_scale(1.0);
	// scene.push({Mesh_Buddha, Material_Gold, Shader_Reflection, model_mat});

	model_mat = mat4_translation({-3.5, -3.5, -1.0})*mat4_rotationz(-global_time*0.3f)*mat4_scale(1.0);
	scene.push({Mesh_Dragon, Material_Gold, Shader_PBRUntextured, model_mat});

	model_mat = mat4_translation({3.5, -3.5, -1.0})*mat4_rotationz(global_time*0.3f)*mat4_scale(1.0);
	scene.push({Mesh_Buddha, Material_Silver, Shader_PBRUntextured, model_mat});

	model_mat = mat4_translation({-3.5, 3.5, -1.0})*mat4_rotationz(-global_time*0.3f)*mat4_scale(1.0);
	scene.push({Mesh_Buddha, Material_Gold, Shader_PBRUntextured, model_mat});

	model_mat = mat4_translation({3.5, 3.5, -1.0})*mat4_rotationz(global_time*0.0f)*mat4_scale(1.0);
	scene.push({Mesh_Dragon, Material_Silver, Shader_PBRUntextured, model_mat});
	// model_mat = mat4_translation({-0.5, -1.5, 0.5})*mat4_scale(1.0f);
	// scene.push({Mesh_Sphere, Material_Bentsteel, Shader_PBR, model_mat});

	// model_mat = mat4_translation({3.5, 3.5, -1.0})*mat4_rotationz(global_time*0.3f)*mat4_scale(1.0);
	// scene.push({Mesh_Dragon, Material_Silver, Shader_PBRUntextured, model_mat});

	model_mat = mat4_translation({-1.5, -2.5, 0.5})*mat4_scale(0.5f);
	scene.push({Mesh_Sphere, Material_test, Shader_Reflection, model_mat});

	createDBGScene(&scene);

	//push all lights
	scene.pushPointLight({36, -100, 37}, {1, 1, 1});
	scene.pushPointLight({0, 10, 0.5}, {0.5, 0.5, 0.5});
	// scene.pushPointLight({36, -100, 75}, {1, 1, 1});
	//scene.pushPointLight({-3, -1, 1},{0, 0, 1});
	scene.lights.sun_light_color = {1, 1, 1};
	scene.lights.sun_light_direction = normalize(vec3{-36, 100, -37});

	drawSkybox(Texture_Skybox, &camera);
	//draw shadowmap
	drawSceneToShadowMap(&scene);
	
	drawScene(&scene);
	// drawLights(&scene.lights);
	if(input.isSet(Input::DrawDebug))
		drawSceneDebug(&scene);

	// drawSkybox(Texture_Skybox, &camera);

	if(input.isSet(Input::ShowProfiling))
	{
		char str[256] = {};
		vec4 color = {0.63f, 0.15f, 0.63f, 1.0f};
		sprintf(str, "GPU frame time: %lldus\n", renderer->last_frame_time/1000);
		drawText(str, {10, 20}, 22, {0.63f, 0.15f, 0.63f, 1.0f});
		sprintf(str, "CPU frame time: %lldus\n", (int64)(last_frame_time*1000000.f));
		drawText(str, {10, 40}, 22, {0.63f, 0.15f, 0.63f, 1.0f});
		sprintf(str, "MSAA: %d\n", renderer->msaa_level);
		drawText(str, {renderer->frame_buffer_w*0.8f, 20}, 22, {0.63f, 0.15f, 0.63f, 1.0f});
		sprintf(str, "Position: %.3fx %.3fy %.3fz    Direction: %.3fx %.3fy %.3fz   Speed: %.3f   Objects: %d", 
			camera.position.x, camera.position.y, camera.position.z,
			camera.look.x, camera.look.y, camera.look.z, move_speed, scene.object_count);
		drawText(str, {10.f, renderer->frame_buffer_h-10.f}, 22, color);
	}

	if(input.isSet(Input::ShowFramebuffer))
	{
		mat4 mvp = mat4_scale({0.5f, 0.5f, 1.f});
		mvp.col[3].xy = {-0.5f, 0.5f};
		drawUIFramebuffer(renderer->frame_buffer_shadow_dir_depth, mvp);
		mvp.col[3].xy = {0.5f, 0.5f};
		drawUIFramebuffer(renderer->fbo_bloom_texture, mvp);
		mvp.col[3].xy = {-0.5f, -0.5f};
		drawUIFramebuffer(renderer->fbo_light_scattering_texture, mvp);
		// drawUIFramebuffer(renderer->fbo_ms_, mvp);
		// drawUIFramebuffer(renderer->fbo_extra_bloom_texture, mvp);
	}
	drawFrameEnd();	

	SDL_GL_SwapWindow(window);
}

void updateState()
{
	if(input.isSet(Input::IncreaseAnimSpeed))
		move_speed += 0.1;
	if(input.isSet(Input::DecreaseAnimSpeed))
		move_speed -= 0.1;
	if(input.isSet(Input::MoveForward)){
        camera.moveForward(move_speed*dt);
    }
    if(input.isSet(Input::MoveBackward)){
        camera.moveForward(-move_speed*dt);
    }
    if(input.isSet(Input::MoveRight)){
        camera.moveRight(move_speed*dt);
    }
    if(input.isSet(Input::MoveLeft)){
        camera.moveRight(-move_speed*dt);
    }
    if(input.isSet(Input::MoveUp)){
        camera.moveUp(move_speed*dt);
    }
    if(input.isSet(Input::MoveDown)){
        camera.moveUp(-move_speed*dt);
    }
	if(input.isSet(Input::DisableMSAA)){
		GLCALL(glDisable(GL_MULTISAMPLE));
	}else{
		GLCALL(glEnable(GL_MULTISAMPLE));
	}
	camera.update();
}

void gameLoop()
{
	handleEvents();
	updateState();
	draw();
	global_time += dt;
}

int main(int argc, char** argv)
{
	if(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) != 0)
		return -1;

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

	int arg_index = checkArg(argc, argv, "-h");
	if(arg_index != -1 && arg_index < argc-1){
		height = strToInt(argv[arg_index+1]);
		width = height*16/9;
	}
	arg_index = checkArg(argc, argv, "-w");
	if(arg_index != -1 && arg_index < argc-1){
		width = strToInt(argv[arg_index+1]);
	}
	arg_index = checkArg(argc, argv, "-tick_rate");
	if(arg_index != -1 && arg_index < argc-1){
		tick_rate = strToInt(argv[arg_index+1]); 
		dt = 1.f/tick_rate;
	}

	window = SDL_CreateWindow(default_title, SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED, width, height,
				SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, context);

	if(!gladLoadGLLoader(SDL_GL_GetProcAddress)){
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Init error",
				"failed to init GL", 0);
		return -1;
	printf("OpenGL glad %d.%d\n", GLVersion.major, GLVersion.minor);
	}

	if(SDL_GL_SetSwapInterval(0) != 0){
		printf("failed to set vsync to %d\n", 0);
		//return -1;
	}

	printf("\tVendor: %s\n", glGetString (GL_VENDOR));
	printf("\tRenderer: %s\n", glGetString (GL_RENDERER));
	printf("\tVersion: %s\n", glGetString (GL_VERSION));
	printf("\tGLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	initState();

	CLK loop_clock;
	CLK perf_clock;

	float accumulator_time = 0.f;
	float perf_time = 0.f;
	int frames = 0;

	loop_clock.start();
	perf_clock.start();
	while(!quit){
		accumulator_time = loop_clock.time();
		if(accumulator_time > dt){
			perf_time += loop_clock.restart();

			gameLoop();

			accumulator_time -= dt;
			++frames;
			last_frame_time = loop_clock.time();
		}else{
			//SDL_Delay((int)((client.dt-accumulator_time)*1000.f));
		}
		if(perf_clock.time() > 1.f){
			float avg_time = perf_time/frames; 
			//printf("fps: %d   avg time: %f\n", frames, avg_time);
			sprintf(avg_fps_str, "fps: %d   avg time: %.6f", frames, avg_time);
			char wtitle[256] = {};
			sprintf(wtitle, "%s  |  %s", default_title, avg_fps_str);
			SDL_SetWindowTitle(window, wtitle);	
			// printf("DEBUG: FPS: %d\tAVG time: %.6f\n", frames, avg_time);
			perf_time = 0;
			frames = 0;
			perf_clock.restart();
		}
	}
	return 0;
}
