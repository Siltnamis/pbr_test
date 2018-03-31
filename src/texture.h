#pragma once
#include "av/av.h"
#include "opengl.h"

enum TextureFlags_Enum
{
    TEXTURE_MIPMAP      = (1 << 0),
    TEXTURE_SRGB        = (1 << 1),
    TEXTURE_ANISOTROPIC = (1 << 2),
    TEXTURE_GREYSCALE   = (1 << 3)
};

enum CubeMap_Enum   
{
    CubeMap_Right, CubeMap_Left,
    CubeMap_Up, CubeMap_Down,
    CubeMap_Back, CubeMap_Front
};
//hopefuly a saner version
struct Image
{
    char*   data;    
    int     w;
    int     h;
    int     components; //maybe deal only with rgba?
};

int  imageLoad(Image* image, const char* file_name);
void imageFree(Image* image);

//returns 0 on failure, some int on success
GLuint texture2DLoadBlackTexture();
GLuint texture2DLoadWhiteTexture();
GLuint texture2DLoad(Image* image, int flags = 0b111);
GLuint texture2DLoad(const char* file_name, int flags = 0b111);
GLuint textureCubeMapLoad(Image images[6], int flags = 0b010);
GLuint textureCubeMapLoad(const char* files[6], int flags = 0b010);
GLuint textureFree(GLuint texture);
void   texture2DBind(GLuint texture, int target = 0);
void   textureCubeMapBind(GLuint texture, int target = 0);
