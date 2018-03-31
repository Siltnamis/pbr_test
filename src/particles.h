#pragma once

#include "av/av.h"
#include "av/avmath.h"

#include "opengl.h"

//particle vertex has:
//position, rotation, scale, texture coordinate?
#define MAX_PARTICLES 10240
struct Particles
{
    vec3    pos[MAX_PARTICLES];
    float   rot[MAX_PARTICLES];
    vec2    sca[MAX_PARTICLES]; //just one float?
    vec2    uv1[MAX_PARTICLES]; //current uv
    vec2    uv2[MAX_PARTICLES]; //next uv
    float   blend[MAX_PARTICLES]; //blend
    vec3    vel[MAX_PARTICLES];
    float   life[MAX_PARTICLES];
    float   time[MAX_PARTICLES];
    int64   num_particles;

    Particles()
    {
        //*this = {};
        for(int i = 0; i < MAX_PARTICLES; ++i){
            this->time[i] = 9999999.f;
        }
        num_particles = 0;
    }
};

struct ParticleBuffer
{
    GLuint  vao;
    GLuint  vbo;
    int     particle_count;
};


struct ParticleEmitter
{
    vec3    start_velocity;
    vec3    position;
    float   particle_scale;
    float   particle_lifetime;
    float   emit_rate;
    float   time_since_emit; 
};

void particleBufferCreate(ParticleBuffer* pbuffer, int pcount);
void particleBufferDestroy(ParticleBuffer* pbuffer);
void particleBufferUpdate(ParticleBuffer* pbuffer, Particles* particles);

static vec2 getUV(int rows, int index)
{
    vec2 result;
    int column = index%rows;
    int row = index/rows;

    result.x = (float)column/rows;
    result.y = (float)row/rows;
    
    return result;
}

inline void simParticlesDefault(Particles* particles, float dt = 1.f/60.f)
{
#if 0
    int p = 0;
    float theta = 0;
    for(int i = 0; i < 100; ++i){
        theta = radians(i/100.f);
        for(int j = 0; j < 100; ++j){
            particles->pos[p] = {j*0.003f, i*0.003f, 0.1f};
            particles->sca[p] = {0.01, 0.01};
            ++p;
        }
    } 
#else
    static ParticleEmitter pemitter;
    static bool inited = false;
    if(!inited){ 
        //pemitter.position = {0.33, 0.3, 1.0};
        pemitter.position = {0.0, 0.0, 0.0};
        pemitter.particle_lifetime = 0.8f;
        pemitter.particle_scale = 0.400f;
        pemitter.emit_rate = 0.01f;
        //pemitter.emit_rate = 0.5;
        pemitter.start_velocity =  vec3{0, 0, 0.8};
        pemitter.time_since_emit = pemitter.emit_rate;
        inited = true;
        *particles = {};
        for(int i = 0; i < MAX_PARTICLES; ++i){
            particles->time[i] = 9999999.f;
        }
    }

    int particle_emmit_count = 2;
    ParticleEmitter* emitter = &pemitter;
    static float grav = -2.f;
    emitter->time_since_emit += dt;
    //emmit
    while(emitter->time_since_emit > emitter->emit_rate){
        emitter->time_since_emit -= emitter->emit_rate;
        int emitted = 0;
        for(i32 i = 0; i < MAX_PARTICLES; ++i){
            if(particles->time[i] > emitter->particle_lifetime){
                float velang = (rand()%6283)/1000.f;
                float velmag = (rand()%1000)/1000.f + 0.5;
                emitter->start_velocity = vec3{0.1f*cos(velang), 0.1f*sin(velang), 
                    emitter->start_velocity.z}; 

                particles->pos[i] = emitter->position;
                particles->vel[i] = emitter->start_velocity;
                particles->vel[i].z *= velmag;

                particles->sca[i] = {emitter->particle_scale, emitter->particle_scale};
                //particles->rot[i] = (rand()%6283)/1000.f;
                particles->rot[i] = 0.f;
                particles->time[i] = 0.f;
                particles->uv1[i] = getUV(4, 0);
                particles->uv2[i] = getUV(4, 13);
                particles->blend[i] = 0;
                ++(particles->num_particles);
                ++emitted;
                if(emitted == particle_emmit_count || 
                        particles->num_particles == MAX_PARTICLES)
                    break;
            }
        }        
    }
    //update
    for(i32 i = 0; i < MAX_PARTICLES; ++i){
        //if(particles->alive[i]){
        if(particles->time[i] < emitter->particle_lifetime){
            particles->pos[i] += particles->vel[i] * dt;
            //particles->pos[i] += vec3{0.03, 0, 0} * dt;
            particles->vel[i] += vec3{0, 0, 0.1f*grav} * dt;
            vec3 origindir = emitter->position - particles->pos[i];
            //origindir.xy = normalize(origindir.xy); 
            //particles->vel[i].xy += 20.f * origindir.xy * dt;
            particles->rot[i] += 0*dt;
            particles->time[i] += dt;

            float life_factor = particles->time[i] / emitter->particle_lifetime;
            int stage_count = 4*4;
            float progression = life_factor * stage_count;
            int index1 = (int)floor(progression);
            int index2 = index1 < stage_count -1 ? index1 + 1 : index1;
            particles->blend[i] = fmod(progression, 1);
            particles->uv1[i] = getUV(4, index1);
            particles->uv2[i] = getUV(4, index2);
            if(particles->time[i] > emitter->particle_lifetime){
                --(particles->num_particles);
                particles->pos[i] = {-999, -999, -999};
            }
        }        
    }
#endif
}

inline void simSparkParticles(Particles* particles, vec3 start_point, vec3 end_point, float dt = 1.f/60.f)
{
    static ParticleEmitter pemitter;
    static bool inited = false;
    if(!inited){ 
        //pemitter.position = {0.33, 0.3, 1.0};
        pemitter.particle_lifetime = 0.5f;
        pemitter.particle_scale = 0.300f;
        pemitter.emit_rate = 0.005f;
        //pemitter.emit_rate = 0.5;
        pemitter.start_velocity =  vec3{0, 0, 3.0};
        pemitter.time_since_emit = pemitter.emit_rate;
        inited = true;
        
    }
    pemitter.position = start_point;
    
    vec3 sdir = normalize(end_point - start_point);
    int particle_emmit_count = 5;
    ParticleEmitter* emitter = &pemitter;
    emitter->time_since_emit += dt;
    while(emitter->time_since_emit > emitter->emit_rate){
        emitter->time_since_emit -= emitter->emit_rate;
        int emitted = 0;
        for(i32 i = 0; i < MAX_PARTICLES; ++i){
            if(particles->time[i] > emitter->particle_lifetime){
                float velang = (rand()%6283)/1000.f;
                float velmag = (rand()%1000)/1000.f + 0.5;
                emitter->start_velocity = sdir*velmag; 

                particles->pos[i] = emitter->position;
                particles->pos[i] += i*sdir*0.01;
                particles->vel[i] = emitter->start_velocity;
                //particles->vel[i].z *= velmag;

                particles->sca[i] = {emitter->particle_scale, emitter->particle_scale};
                //particles->rot[i] = (rand()%6283)/1000.f;
                particles->rot[i] = 0.f;
                particles->time[i] = 0.f;
                particles->uv1[i] = getUV(4, 0);
                particles->uv2[i] = getUV(4, 13);
                particles->blend[i] = 0;
                ++(particles->num_particles);
                ++emitted;
                if(emitted == particle_emmit_count || 
                        particles->num_particles == MAX_PARTICLES)
                    break;
            }
        }        
    }

    for(i32 i = 0; i < MAX_PARTICLES; ++i){
        //if(particles->alive[i]){
        if(particles->time[i] < emitter->particle_lifetime){
            particles->pos[i] += particles->vel[i] * dt;
            //particles->pos[i] += vec3{0.03, 0, 0} * dt;
            //particles->vel[i] += vec3{0, 0, 0.1f*grav} * dt;
            vec3 dir = normalize(end_point - particles->pos[i]);
            particles->vel[i] += dir*rand()/(float)RAND_MAX;
            particles->vel[i] += {0.1f*rand()/(float)RAND_MAX, 0.1f*rand()/(float)RAND_MAX,
                                    0.1f*rand()/(float)RAND_MAX};
            vec3 origindir = emitter->position - particles->pos[i];
            //origindir.xy = normalize(origindir.xy); 
            //particles->vel[i].xy += 20.f * origindir.xy * dt;
            particles->rot[i] += 0*dt;
            particles->time[i] += dt;

            float life_factor = particles->time[i] / emitter->particle_lifetime;
            int stage_count = 4*4;
            float progression = life_factor * stage_count;
            int index1 = (int)floor(progression);
            int index2 = index1 < stage_count -1 ? index1 + 1 : index1;
            particles->blend[i] = fmod(progression, 1);
            particles->uv1[i] = getUV(4, index1);
            particles->uv2[i] = getUV(4, index2);
            if(particles->time[i] > emitter->particle_lifetime){
                --(particles->num_particles);
                particles->pos[i] = {-999, -999, -999};
            }
        }        
    }
}