#include "particles.h"

void particleBufferCreate(ParticleBuffer* pbuffer, int pcount)
{
    //assert(false);
    glGenVertexArrays(1, &pbuffer->vao);
    glBindVertexArray(pbuffer->vao);

    glGenBuffers(1, &pbuffer->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, pbuffer->vbo);

    glEnableVertexAttribArray(0); // positions
    glEnableVertexAttribArray(1); // rotations
    glEnableVertexAttribArray(2); // scales
    glEnableVertexAttribArray(3); // uv1
    glEnableVertexAttribArray(4); // uv2
    glEnableVertexAttribArray(5); // blend

    GLintptr rotation_offset = sizeof(vec3)*MAX_PARTICLES; 
    GLintptr scale_offset = rotation_offset + sizeof(float)*MAX_PARTICLES;
    GLintptr uv1_offset =  scale_offset + sizeof(vec2)*MAX_PARTICLES;
    GLintptr uv2_offset = uv1_offset + sizeof(vec2)*MAX_PARTICLES;
    GLintptr blend_offset = uv2_offset + sizeof(vec2)*MAX_PARTICLES;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), 
            (const void*)rotation_offset);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 
            (const void*)scale_offset);

    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 
            (const void*)uv1_offset);

    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 
            (const void*)uv2_offset);

    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(float), 
            (const void*)blend_offset);


    glBufferData(GL_ARRAY_BUFFER, 
            blend_offset+sizeof(float)*MAX_PARTICLES,
            nullptr, GL_DYNAMIC_DRAW);
    printf(" PARTICLE BUFFER SIZE: %lu\n", blend_offset+sizeof(float)*MAX_PARTICLES);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    pbuffer->particle_count = pcount;
}

void particleBufferDestroy(ParticleBuffer* pbuffer)
{
    glDeleteBuffers(1, &pbuffer->vbo);
    glDeleteVertexArrays(1, &pbuffer->vao);
}

void particleBufferUpdate(ParticleBuffer* pbuffer, Particles* particles)
{
    GLintptr rotation_offset = sizeof(vec3)*MAX_PARTICLES; 
    GLintptr scale_offset = rotation_offset + sizeof(float)*MAX_PARTICLES;
    GLintptr uv1_offset =  scale_offset + sizeof(vec2)*MAX_PARTICLES;
    GLintptr uv2_offset =  uv1_offset + sizeof(vec2)*MAX_PARTICLES;
    GLintptr blend_offset =  uv2_offset + sizeof(vec2)*MAX_PARTICLES;

    glBindBuffer(GL_ARRAY_BUFFER, pbuffer->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3)*MAX_PARTICLES, particles->pos);

    glBufferSubData(GL_ARRAY_BUFFER, rotation_offset, 
            sizeof(float)*MAX_PARTICLES, particles->rot);

    glBufferSubData(GL_ARRAY_BUFFER, scale_offset, 
            sizeof(vec2)*MAX_PARTICLES, particles->sca);

    glBufferSubData(GL_ARRAY_BUFFER, uv1_offset, 
            sizeof(vec2)*MAX_PARTICLES, particles->uv1);

    glBufferSubData(GL_ARRAY_BUFFER, uv2_offset, 
            sizeof(vec2)*MAX_PARTICLES, particles->uv2);

    glBufferSubData(GL_ARRAY_BUFFER, blend_offset, 
            sizeof(float)*MAX_PARTICLES, particles->blend);

}
