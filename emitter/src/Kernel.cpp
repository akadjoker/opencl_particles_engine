#include "Kernel.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3native.h>
#include <iostream>


bool Program::initialize()
{
    cl_int err;


    err = clGetPlatformIDs(1, &platform, nullptr);
    if (err != CL_SUCCESS)
    {
        std::cout << "Failed to get OpenCL platform" << std::endl;
        return false;
    }

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
    if (err != CL_SUCCESS)
    {
        std::cout << "Failed to get OpenCL device" << std::endl;
        return false;
    }

    // Create OpenCL context with OpenGL interop
    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)platform,
        CL_GL_CONTEXT_KHR,
        (cl_context_properties)glfwGetGLXContext(glfwGetCurrentContext()),
        CL_GLX_DISPLAY_KHR,
        (cl_context_properties)glfwGetX11Display(),
        0
    };

    context = clCreateContext(properties, 1, &device, nullptr, nullptr, &err);
    if (err != CL_SUCCESS)
    {
        std::cout << "Failed to create OpenCL context" << std::endl;
        return false;
    }

    // Create command queue
    cl_queue_properties queue_props[] = { 0 };
    queue =
        clCreateCommandQueueWithProperties(context, device, queue_props, &err);
    if (err != CL_SUCCESS)
    {
        std::cout << "Failed to create OpenCL command queue" << std::endl;
        return false;
    }

    return true;
}

cl_kernel Program::build(const std::string& source,
                         const std::string& kernel_name)
{
    cl_int err;
    const char* source_str = source.c_str();
    size_t source_size = source.size();
    cl_kernel kernel = nullptr;

    program =
        clCreateProgramWithSource(context, 1, &source_str, &source_size, &err);
    if (err != CL_SUCCESS)
    {
        std::cout << "Failed to create OpenCL program" << std::endl;
        return kernel;
    }

    err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS)
    {

        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr,
                              &log_size);
        std::vector<char> log(log_size);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size,
                              log.data(), nullptr);
        std::cout << "Failed to build OpenCL program: " << std::endl;
        std::cout << log.data() << std::endl;
        return kernel;
    }

    kernel = clCreateKernel(program, kernel_name.c_str(), &err);
    if (err != CL_SUCCESS)
    {
        std::cout << "Failed to create OpenCL kernel" << std::endl;
        return kernel;
    }

    kernels.push_back(kernel);

    return kernel;
}

cl_mem Program::createBuffer(cl_mem_flags flags, size_t size, void* host_ptr)
{
    cl_int err;
    cl_mem buffer = clCreateBuffer(context, flags, size, host_ptr, &err);
    if (err != CL_SUCCESS)
    {
        std::cout << "Failed to create OpenCL buffer" << std::endl;
        return nullptr;
    }
    buffers.push_back(buffer);
    return buffer;
}

cl_mem Program::createGLBuffer(cl_mem_flags flags, GLuint gl_buffer)
{
    cl_int err;
    cl_mem buffer = clCreateFromGLBuffer(context, flags, gl_buffer, &err);
    if (err != CL_SUCCESS)
    {
        throw std::runtime_error("Failed to create GL buffer");
    }
    gl_buffers.push_back(buffer);
    return buffer;
}

void Program::lockVBO(cl_mem buffer)
{
    glFinish();
    cl_int err =
        clEnqueueAcquireGLObjects(queue, 1, &buffer, 0, nullptr, nullptr);
    if (err != CL_SUCCESS)
    {
        std::cout << "Failed to lock VBO" << std::endl;
    }
}

void Program::unlockVBO(cl_mem buffer)
{
    cl_int err =
        clEnqueueReleaseGLObjects(queue, 1, &buffer, 0, nullptr, nullptr);
    if (err != CL_SUCCESS)
    {
        std::cout << "Failed to unlock VBO" << std::endl;
    }
}

bool Program::setArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size,
                     const void* arg_value)
{
    cl_int err = clSetKernelArg(kernel, arg_index, arg_size, arg_value);
    if (err != CL_SUCCESS)
    {
        std::cout << "Failed to set OpenCL kernel argument" << std::endl;
        return false;
    }

    return true;
}

bool Program::setArg(cl_kernel kernel,  size_t arg_size, const void* arg_value)
{
return setArg(kernel, m_arg_index++, arg_size, arg_value);
}

bool Program::enqueue(cl_kernel kernel, cl_uint work_dim,
                      const size_t* global_work_size,
                      const size_t* local_work_size)
{
    cl_int err = clEnqueueNDRangeKernel(queue, kernel, work_dim, nullptr,
                                        global_work_size, local_work_size, 0,
                                        nullptr, nullptr);
    if (err != CL_SUCCESS)
    {
        std::cout << "Failed to enqueue OpenCL kernel" << std::endl;
        return false;
    }

    return true;
}

void Program::finish() { clFinish(queue); }

void Program::release()
{
    if (!program) return;
    for (cl_mem buffer : buffers)
    {
        clReleaseMemObject(buffer);
    }
    for (cl_mem buffer : gl_buffers)
    {
        clReleaseMemObject(buffer);
    }
    buffers.clear();
    gl_buffers.clear();
    for (cl_kernel kernel : kernels)
    {
        clReleaseKernel(kernel);
    }
    kernels.clear();

    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    platform = nullptr;
    device = nullptr;
    context = nullptr;
    queue = nullptr;
    program = nullptr;
}