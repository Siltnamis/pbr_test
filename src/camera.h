#pragma once

#include "av/avmath.h"

struct Camera2
{
    vec3 position;
    vec3 look;
    vec3 up;
    vec3 right;

    mat4 projection;
    mat4 view;

    void rotate(float yaw, float pitch, float roll)
    {

        float cosy = cosf(yaw);
        float cosp = cosf(pitch);
        float siny = sinf(yaw);
        float sinp = sinf(pitch);

        look.x = cosy*cosp;


#if 1
        look.y = sinp;
        look.z = siny*cosp;
#else
        look.z = sinp;
        look.y = siny*cosp;
#endif
    }
};

struct Camera
{
    Camera()
    {
        yaw = radians(-90.f);
        pitch = 0.f;
    }
//members
    vec3    position;
    vec3    look;
    vec3    up;
    vec3    right;

    float   yaw;
    float   pitch; 
    //quat    oriantation;
    
    //
    float   fov;
    float   aspect_ratio;
    float   near_plane;
    float   far_plane;

    mat4    projection;
    mat4    view;

    void setupProjection(float fov, float aspectRatio, float nearPlane, float farPlane)
    {
        this->fov       = fov;
        aspect_ratio    = aspectRatio; 
        near_plane      = nearPlane; 
        far_plane       = farPlane;
        projection      = perspective(fov, aspect_ratio, near_plane, far_plane);
    }

    mat4 getViewProjectionMatrix()const
    {
        return projection * view;
    }

    mat4 getViewMatrix()const
    {
        return view;
    }

    void move(float amt, vec3 dir)
    {
        position += dir * amt; 
    }

    void moveUp(float amt)
    {
        position += up * amt; 
    }

    void moveRight(float amt)
    {
        position += right * amt; 
    }

    void moveForward(float amt)
    {
        position += look * amt;
    }


    void update()
    {
        

        float hsiny = sinf(yaw/2.f);
        float hcosy = cosf(yaw/2.f);
        float hsinp = sinf(pitch/2.f);
        float hcosp = cosf(pitch/2.f);

        

#if 1
        if(yaw > 2*PI)
            yaw -= 2*PI;
        if(yaw < -2*PI)
            yaw += 2*PI;

        float cosy = cosf(yaw);
        float cosp = cosf(pitch);
        float siny = sinf(yaw);
        float sinp = sinf(pitch);

//        look.x = cosy*cosp;
//        look.y = sinp;
//        look.z = siny*cosp; 
        look.x = cosy*cosp;
#if 1
        look.y = siny*cosp; 
        look.z = sinp;
#else
        look.y = sinp;
        look.z = siny*cosp;
#endif
#else
        
        
        quat pitchrot = {1*hsinp, 0*hsinp, 0*hsinp, hcosp};
        quat yawrot = {0*hsiny, 1*hsiny, 0*hsiny, hcosy};
        
        //quat rot; av_mul(&rot, pitchrot, yawrot);

        quat con;
        av_quat_conj(&con, pitchrot);
        //av_quat_conj(&con, rot);

        quat look_quat = {0, 0, -1, 0};
        quat q;

        //av_mul(&q, rot, look_quat);
        //av_mul(&q, q, con);

        q = pitchrot*look_quat*con;
        
        av_quat_conj(&con, yawrot);
        q = yawrot*q*con;

        look = {q.x, q.y, q.z};
    
#endif
        right = cross(look, up);
        
        look = normalize(look);
        up = normalize(up);
        right = normalize(right);
        view = look_at(position, position + look, up); 
    }
    void rotate(vec3 v, float angle)
    {
#if 0
        float hangle = angle/2.f;
        float hcos = cosf(hangle);
        float hsin = sinf(hangle);
        av_normalize(&v, v); 
        quat rotation = {v.x*hsin, v.y*hsin, v.z*hsin, hcos};
        quat conjugate;
        av_quat_conj(&conjugate, rotation);
        quat qlook = {look.x, look.y, look.z, 0};
        quat q;
        av_mul(&q, rotation, qlook);
        av_mul(&q, q, conjugate);
        look = {q.x, q.y, q.z};
        av_normalize(&look, look);
        right = av_cross(look, up);
#endif
    }
};
