#pragma once
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200
#define CL_TARGET_OPENCL_VERSION 200
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#define __CL_ENABLE_EXCEPTIONS

#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <string>

class Program {
private:
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;

    std::vector<cl_mem> buffers;
    std::vector<cl_mem> gl_buffers;
    std::vector<cl_kernel> kernels;

    cl_uint m_arg_index;

public:
    Program()
        : platform(nullptr), device(nullptr), context(nullptr), queue(nullptr),
          program(nullptr), m_arg_index(0)
    {}


    bool initialize();

    cl_kernel build(const std::string& source, const std::string& kernel_name);


    cl_mem createBuffer(cl_mem_flags flags, size_t size,
                        void* host_ptr = nullptr);
    cl_mem createGLBuffer(cl_mem_flags flags, GLuint gl_buffer);


    void lockVBO(cl_mem buffer);

    void unlockVBO(cl_mem buffer);

    bool setArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size,
                const void* arg_value);

    bool setArg(cl_kernel kernel,   size_t arg_size,
                    const void* arg_value);
    
    bool enqueue(cl_kernel kernel, cl_uint work_dim,
                 const size_t* global_work_size,
                 const size_t* local_work_size = nullptr);

    void finish();
    void release();
};