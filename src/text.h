#pragma once
#include "stb/stb_truetype.h"

#include "mesh.h"

struct Font
{
    GLuint          texture;
    stbtt_bakedchar cdata[96];
    float           height;
};

struct Text
{
    Font*           font;
    const char*     str;
    float           size; //height of a char
    vec2            pos;
    vec4            color;
};

struct TextBuffer
{
    GLuint      vao;
    GLuint      vbo;
    Vertex2D*   vert_buff; 
    int         vertex_count;
};

int fontLoad(Font* font, const char* file_name, float pixel_height);
void fontFree(Font* font);

void textBufferCreate(TextBuffer* tbuffer, int size);
void textBufferDestroy(TextBuffer* tbuffer);

void textBufferBegin(TextBuffer* tbuffer);
void textBufferPush(TextBuffer* tbuffer, Text* text);
void textBufferEnd(TextBuffer* tbuffer);
