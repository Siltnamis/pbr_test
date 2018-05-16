#pragma once

#if defined (AV_GL_GLAD)
	#include "glad/glad.h"
#elif defined (AV_GL_GLEW)
	#include <GL/glew.h>
#else
	#include <GLES3/gl3.h>
	#include <GLES3/gl2ext.h>
#endif

#ifdef NDEBUG
	#define GLCALL(x) x;	
#else
	// #define GLCALL(x) x;	
	#define GLCALL(x) GLClearError();\
		x;\

		// if(!GLCheckError(#x, __FILE__, __LINE__)) asm("int $3");
		//assert(GLCheckError(#x, __FILE__, __LINE__))
#endif


static inline void GLClearError()
{
	while(glGetError() != GL_NO_ERROR);
}

static inline bool GLCheckError(const char* function, const char* file, int line)
{
	if(GLenum error = glGetError())
	{
		printf("[OpenGL error] (%d): %s %s:%d\n",
			error, function, file, line);
		return false;
	}
	return true;
}