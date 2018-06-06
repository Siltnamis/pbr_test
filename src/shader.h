#pragma once

#include <string.h>
#include "av/av.h"
#include "opengl.h"

#define MAX_UNIFORMS 32

struct Shader
{
    GLuint  program = 0;
    GLuint  vertex = 0;
    GLuint  fragment = 0;
    GLuint  geometry = 0;
    int8    uniform_count = 0;

    GLint   uniforms[MAX_UNIFORMS];
    char*   uniform_names[MAX_UNIFORMS]; 

    GLint getUniform(const char* name)
    {
        for(int i = 0; i < uniform_count; ++i){
            if(uniform_names[i]){
                if( strcmp(uniform_names[i], name) == 0 ){
                    return uniforms[i];
                }
            }
        }
        return -1;
    }
        
    void addUniforms(int num, ...) 
    {
        uniform_count = num;
        va_list va;
        va_start(va, num);
        for(int i = 0; i < num; ++i){
            char* name = va_arg(va, char*);
            i32 len = strlen(name);
            uniform_names[i] = (char*)calloc(len+1, sizeof(char));
            strcpy(uniform_names[i], name);
            GLCALL(uniforms[i] = glGetUniformLocation(program, name));
            if(uniforms[i] == -1){
                printf("Program:%d\tno uniform variable named %s\n",
                       program, name);
            }
        } 
        va_end(va); 
    }

    void clearUniforms()
    {
        for(int i = 0; i < uniform_count; ++i){
            uniforms[i] = 0;
            if(uniform_names[i]){
                free(uniform_names[i]);
            }
            uniform_names[i] = NULL;
        }
        uniform_count = 0;
    }

    bool loadFromFile(const char* file_name)
    {
        FILE* source = fopen(file_name, "r");
        if(!source)
        {
            printf("Failed to open file: %s\n", file_name);
            return false;
        }

        fseek(source, 0, SEEK_END);
        u32 size = ftell(source);
        fseek(source, 0, SEEK_SET);

        char* vs_str = (char*)calloc(size+1, 1);
        char* fs_str = (char*)calloc(size+1, 1);
        char* gs_str = (char*)calloc(size+1, 1);

        char* current_str = nullptr;

        char line[512] = {0};
        int line_count = 0;
        while(fgets(line, 512, source))
        {
            ++line_count;
            if(strstr(line, "#VERTEX") != nullptr)
            {
                current_str = vs_str;
                continue;
            }
            else if(strstr(line, "#FRAGMENT") != nullptr)
            {
                current_str = fs_str;
                continue;
            }
            else if (strstr(line, "#GEOMETRY") != nullptr)
            {
                current_str = gs_str;
                continue;
            }

            strncat(current_str, line, 512);
        }
        loadFromString(GL_VERTEX_SHADER, vs_str, file_name);
        loadFromString(GL_FRAGMENT_SHADER, fs_str, file_name);
        if(gs_str[0] != 0)
            loadFromString(GL_GEOMETRY_SHADER, gs_str, file_name);

        createAndLinkProgram();

        free(vs_str);
        free(fs_str);
        free(gs_str);
        return true;
    }


    bool loadFromFile(GLenum type, const char* file_name)
    {   
        bool status = false;
        FILE* source = fopen(file_name, "r"); 
        if(source){
            fseek(source, 0, SEEK_END); 
            u32 size = ftell(source);
            fseek(source, 0, SEEK_SET);

            char* source_str = (char*)calloc(size+1, sizeof(char));
            status = ( fread(source_str, sizeof(char), size, source) == size );
            loadFromString(type, source_str, file_name); 

            free(source_str);
            fclose(source);
        } else {
            printf("Failed to open file: %s\n", file_name);
        }
        return status;
    }

    void loadFromString(GLenum type, const char* source, const char* fname)   
    {
        char vertex_name[]  = "vertex";
        char fragment_name[]= "fragment";
        char geometry_name[]= "geometry";
        char* shader_name;
        GLCALL(GLuint shader = glCreateShader(type));

        if(type == GL_VERTEX_SHADER){
            vertex = shader;
            shader_name = vertex_name;
        } else if(type == GL_FRAGMENT_SHADER){
            fragment = shader;
            shader_name = fragment_name;
        } 
        else if(type == GL_GEOMETRY_SHADER){
            geometry = shader;
            shader_name = geometry_name;
        }

        GLCALL(glShaderSource(shader, 1,&source, NULL));
        GLint status;
        GLCALL(glCompileShader(shader));
        GLCALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &status));
        if(status == GL_FALSE){
            GLint log_length;
            GLCALL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length));
            char* info_log = (char*)calloc(log_length+1, sizeof(char));
            GLCALL(glGetShaderInfoLog(shader, log_length, NULL, info_log));
            printf("Compile failure in %s shader(%s):\n%s\n", shader_name,
                    fname, info_log);
            free(info_log);
        }
    }

    void createAndLinkProgram()
    {
        GLCALL(program = glCreateProgram());
        if(vertex)   {GLCALL(glAttachShader(program, vertex));}
        if(geometry) {GLCALL(glAttachShader(program, geometry));}
        if(fragment) {GLCALL(glAttachShader(program, fragment));}

        GLint status;
        GLCALL(glLinkProgram(program));
        GLCALL(glGetProgramiv(program, GL_LINK_STATUS, &status));
        if(status == GL_FALSE){
            GLint log_length;  
            GLCALL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length));
            char* info_log = (char*)calloc(log_length+1, sizeof(char));
            GLCALL(glGetProgramInfoLog(program, log_length, NULL, info_log));
            printf("Linker failure: %s\n", info_log);
            free(info_log);
        }
        if(vertex){
            GLCALL(glDetachShader(program, vertex));
            GLCALL(glDeleteShader(vertex));
            vertex = 0;
        }
        if(fragment){
            GLCALL(glDetachShader(program, fragment));
            GLCALL(glDeleteShader(fragment));
            fragment = 0;
        }
        if(geometry){
            GLCALL(glDetachShader(program, geometry));
            GLCALL(glDeleteShader(geometry));
            geometry = 0;
        }
    } 

    void deleteShaderProgram()
    {
        if (program == 0)
            return;
        GLCALL(glDeleteProgram(program));
        program = 0;
        clearUniforms();
    }

    void use()
    {
        GLCALL(glUseProgram(program));
    }
    void unuse()
    {
        GLCALL(glUseProgram(0));
    }

    void setUniform(const char* name, mat4 matrix)
    {
        GLCALL(glUniformMatrix4fv(getUniform(name), 1, GL_FALSE, matrix.e));
    }

    void setUniform(const char* name, vec3 v)
    {
        GLCALL(glUniform3f(getUniform(name), v.x, v.y, v.z));
    }

    void setUniform(const char* name, int count, const vec3* v)
    {
        GLCALL(glUniform3fv(getUniform(name), count, v->e));
    }

    void setUniform(const char* name, int i)
    {
        GLCALL(glUniform1i(getUniform(name), i));
    }

    void setUniform(const char* name, int count, const int* i)
    {
        GLCALL(glUniform1iv(getUniform(name), count, i));
    }

    void setUniform(const char* name, float f)
    {
        GLCALL(glUniform1f(getUniform(name), f));
    }

    void setUniform(const char* name, int count, const float* f)
    {
        GLCALL(glUniform1fv(getUniform(name), count, f));
    }
};
