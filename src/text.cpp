#define STB_TRUETYPE_IMPLEMENTATION
#include "text.h"

int fontLoad(Font* font, const char* file_name, float pixel_height)
{
    int result = 0;
    unsigned char* ttf_buffer = (unsigned char*)malloc(Megabytes(1));
    unsigned char* temp_bitmap = (unsigned char*)malloc(512*512);
    FILE* file = fopen(file_name, "rb");
    if(file){
        fread(ttf_buffer, 1, Megabytes(1), file);
        stbtt_BakeFontBitmap(ttf_buffer, 0, pixel_height, temp_bitmap,
                512, 512, 32, 96, font->cdata);

        GLCALL(glGenTextures(1, &font->texture));
        GLCALL(glBindTexture(GL_TEXTURE_2D, font->texture));
        GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE,
                temp_bitmap));
        GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));

        font->height = pixel_height;
    } else {
        result = -1;
    }

    free(temp_bitmap);
    free(ttf_buffer);
    return result;
}

void fontFree(Font* font)
{
    GLCALL(glDeleteTextures(1, &font->texture));
};

void textBufferCreate(TextBuffer* tbuffer, int size)
{
    GLCALL(glGenVertexArrays(1, &tbuffer->vao));
    GLCALL(glBindVertexArray(tbuffer->vao));

    GLCALL(glGenBuffers(1, &tbuffer->vbo));
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, tbuffer->vbo));

    GLCALL(glEnableVertexAttribArray(0));
    GLCALL(glEnableVertexAttribArray(1));
    GLCALL(glEnableVertexAttribArray(2));

    GLCALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), 0));
    GLCALL(glEnableVertexAttribArray(1));
    GLCALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D),
                (const void*)offsetof(Vertex2D, uv)));
    GLCALL(glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2D),
                (const void*)offsetof(Vertex2D, color)));

    GLCALL(glBufferData(GL_ARRAY_BUFFER, 6*size*sizeof(Vertex2D), NULL, GL_STREAM_DRAW));
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCALL(glBindVertexArray(0));

    tbuffer->vertex_count = 0;
}

void textBufferDestroy(TextBuffer* tbuffer)
{
    GLCALL(glDeleteBuffers(1, &tbuffer->vbo));
    GLCALL(glDeleteVertexArrays(1, &tbuffer->vao));
}


void textBufferBegin(TextBuffer* tbuffer)
{
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, tbuffer->vbo));
    GLCALL(tbuffer->vert_buff = (Vertex2D*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
    tbuffer->vertex_count = 0;
}


void textBufferPush(TextBuffer* tbuffer, Text* text)
{
    float tx = text->pos.x; 
    float ty = text->pos.y;
    const char* str = text->str;
    float scale = text->size/text->font->height;
    vec2 pos_offset;
    pos_offset = vec2{tx, ty} - vec2{tx, ty}*scale;
    while(*str){
        if(*str >= 32 && *str < 128){
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(text->font->cdata, 512, 512, *str-32, &tx, &ty, &q, 1);

            tbuffer->vert_buff->pos = vec2{q.x0, q.y0}*scale + pos_offset; 
            tbuffer->vert_buff->uv = {q.s0, q.t0}; 
            tbuffer->vert_buff->color = text->color;
            ++(tbuffer->vert_buff);

            tbuffer->vert_buff->pos = vec2{q.x0, q.y1}*scale + pos_offset; 
            tbuffer->vert_buff->uv = {q.s0, q.t1}; 
            tbuffer->vert_buff->color = text->color;
            ++(tbuffer->vert_buff);

            tbuffer->vert_buff->pos = vec2{q.x1, q.y1}*scale + pos_offset; 
            tbuffer->vert_buff->uv = {q.s1, q.t1}; 
            tbuffer->vert_buff->color = text->color;
            ++(tbuffer->vert_buff);

            tbuffer->vert_buff->pos = vec2{q.x0, q.y0}*scale + pos_offset; 
            tbuffer->vert_buff->uv = {q.s0, q.t0}; 
            tbuffer->vert_buff->color = text->color;
            ++(tbuffer->vert_buff);

            tbuffer->vert_buff->pos = vec2{q.x1, q.y1}*scale + pos_offset; 
            tbuffer->vert_buff->uv = {q.s1, q.t1}; 
            tbuffer->vert_buff->color = text->color;
            ++(tbuffer->vert_buff);

            tbuffer->vert_buff->pos = vec2{q.x1, q.y0}*scale + pos_offset; 
            tbuffer->vert_buff->uv = {q.s1, q.t0}; 
            tbuffer->vert_buff->color = text->color;
            ++(tbuffer->vert_buff);

            tbuffer->vertex_count += 6;
        } else if(*str == '\n'){
            //ty += text->size;
            ty += text->font->height;
            tx = text->pos.x;
        }
        ++str;
    }
}

void textBufferEnd(TextBuffer* tbuffer)
{
    GLCALL(glBindBuffer(GL_ARRAY_BUFFER, tbuffer->vbo));
    GLCALL(glUnmapBuffer(GL_ARRAY_BUFFER));
}