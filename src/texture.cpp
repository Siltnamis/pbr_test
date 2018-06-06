#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

int imageLoad(Image* image, const char* file_name)
{
    image->data = (char*)stbi_load(file_name, &image->w, &image->h, &image->components, 4);
    if(image->data == NULL){
        return -1; 
    }
    return 0;
}

void imageFree(Image* image)
{
    stbi_image_free(image->data);
}

void setTextureParams2D(int flags)
{
    //Set anisotropic filtering
    //NOTE::check if supported?
    if(flags & TEXTURE_ANISOTROPIC){
        float amount;
        GLCALL(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &amount));
        GLCALL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fmin(4.f,amount)));
    }

 	//set mipmap base and max level
    //defualts are good enaugh probably
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 2);
    //generate mipmaps
    GLenum min_filter = GL_LINEAR;
    if(flags & TEXTURE_MIPMAP){
        GLCALL(glGenerateMipmap(GL_TEXTURE_2D));
        min_filter = GL_LINEAR_MIPMAP_LINEAR;
    }
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
}

GLuint texture2DLoad(Image* image, int flags)
{
    GLuint texture;
    GLCALL(glGenTextures(1, &texture));
    GLCALL(glBindTexture(GL_TEXTURE_2D, texture));
    GLenum internal_format = GL_RGBA;
    // GLenum internal_format = GL_COMPRESSED_RGBA;
    if(flags & TEXTURE_SRGB)
        // internal_format = GL_COMPRESSED_SRGB_ALPHA;
        internal_format = GL_SRGB8_ALPHA8;
    GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, internal_format, image->w, image->h, 0, 
            GL_RGBA, GL_UNSIGNED_BYTE, image->data));
    
    setTextureParams2D(flags);
    return texture;
}

GLuint texture2DLoad(const char* file_name, int flags)
{
    GLuint texture;
    Image img;
    int status = imageLoad(&img, file_name); 
    if(status != 0)
        return 0;
    texture = texture2DLoad(&img, flags);
    imageFree(&img);
    return texture;
};

GLuint textureCubeMapLoad(Image images[6], int flags)
{
    GLuint texture;
    GLCALL(glGenTextures(1, &texture));
    GLCALL(glBindTexture(GL_TEXTURE_CUBE_MAP, texture));
    GLenum internal_format = GL_RGBA;
    if(flags & TEXTURE_SRGB)
        internal_format = GL_SRGB8_ALPHA8;
    for(int i = 0; i < 6; ++i){
        GLCALL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal_format,
                images[i].w, images[i].h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                images[i].data));
    }
    GLCALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));

    return texture;

}
GLuint textureCubeMapLoad(const char* files[6], int flags)
{
    GLuint texture;
    Image images[6];
    int status;
    for(int i = 0; i < 6; ++i){
        status = imageLoad(&images[i], files[i]);
        if(status != 0){
            for(int j = 0; j < i; ++j)
                imageFree(&images[j]);
            return 0;
        }
    }
    texture = textureCubeMapLoad(images, flags);
    for(int i = 0; i < 6; ++i){
        imageFree(&images[i]);
    }
    return texture;
}

GLuint texture2DLoadBlackTexture()
{
    static GLuint texture = 0;
    if(texture != 0)
        return texture;

    char pixel = 0;
    GLCALL(glGenTextures(1, &texture));
    GLCALL(glBindTexture(GL_TEXTURE_2D, texture));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 1, 1, 0, 
            GL_RED, GL_UNSIGNED_BYTE, &pixel));
    return texture;
}

GLuint texture2DLoadWhiteTexture()
{
    static GLuint texture = 0;
    if(texture != 0)
        return texture;

    unsigned char pixel = 255;
    GLCALL(glGenTextures(1, &texture));
    GLCALL(glBindTexture(GL_TEXTURE_2D, texture));
    GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 1, 1, 0, 
            GL_RED, GL_UNSIGNED_BYTE, &pixel));
    return texture;
}

void textureCubeMapBind(GLuint texture, int target)
{
    GLCALL(glActiveTexture(GL_TEXTURE0+target));
    GLCALL(glBindTexture(GL_TEXTURE_CUBE_MAP, texture));
}

void texture2DBind(GLuint texture, int target)
{
    GLCALL(glActiveTexture(GL_TEXTURE0+target));
    GLCALL(glBindTexture(GL_TEXTURE_2D, texture));
}

GLuint textureFree(GLuint texture)
{
    GLCALL(glDeleteTextures(1, &texture));
    return texture;
}
