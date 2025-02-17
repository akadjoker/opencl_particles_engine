

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200
#define CL_TARGET_OPENCL_VERSION 200


#include <iostream>
#include <vector>
#include <random>

#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#define __CL_ENABLE_EXCEPTIONS
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <stdio.h>
#include <stdlib.h>
#include "Shader.hpp"
#include "Camera.hpp"
#include "Grid.hpp"
#include "Time.hpp"
#include "Kernel.hpp"
#include "Mesh.hpp"
#include "Font.hpp"


static bool WINDO_RESIZE = true;

void window_resize_callback(GLFWwindow* window, int width, int height)
{
    (void) window;
    (void) width;
    (void) height;
    glViewport(0, 0, width, height);

    WINDO_RESIZE = true;
}

// Shader mais simples para pontos
const char* vertex_shader_source = R"(
    #version 440 core
    layout(location = 0) in vec3 position;
    layout(location = 1) in vec4 color;
    

    uniform mat4 view;
    uniform mat4 projection;

    out vec4 vertexColor;

    void main() 
    {
        gl_Position =  projection * view * vec4(position, 1.0);
        vertexColor = color;
    }
    )";

const char* fragment_shader_source = R"(
    #version 440 core
    out vec4 FragColor;
    in vec4 vertexColor;
    void main() 
    {
        FragColor = vec4(vertexColor);   
    }
//     )";
// const char* kernel_init = R"(


//      float rand_float(int seed)
//      {
//         seed = (seed << 13) ^ seed;
//         return (1.0f - ((seed * (seed * seed * 15731 + 789221) + 1376312589)
//         & 0x7fffffff) / 1073741824.0f);
//     }


//         __kernel void create_points(__global float* points,__global float*
//         velocity,__global float* lifetime, int count, int shape)
//         {
//             int gid = get_global_id(0);
//             int index6 = gid * 6; // Cada partícula tem 3 posições + 3 cores
//             int index3 = gid * 3;// Cada partícula tem 3 posições


//               // Gerar um tempo de vida entre 2.0 e 5.0 segundos
//             lifetime[gid] = 2.0f + rand_float(gid * 7) * 3.0f;

//             float x, y, z;

//             if (shape == 0) //  Cubo
//             {
//                 x = (float)gid / count * 2.0f - 1.0f;
//                 y = ((gid * 31) % count) / (float)count * 2.0f - 1.0f;
//                 z = ((gid * 53) % count) / (float)count * 2.0f - 1.0f;
//             }
//             else //  Esfera
//             {
//                 float theta = (float)(gid % count) / count * 6.283185; //
//                 2*PI float phi = (float)(gid / count) / count * 3.141592; //
//                 PI x = sin(phi) * cos(theta); y = sin(phi) * sin(theta); z =
//                 cos(phi);
//             }


//             points[index6 + 0] = x;
//             points[index6 + 1] = y;
//             points[index6 + 2] = z;

//             // Cor baseada na distância ao centro
//             float dist = sqrt(x*x + y*y + z*z);
//             points[index6 + 3] = 1.0f;
//             points[index6 + 4] = 1.0f;
//             points[index6 + 5] = 1.0f ;

//             float time = 1.0f;

//             float3 vel  = (float3)(rand_float(gid * 3 + (int)(time * 250)) -
//             0.5f,
//                            rand_float(gid * 4 + (int)(time * 600)),
//                            rand_float(gid * 5 + (int)(time * 350)) - 0.5f);

//             velocity[index3 + 0] =      vel.x;
//             velocity[index3 + 1] =      vel.y;
//             velocity[index3 + 2] =      vel.z;


//         }
//     )";


// const char* kernel_update = R"(

//      __constant float GRAV = 9.8f;


//      float rand_float(int seed)
//      {
//         seed = (seed << 13) ^ seed;
//         return (1.0f - ((seed * (seed * seed * 15731 + 789221) + 1376312589)
//         & 0x7fffffff) / 1073741824.0f);
//     }

//         __kernel void update_points(__global float* points, __global float*
//         velocity, __global float* lifetime, int count,float time, int
//         gravity_on,int shape)
//         {
//             int gid = get_global_id(0);
//             int index6 = gid * 6;
//             int index3 = gid * 3;


//             float3 pos = (float3)(points[index6 + 0], points[index6 + 1],
//             points[index6 + 2]); float3 vel = (float3)(velocity[index3 + 0],
//             velocity[index3 + 1], velocity[index3 + 2]);


//             if (gravity_on)
//             {
//                 vel.y += GRAV * time * 0.1f;
//             }


//             lifetime[gid] -= time;

//             if (lifetime[gid] <= 0.01f )
//             {

//                     float x, y, z;
//                     // Resetar a velocidade com um pequeno impulso aleatório


//             float3 vel  = (float3)(rand_float(gid * 3 + (int)(time * 250)) -
//             0.5f,
//                            rand_float(gid * 4 + (int)(time * 300)) - 0.5f,
//                            rand_float(gid * 5 + (int)(time * 350)) - 0.5f);

//             velocity[index3 + 0] =  vel.x;
//             velocity[index3 + 1] =      vel.y;
//             velocity[index3 + 2] =      vel.z;

//                     // Gerar um novo lifetime aleatório entre 2.0s e 5.0s
//                      lifetime[gid] = 2.0f + rand_float(gid * 7 + time * 900)
//                      * 3.0f;


//                     if (shape == 0) //  Cubo
//                     {
//                         x = (float)gid / count * 2.0f - 1.0f;
//                         y = ((gid * 31) % count) / (float)count * 2.0f
//                         - 1.0f; z = ((gid * 53) % count) / (float)count
//                         * 2.0f - 1.0f;
//                     }
//                     else //  Esfera
//                     {
//                         float theta = (float)(gid % count) / count
//                         * 6.283185; // 2*PI float phi = (float)(gid / count)
//                         / count * 3.141592;   // PI x = sin(phi) *
//                         cos(theta); y = sin(phi) * sin(theta); z = cos(phi);
//                     }
//                 pos.x = x;
//                 pos.y = y;
//                 pos.z = z;


//             }


//              pos += vel * time;


//             // Guardar os novos valores

//             points[index6 + 0] = pos.x;
//             points[index6 + 1] = pos.y;
//             points[index6 + 2] = pos.z;

//             // velocity[index3 + 0] = vel.x;
//             // velocity[index3 + 1] = vel.y;
//             // velocity[index3 + 2] = vel.z;
//         }
//     )";

const char* kernel_init = R"(

    float rand_float(int seed, float time) 
    {
        seed = (seed << 13) ^ seed;
        int t = (int)(time * 1000);
        seed = seed + t;
        return (1.0f - ((seed * (seed * seed * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
    }

    float3 hsv_to_rgb(float h, float s, float v) 
    {
        float c = v * s;
        float x = c * (1.0f - fabs(fmod(h / 60.0f, 2.0f) - 1.0f));
        float m = v - c;
        float3 rgb;
        
        if (h < 60.0f) rgb = (float3)(c, x, 0.0f);
        else if (h < 120.0f) rgb = (float3)(x, c, 0.0f);
        else if (h < 180.0f) rgb = (float3)(0.0f, c, x);
        else if (h < 240.0f) rgb = (float3)(0.0f, x, c);
        else if (h < 300.0f) rgb = (float3)(x, 0.0f, c);
        else rgb = (float3)(c, 0.0f, x);
        
        return rgb + m;
    }

    __kernel void create_points(__global float* points, __global float* velocity, 
                              __global float* lifetime, __global float* size,
                              int count, int shape, float time) 
        {
        int gid = get_global_id(0);
        int index6 = gid * 7;
        int index3 = gid * 3;

   
        lifetime[gid] = 1.5f + rand_float(gid * 7, time) * 4.0f;

        size[gid] = 0.5f + rand_float(gid * 11, time) * 1.5f;

        float x, y, z;
        if (shape == 0) 
        { // Cube
            x = rand_float(gid * 13, time) * 2.0f - 1.0f;
            y = rand_float(gid * 17, time) * 2.0f - 1.0f;
            z = rand_float(gid * 19, time) * 2.0f - 1.0f;
        } 
        else if (shape == 1) 
        { // Sphere
            float theta = rand_float(gid * 23, time) * 6.283185f;
            float phi = rand_float(gid * 29, time) * 3.141592f;
            float radius = pow(rand_float(gid * 31, time), 0.333f); 
            
            x = radius * sin(phi) * cos(theta);
            y = radius * sin(phi) * sin(theta) + 150.0f;
            z = radius * cos(phi);
        }
        else
         { // Fountain
            float angle = rand_float(gid * 37, time) * 6.283185f;
            float spread = rand_float(gid * 41, time) * 0.2f;
            x = cos(angle) * spread;
            y = -1.0f; // Start at bottom
            z = sin(angle) * spread;
        }

        points[index6 + 0] = x;
        points[index6 + 1] = y;
        points[index6 + 2] = z;

  
        float hue = (atan2(z, x) + 3.141592f) / 6.283185f * 360.0f;
        float sat = 0.8f + rand_float(gid * 43, time) * 0.2f;
        float val = 0.7f + rand_float(gid * 47, time) * 0.3f;
        float3 color = hsv_to_rgb(hue, sat, val);
        if (shape == 2)   // Fountain
        {
            color = (float3)(0.2f, 0.2f, 0.8f);

        }
        
        points[index6 + 3] = color.x;
        points[index6 + 4] = color.y;
        points[index6 + 5] = color.z;
        points[index6 + 6] = 1.0f;
        

        // Initial velocity based on shape
        float3 vel;
        if (shape == 2)   // Fountain
        {
            float upward_speed = 2.0f + rand_float(gid * 53, time);
            vel = (float3)(x * 2.0f, upward_speed, z * 2.0f);
        } else 
         {
            vel = (float3)(rand_float(gid * 59, time) - 0.5f,
                          rand_float(gid * 61, time) - 0.5f,
                          rand_float(gid * 67, time) - 0.5f);

        if (vel.x == 0.0f && vel.y == 0.0f && vel.z == 0.0f)
        {
            vel = (float3)(0.0f, 1.0f, 0.0f);
        } 
            float upward_speed = 2.0f + rand_float(gid * 53, time);
            vel = (float3)(x * 2.0f, upward_speed, z * 2.0f);
        }

        velocity[index3 + 0] = vel.x;
        velocity[index3 + 1] = vel.y;
        velocity[index3 + 2] = vel.z;
    }
)";

const char* kernel_update = R"(

__constant float3 vertices[] = {
    (float3)(0.023241, -0.074550, 0.147773),
    (float3)(0.023241, -0.074550, 0.284310),
    (float3)(-0.022747, -0.074550, 0.284310),
    (float3)(0.023241, -0.074550, 0.147773),
    (float3)(-0.022747, -0.074550, 0.284310),
    (float3)(-0.022747, -0.074550, 0.147773),
    (float3)(0.023241, 0.111998, 0.113382),
    (float3)(-0.022747, 0.111998, 0.113382),
    (float3)(-0.022747, 0.111998, 0.160281),
    (float3)(0.023241, 0.111998, 0.113382),
    (float3)(-0.022747, 0.111998, 0.160281),
    (float3)(0.023241, 0.111998, 0.160281),
    (float3)(-0.022740, 0.110427, 0.040850),
    (float3)(-0.022740, 0.050734, 0.090148),
    (float3)(-0.022740, 0.111938, 0.090148),
    (float3)(0.023241, -0.034032, 0.284310),
    (float3)(0.023241, 0.111998, 0.160281),
    (float3)(-0.022747, 0.111998, 0.160281),
    (float3)(0.023241, -0.034032, 0.284310),
    (float3)(-0.022747, 0.111998, 0.160281),
    (float3)(-0.022747, -0.034032, 0.284310),
    (float3)(-0.022747, -0.029450, 0.229794),
    (float3)(-0.022747, -0.030519, 0.113382),
    (float3)(-0.022747, -0.074550, 0.147773),
    (float3)(0.023241, -0.030519, 0.113382),
    (float3)(0.023241, -0.121663, 0.113382),
    (float3)(-0.022747, -0.121663, 0.113382),
    (float3)(0.023241, -0.030519, 0.113382),
    (float3)(-0.022747, -0.121663, 0.113382),
    (float3)(-0.022747, -0.030519, 0.113382),
    (float3)(0.023241, -0.074550, 0.284310),
    (float3)(0.023241, -0.034032, 0.284310),
    (float3)(-0.022747, -0.034032, 0.284310),
    (float3)(0.023241, -0.074550, 0.284310),
    (float3)(-0.022747, -0.034032, 0.284310),
    (float3)(-0.022747, -0.074550, 0.284310),
    (float3)(0.023241, 0.111998, 0.113382),
    (float3)(0.023241, -0.029450, 0.229794),
    (float3)(-0.022747, -0.029450, 0.229794),
    (float3)(0.023241, 0.111998, 0.113382),
    (float3)(-0.022747, -0.029450, 0.229794),
    (float3)(-0.022747, 0.111998, 0.113382),
    (float3)(0.023241, -0.029450, 0.229794),
    (float3)(0.023241, -0.030519, 0.113382),
    (float3)(-0.022747, -0.030519, 0.113382),
    (float3)(0.023241, -0.029450, 0.229794),
    (float3)(-0.022747, -0.030519, 0.113382),
    (float3)(-0.022747, -0.029450, 0.229794),
    (float3)(0.023241, -0.121663, 0.113382),
    (float3)(0.023241, -0.122257, 0.149720),
    (float3)(-0.022747, -0.122257, 0.149719),
    (float3)(0.023241, -0.121663, 0.113382),
    (float3)(-0.022747, -0.122257, 0.149719),
    (float3)(-0.022747, -0.121663, 0.113382),
    (float3)(0.023241, -0.122257, 0.149720),
    (float3)(0.023241, -0.074550, 0.147773),
    (float3)(-0.022747, -0.074550, 0.147773),
    (float3)(0.023241, -0.122257, 0.149720),
    (float3)(-0.022747, -0.074550, 0.147773),
    (float3)(-0.022747, -0.122257, 0.149719),
    (float3)(-0.022740, -0.007552, -0.003762),
    (float3)(0.022370, -0.007552, -0.003762),
    (float3)(0.022370, -0.073118, -0.003762),
    (float3)(-0.022740, -0.007552, -0.003762),
    (float3)(0.022370, -0.073118, -0.003762),
    (float3)(-0.022740, -0.073118, -0.003762),
    (float3)(0.022370, 0.055149, 0.038449),
    (float3)(0.022370, 0.063413, -0.003762),
    (float3)(0.022370, 0.111938, -0.003762),
    (float3)(0.022370, 0.055149, 0.038449),
    (float3)(0.022370, 0.111938, -0.003762),
    (float3)(0.022370, 0.110427, 0.040850),
    (float3)(0.022370, -0.012977, 0.090148),
    (float3)(-0.022740, -0.012977, 0.090148),
    (float3)(-0.022740, -0.073118, 0.090148),
    (float3)(0.022370, -0.012977, 0.090148),
    (float3)(-0.022740, -0.073118, 0.090148),
    (float3)(0.022370, -0.073118, 0.090148),
    (float3)(-0.022740, -0.073118, 0.090148),
    (float3)(-0.022740, -0.073118, -0.003762),
    (float3)(0.022370, -0.073118, -0.003762),
    (float3)(-0.022740, -0.073118, 0.090148),
    (float3)(0.022370, -0.073118, -0.003762),
    (float3)(0.022370, -0.073118, 0.090148),
    (float3)(0.022370, 0.111938, 0.090148),
    (float3)(0.022370, 0.111938, -0.003762),
    (float3)(-0.022740, 0.111938, -0.003762),
    (float3)(0.022370, 0.111938, 0.090148),
    (float3)(-0.022740, 0.111938, -0.003762),
    (float3)(-0.022740, 0.111938, 0.090148),
    (float3)(0.022370, 0.111938, 0.090148),
    (float3)(-0.022740, 0.111938, 0.090148),
    (float3)(-0.022740, 0.050734, 0.090148),
    (float3)(0.022370, 0.111938, 0.090148),
    (float3)(-0.022740, 0.050734, 0.090148),
    (float3)(0.022370, 0.050734, 0.090148),
    (float3)(0.022370, 0.050734, 0.090148),
    (float3)(-0.022740, 0.050734, 0.090148),
    (float3)(-0.022740, 0.110427, 0.040850),
    (float3)(0.022370, 0.050734, 0.090148),
    (float3)(-0.022740, 0.110427, 0.040850),
    (float3)(0.022370, 0.110427, 0.040850),
    (float3)(0.022370, 0.110427, 0.040850),
    (float3)(-0.022740, 0.110427, 0.040850),
    (float3)(-0.022740, 0.055149, 0.038449),
    (float3)(0.022370, 0.110427, 0.040850),
    (float3)(-0.022740, 0.055149, 0.038449),
    (float3)(0.022370, 0.055149, 0.038449),
    (float3)(0.022370, 0.055149, 0.038449),
    (float3)(-0.022740, 0.055149, 0.038449),
    (float3)(-0.022740, -0.012977, 0.090148),
    (float3)(0.022370, 0.055149, 0.038449),
    (float3)(-0.022740, -0.012977, 0.090148),
    (float3)(0.022370, -0.012977, 0.090148),
    (float3)(-0.022740, 0.111938, -0.003762),
    (float3)(0.022370, 0.111938, -0.003762),
    (float3)(0.022370, 0.063413, -0.003762),
    (float3)(-0.022740, 0.111938, -0.003762),
    (float3)(0.022370, 0.063413, -0.003762),
    (float3)(-0.022740, 0.063413, -0.003762),
    (float3)(-0.022740, 0.063413, -0.003762),
    (float3)(0.022370, 0.063413, -0.003762),
    (float3)(0.022370, -0.006677, 0.039857),
    (float3)(-0.022740, 0.063413, -0.003762),
    (float3)(0.022370, -0.006677, 0.039857),
    (float3)(-0.022740, -0.006677, 0.039857),
    (float3)(-0.022740, -0.006677, 0.039857),
    (float3)(0.022370, -0.006677, 0.039857),
    (float3)(0.022370, -0.068465, 0.038968),
    (float3)(-0.022740, -0.006677, 0.039857),
    (float3)(0.022370, -0.068465, 0.038968),
    (float3)(-0.022740, -0.068465, 0.038968),
    (float3)(-0.022740, -0.068465, 0.038968),
    (float3)(0.022370, -0.068465, 0.038968),
    (float3)(0.022370, -0.007552, -0.003762),
    (float3)(-0.022740, -0.068465, 0.038968),
    (float3)(0.022370, -0.007552, -0.003762),
    (float3)(-0.022740, -0.007552, -0.003762),
    (float3)(0.022370, 0.111938, 0.090148),
    (float3)(0.022370, 0.050734, 0.090148),
    (float3)(0.022370, 0.110427, 0.040850),
    (float3)(-0.022740, -0.068465, 0.038968),
    (float3)(-0.022740, -0.007552, -0.003762),
    (float3)(-0.022740, -0.073118, -0.003762),
    (float3)(0.022370, -0.073118, -0.003762),
    (float3)(0.022370, -0.007552, -0.003762),
    (float3)(0.022370, -0.068465, 0.038968),
    (float3)(0.023241, -0.029450, 0.229794),
    (float3)(0.023241, 0.111998, 0.113382),
    (float3)(0.023241, 0.111998, 0.160281),
    (float3)(0.022370, 0.111938, -0.003762),
    (float3)(0.022370, 0.111938, 0.090148),
    (float3)(0.022370, 0.110427, 0.040850),
    (float3)(0.022370, -0.073118, 0.090148),
    (float3)(0.022370, -0.073118, -0.003762),
    (float3)(0.022370, -0.068465, 0.038968),
    (float3)(-0.022747, 0.111998, 0.113382),
    (float3)(-0.022747, -0.029450, 0.229794),
    (float3)(-0.022747, 0.111998, 0.160281),
    (float3)(-0.022740, 0.055149, 0.038449),
    (float3)(-0.022740, -0.006677, 0.039857),
    (float3)(-0.022740, -0.012977, 0.090148),
    (float3)(-0.022747, -0.034032, 0.284310),
    (float3)(-0.022747, -0.029450, 0.229794),
    (float3)(-0.022747, -0.074550, 0.147773),
    (float3)(-0.022747, -0.034032, 0.284310),
    (float3)(-0.022747, -0.074550, 0.147773),
    (float3)(-0.022747, -0.074550, 0.284310),
    (float3)(0.022370, 0.055149, 0.038449),
    (float3)(0.022370, -0.006677, 0.039857),
    (float3)(0.022370, 0.063413, -0.003762),
    (float3)(0.022370, -0.006677, 0.039857),
    (float3)(0.022370, 0.055149, 0.038449),
    (float3)(0.022370, -0.012977, 0.090148),
    (float3)(0.023241, -0.030519, 0.113382),
    (float3)(0.023241, -0.074550, 0.147773),
    (float3)(0.023241, -0.122257, 0.149720),
    (float3)(0.023241, -0.030519, 0.113382),
    (float3)(0.023241, -0.122257, 0.149720),
    (float3)(0.023241, -0.121663, 0.113382),
    (float3)(-0.022747, -0.029450, 0.229794),
    (float3)(-0.022747, -0.034032, 0.284310),
    (float3)(-0.022747, 0.111998, 0.160281),
    (float3)(0.023241, -0.074550, 0.284310),
    (float3)(0.023241, -0.074550, 0.147773),
    (float3)(0.023241, -0.029450, 0.229794),
    (float3)(0.023241, -0.074550, 0.284310),
    (float3)(0.023241, -0.029450, 0.229794),
    (float3)(0.023241, -0.034032, 0.284310),
    (float3)(0.023241, -0.030519, 0.113382),
    (float3)(0.023241, -0.029450, 0.229794),
    (float3)(0.023241, -0.074550, 0.147773),
    (float3)(0.022370, -0.006677, 0.039857),
    (float3)(0.022370, -0.012977, 0.090148),
    (float3)(0.022370, -0.073118, 0.090148),
    (float3)(0.022370, -0.006677, 0.039857),
    (float3)(0.022370, -0.073118, 0.090148),
    (float3)(0.022370, -0.068465, 0.038968),
    (float3)(0.023241, -0.029450, 0.229794),
    (float3)(0.023241, 0.111998, 0.160281),
    (float3)(0.023241, -0.034032, 0.284310),
    (float3)(-0.022747, -0.122257, 0.149719),
    (float3)(-0.022747, -0.074550, 0.147773),
    (float3)(-0.022747, -0.030519, 0.113382),
    (float3)(-0.022747, -0.122257, 0.149719),
    (float3)(-0.022747, -0.030519, 0.113382),
    (float3)(-0.022747, -0.121663, 0.113382),
    (float3)(-0.022740, -0.006677, 0.039857),
    (float3)(-0.022740, 0.055149, 0.038449),
    (float3)(-0.022740, 0.063413, -0.003762),
    (float3)(-0.022740, -0.068465, 0.038968),
    (float3)(-0.022740, -0.073118, -0.003762),
    (float3)(-0.022740, -0.073118, 0.090148),
    (float3)(-0.022740, 0.110427, 0.040850),
    (float3)(-0.022740, 0.111938, 0.090148),
    (float3)(-0.022740, 0.111938, -0.003762),
    (float3)(-0.022740, -0.073118, 0.090148),
    (float3)(-0.022740, -0.012977, 0.090148),
    (float3)(-0.022740, -0.006677, 0.039857),
    (float3)(-0.022740, -0.073118, 0.090148),
    (float3)(-0.022740, -0.006677, 0.039857),
    (float3)(-0.022740, -0.068465, 0.038968),
    (float3)(-0.022740, 0.111938, -0.003762),
    (float3)(-0.022740, 0.063413, -0.003762),
    (float3)(-0.022740, 0.055149, 0.038449),
    (float3)(-0.022740, 0.111938, -0.003762),
    (float3)(-0.022740, 0.055149, 0.038449),
    (float3)(-0.022740, 0.110427, 0.040850),
};
    __constant float GRAV = 9.8f;
    __constant float AIR_RESISTANCE = 0.3f;
    __constant float BOUNCE_DAMPING = 0.3f;
    __constant float FLOOR_Y = 0.0f;
      __constant float FADE_START = 0.5f;

     __constant int NUM_VERTICES =  228;

    float rand_float(int seed, float time) 
    {
        seed = (seed << 13) ^ seed;
        int t = (int)(time * 1000);
        seed = seed + t;
        return (1.0f - ((seed * (seed * seed * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
    }

    float3 hsv_to_rgb(float h, float s, float v) 
    {
        float c = v * s;
        float x = c * (1.0f - fabs(fmod(h / 60.0f, 2.0f) - 1.0f));
        float m = v - c;
        float3 rgb;
        
        if (h < 60.0f) rgb = (float3)(c, x, 0.0f);
        else if (h < 120.0f) rgb = (float3)(x, c, 0.0f);
        else if (h < 180.0f) rgb = (float3)(0.0f, c, x);
        else if (h < 240.0f) rgb = (float3)(0.0f, x, c);
        else if (h < 300.0f) rgb = (float3)(x, 0.0f, c);
        else rgb = (float3)(c, 0.0f, x);
        
        return rgb + m;
    }

      float distanceToRay(float3 point, float3 rayOrigin, float3 rayDir) 
    {
        float3 toPoint = point - rayOrigin;
        float projectionLength = dot(toPoint, rayDir);
        float3 closestPoint = rayOrigin + projectionLength * rayDir;
        return length(closestPoint - point);
    }

    __kernel void update_points(__global float* points, __global float* velocity,
                              __global float* lifetime, __global float* size,
                              int count, float time, int gravity_on, int shape,  float3 rayOrigin, float3 rayDir) 
                              {
        int gid = get_global_id(0);
        int index6 = gid * 7;
        int index3 = gid * 3;
             float INTERACTION_RADIUS = 2.0f;  
     float INTERACTION_STRENGTH = 3.0f; 
        float3 pos = (float3)(points[index6 + 0], points[index6 + 1], points[index6 + 2]);
        float3 vel = (float3)(velocity[index3 + 0], velocity[index3 + 1], velocity[index3 + 2]);
        
           // Verificar se a partícula está próxima do raio
        float distance = distanceToRay(pos, rayOrigin, rayDir);
        float3 toRay = normalize(cross(rayDir, pos - rayOrigin));
        if (distance < INTERACTION_RADIUS)
        {
            toRay = normalize(cross(rayDir, pos - rayOrigin));
            float force = INTERACTION_STRENGTH * (1.0f - (distance / INTERACTION_RADIUS));
            vel += toRay * force * time; // Empurrar partículas perpendicularmente ao raio
        }

        
    
        lifetime[gid] -= time;
        
        if (lifetime[gid] <= 0.0f ||  points[index6 + 6]<=0.0f) 
        {
            float x, y, z;

            points[index6 + 6] = 1.0f;
            // Reset particle
            if (shape == 2) 
            { // Fountain
                float angle = rand_float(gid * 37, time) * 6.283185f;
                float spread = rand_float(gid * 41, time) * 0.2f;
                pos = (float3)(cos(angle) * spread, FLOOR_Y, sin(angle) * spread);
                
                float upward_speed = 2.0f + rand_float(gid * 83, time);
                vel = (float3)(pos.x * 2.0f, upward_speed, pos.z * 2.0f);

                      INTERACTION_RADIUS = 2.0f;  
                      INTERACTION_STRENGTH = 3.0f; 

            } else 
             {
                // Reset to original shape
     
                if (shape == 0)
                 { // Cube
                    x = rand_float(gid * 13, time) * 2.0f - 1.0f;
                    y = rand_float(gid * 17, time) * 2.0f - 1.0f;
                    z = rand_float(gid * 19, time) * 2.0f - 1.0f;

                         
                      vel = (float3)(rand_float(gid * 59, time) - 0.5f,
                               rand_float(gid * 61, time) - 0.5f,
                               rand_float(gid * 67, time) - 0.5f);


                        INTERACTION_RADIUS = 20.0f;  
                        INTERACTION_STRENGTH = 40.0f; 
                } else  if (shape == 1)
                 { // Sphere
                    float theta = rand_float(gid * 23, time) * 6.283185f;
                    float phi = rand_float(gid * 29, time) * 3.141592f;
                    float radius = pow(rand_float(gid * 31, time), 0.333f);
                    x = radius * sin(phi) * cos(theta);
                    y = radius * sin(phi) * sin(theta);
                    z = radius * cos(phi);

                        float angle = rand_float(gid * 37, time) * 6.283185f;
                        float spread = rand_float(gid * 2, time) * 0.2f;
                        
                        float upward_speed = 2.0f + rand_float(gid * 293, time);
                        vel = (float3)(x * 2.0f, upward_speed, z * 2.0f);

                        INTERACTION_RADIUS = 1.0f;  
                        INTERACTION_STRENGTH = 25.0f; 
                } else  if (shape == 3) 
                {

                   

                    int vertexIndex = (index6 + (int)(rand_float(gid * NUM_VERTICES, time) * NUM_VERTICES)) % NUM_VERTICES;



       
                    
                    // Pega a posição do vértice
                    x = vertices[vertexIndex].x - toRay.x * 2.0f;
                    y = vertices[vertexIndex].y - toRay.y * 2.0f;
                    z = vertices[vertexIndex].z - toRay.z * 2.0f;
                    
             
                    float spread = 0.001f;
                    x += (rand_float(gid * 13, time) - 0.5f) * spread;
                    y += (rand_float(gid * 17, time) - 0.5f) * spread;
                    z += (rand_float(gid * 19, time) - 0.5f) * spread;
                    
                    
                    float upward_speed = 2.0f + rand_float(gid * 0.01f, time);
                    vel = (float3)(x * 2.0f, upward_speed, z * 2.0f);
                    
           
                    INTERACTION_RADIUS = 0.9f;    
                    INTERACTION_STRENGTH = -0.5f;    
    
                
                }else 
                {
                    float angle = rand_float(gid * 37, time) * 6.283185f;
                    float spread = rand_float(gid * 41, time) * 0.2f;
                    x = cos(angle) * spread;
                    y = rand_float(gid * 47, time) * 0.5f;
                    z = sin(angle) * spread;
                    INTERACTION_RADIUS = 2.0f;  
                    INTERACTION_STRENGTH = 10.0f; 
                }
                pos = (float3)(x, y, z);

         


            }

            float speed = length(vel);
            if  (length(vel)<=0.0f &&  shape != 3)
            {
                 float upward_speed = 2.0f + rand_float(gid * 53, time);
                 vel = (float3)(x * 2.0f, upward_speed, z * 2.0f);
            }

          if ( shape != 3)
           {
            lifetime[gid] = 1.5f + rand_float(gid * 7, time) * 4.0f;
              
          } else
           {
            lifetime[gid] = 1.5f + rand_float(gid * 8, time) * 0.5f;
          }
             
            size[gid] = 0.5f + rand_float(gid * 11, time) * 1.5f;
        }

        // Physics update
        if (gravity_on && shape != 3)
         {
            vel.y -= GRAV * time;
            
 
            vel *= (1.0f - AIR_RESISTANCE * time);

                    if (shape == 2)
                    {
                    
                    // Floor collision
                    if (pos.y < FLOOR_Y)
                    {
                        pos.y = FLOOR_Y;
                        vel.y = -vel.y * BOUNCE_DAMPING;
                    }
            }   
        }


                
        // Update position
        pos += vel * time;

        // Color update based on velocity and lifetime
        float speed = length(vel);
        float life_factor = lifetime[gid] / (1.5f + 4.0f);
        float hue = (atan2(vel.z, vel.x) + 3.141592f) / 6.283185f * 360.0f;
        float sat = 0.8f + 0.2f * life_factor;
        float val = 0.7f + 0.3f * (speed / 5.0f);
        float3 color = hsv_to_rgb(hue, sat, val);
     

        // Store updated values
        points[index6 + 0] = pos.x;
        points[index6 + 1] = pos.y;
        points[index6 + 2] = pos.z;
        points[index6 + 3] = color.x;
        points[index6 + 4] = color.y;
        points[index6 + 5] = color.z;
        points[index6 + 6] = 1.0f;
        
        velocity[index3 + 0] = vel.x;
        velocity[index3 + 1] = vel.y;
        velocity[index3 + 2] = vel.z;
    }
)";

int main(int argc, char** argv)
{
    if (!glfwInit())
    {
        printf("Falha ao inicializar GLFW\n");
        return -1;
    }

    int NUM_POINTS = 1000;  
        //int one_million = 1000 * 1000; // 1.000.000
    const int three_million = 1000 * 1000 * 3;



    if (argc > 1)
    {
        NUM_POINTS = atoi(argv[1]);
        if (NUM_POINTS > three_million)
        {
            NUM_POINTS = three_million;
        }
    }


    GLFWwindow* window = glfwCreateWindow(800, 600, "Points Demo", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) return -1;

    Program program;
    if (!program.initialize())
    {
        std::cout << "Failed to initialize OpenCL" << std::endl;
        glfwTerminate();
        return -1;
    }

    cl_kernel kernelUpdate = program.build(kernel_update, "update_points");

    if (kernelUpdate == nullptr)
    {
        std::cout << "Failed to build program" << std::endl;
        program.release();
        glfwTerminate();
        return -1;
    }


    cl_kernel kernelInit = program.build(kernel_init, "create_points");

    if (kernelInit == nullptr)
    {
        std::cout << "Failed to build program" << std::endl;
        program.release();
        glfwTerminate();
        return -1;
    }


    glfwSetWindowSizeCallback(window, window_resize_callback);

    // glfwSetKeyCallback(window, key_callback);
    // glfwSetCursorPosCallback(window, mouse_callback);
    // glfwSetMouseButtonCallback(window, mouse_callback);
    Shader shader(vertex_shader_source, fragment_shader_source);


  


    MeshBuffer mesh(true);


    VertexFormat vertexFormat;
    vertexFormat.addElement(VertexType::POSITION, 3);
    vertexFormat.addElement(VertexType::COLOR, 4);
    mesh.createVertexBuffer(vertexFormat, NUM_POINTS);


    cl_mem gl_buffer = program.createGLBuffer(CL_MEM_READ_WRITE, mesh.getVBO());
    if (gl_buffer == nullptr)
    {
        std::cout << "Failed to create GL buffer" << std::endl;
        program.release();
        glfwTerminate();
        return -1;
    }
    cl_mem bufferVelocity =
        program.createBuffer(CL_MEM_HOST_NO_ACCESS | CL_MEM_READ_WRITE,
                             NUM_POINTS * 3 * sizeof(float));
    if (bufferVelocity == nullptr)
    {
        std::cout << "Failed to create buffer" << std::endl;
        program.release();
        glfwTerminate();
        return -1;
    }

    cl_mem bufferLifetime = program.createBuffer(
        CL_MEM_HOST_NO_ACCESS | CL_MEM_READ_WRITE, NUM_POINTS * sizeof(float));
    if (bufferLifetime == nullptr)
    {
        std::cout << "Failed to create buffer" << std::endl;
        program.release();
        glfwTerminate();
        return -1;
    }

    cl_mem bufferSize = program.createBuffer(
        CL_MEM_HOST_NO_ACCESS | CL_MEM_READ_WRITE, NUM_POINTS * sizeof(float));
    if (bufferSize == nullptr)
    {
        std::cout << "Failed to create buffer" << std::endl;
        program.release();
        glfwTerminate();
        return -1;
    }


    Timer time;
    FontRenderer font("DotGothic16-Regular.ttf", 16);


    glViewport(0, 0, 800, 600);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glPointSize(5.0f);
    Grid grid(10, 1.0f);
    Camera camera(800, 600);
    double lastX, lastY;
    bool firstMouse;
    float deltaX, deltaY;
    lastX = 400;
    lastY = 300;
    firstMouse = true;


    time.update();


    int shape = 2;
    program.lockVBO(gl_buffer);
    size_t global_size = NUM_POINTS;
    cl_uint index = 0;
    float dt = 1.0f / 60.0f;
    program.setArg(kernelInit, index++, sizeof(cl_mem), &gl_buffer);
    program.setArg(kernelInit, index++, sizeof(cl_mem), &bufferVelocity);
    program.setArg(kernelInit, index++, sizeof(cl_mem), &bufferLifetime);
    program.setArg(kernelInit, index++, sizeof(cl_mem), &bufferSize);
    program.setArg(kernelInit, index++, sizeof(int), &NUM_POINTS);
    program.setArg(kernelInit, index++, sizeof(int), &shape);
    program.setArg(kernelInit, index++, sizeof(float), &dt);
    program.enqueue(kernelInit, 1, &global_size);
    program.unlockVBO(gl_buffer);
    program.finish();


    while (!glfwWindowShouldClose(window))
    {

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        {
            shape = 0;
        }

        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        {
            shape = 1;
        }

        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        {
            shape = 2;
        }
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        {
            shape = 3;
        }
        if (WINDO_RESIZE)
        {
            WINDO_RESIZE = false;
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            font.setSize(width, height);
            camera.setSize(width, height);
        }

        double currentX, currentY;
        glfwGetCursorPos(window, &currentX, &currentY);

        if (firstMouse)
        {
            lastX = currentX;
            lastY = currentY;
            firstMouse = false;
        }


        deltaX = static_cast<float>(currentX - lastX);
        deltaY = static_cast<float>(lastY - currentY);


        lastX = currentX;
        lastY = currentY;

        float deltaTime = time.getDeltaTime();
        camera.processKeyboard(window, deltaTime);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            camera.processMouse(deltaX, deltaY);
        }


        int gravity_on = 1;

        // Obter a direção do cursor no mundo
        glm::vec3 rayDir = camera.screenToWorldRay(currentX, currentY);

        // Definir um ponto de origem para o raio (posição da câmara)
        glm::vec3 rayOrigin = camera.position;

        // Converter `glm::vec3` para `cl_float3`
        cl_float3 rayOriginCL = { rayOrigin.x, rayOrigin.y, rayOrigin.z };
        cl_float3 rayDirCL = { rayDir.x, rayDir.y, rayDir.z };


        // glm::vec3 m_pos = camera.screenToWorldRay(currentX, currentY);
        //    cl_float3 mouse[3] = {{m_pos.x, m_pos.y, m_pos.z}};

        program.lockVBO(gl_buffer);
        index = 0;
        program.setArg(kernelUpdate, index++, sizeof(cl_mem), &gl_buffer);
        program.setArg(kernelUpdate, index++, sizeof(cl_mem), &bufferVelocity);
        program.setArg(kernelUpdate, index++, sizeof(cl_mem), &bufferLifetime);
        program.setArg(kernelUpdate, index++, sizeof(cl_mem), &bufferSize);
        program.setArg(kernelUpdate, index++, sizeof(int), &NUM_POINTS);
        program.setArg(kernelUpdate, index++, sizeof(float), &deltaTime);
        program.setArg(kernelUpdate, index++, sizeof(int), &gravity_on);
        program.setArg(kernelUpdate, index++, sizeof(int), &shape);
        program.setArg(kernelUpdate, index++, sizeof(cl_float3), &rayOriginCL);
        program.setArg(kernelUpdate, index++, sizeof(cl_float3), &rayDirCL);

        // cl_float3 gravity_center = {0.0f, -0.1f, 0.0f};
        // program.setArg(kernelUpdate, 4, sizeof(cl_float3), &gravity_center);


        if (!program.enqueue(kernelUpdate, 1, &global_size))
        {
            break;
        }

        program.unlockVBO(gl_buffer);

        program.finish();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = camera.getProjectionMatrix();
        glm::mat4 view = camera.getViewMatrix();
        shader.use();
        shader.setUniform("view", view);
        shader.setUniform("projection", projection);

        glEnable(GL_DEPTH_TEST);
        //  glDisable(GL_BLEND);
        //  glDisable(GL_DEPTH_TEST);

        mesh.render(GL_POINTS, NUM_POINTS);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        grid.draw(view, projection);


        glDisable(GL_DEPTH_TEST);

        int FPS = static_cast<int>(time.getFPS());
        if (FPS < 30)
        {
            font.printf(10, 20, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f), "FPS 0%d",
                        FPS);
        }
        else
        {
            if (FPS < 59)
                font.printf(10, 20, 1.0f, glm::vec3(0.4f, 1.0f, 0.4f), "FPS %d",
                            FPS);
            else
                font.printf(10, 20, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f), "FPS %d",
                            FPS);
        }


        glfwSwapBuffers(window);
        glfwPollEvents();
        time.update();
    }
    font.release();
    program.release();
    mesh.release();
    shader.release();
    grid.release();

    glfwTerminate();
    return 0;
}