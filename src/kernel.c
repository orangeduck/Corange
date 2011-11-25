#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
#endif

#include "asset_manager.h"
#include "error.h"

#include "CL/cl_gl.h"

#include "kernel.h"

static cl_int error = 0;
static cl_platform_id platform;
static cl_context context;
static cl_command_queue queue;
static cl_device_id device;

void kernels_init() {
  
  error = clGetPlatformIDs(1, &platform, NULL);
  kernels_check_error("oclGetPlatformID");
  
  error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  kernels_check_error("clGetDeviceIDs");
  
  context = clCreateContext(0, 1, &device, NULL, NULL, &error);
  kernels_check_error("clCreateContext");
  
  queue = clCreateCommandQueue(context, device, 0, &error);
  kernels_check_error("clCreateCommandQueue");
  
}

void kernels_init_with_cpu() {

  error = clGetPlatformIDs(1, &platform, NULL);
  kernels_check_error("oclGetPlatformID");
  
  error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
  kernels_check_error("clGetDeviceIDs");
  
  context = clCreateContext(0, 1, &device, NULL, NULL, &error);
  kernels_check_error("clCreateContext");
  
  queue = clCreateCommandQueue(context, device, 0, &error);
  kernels_check_error("clCreateCommandQueue");

}

void kernels_init_with_opengl() {

#ifdef _WIN32

#define CL_GL_CONTEXT_KHR	0x2008
#define CL_WGL_HDC_KHR 0x200B

  error = clGetPlatformIDs(1, &platform, NULL);
  kernels_check_error("oclGetPlatformID");

  error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  kernels_check_error("clGetDeviceIDs");

  cl_context_properties props[] = 
  {
    CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(), 
    CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(), 
    CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 
    0
  };
  context = clCreateContext(props, 1, &device, NULL, NULL, &error);
  kernels_check_error("clCreateContext");
  
  queue = clCreateCommandQueue(context, device, 0, &error);
  kernels_check_error("clCreateCommandQueue");
  
#else
  
  error("Can't interlop CL with GL unless on windows!");

#endif

}

void kernels_finish() {
  
  clReleaseCommandQueue(queue);
  clReleaseContext(context);

}

void kernels_check_error(const char* name) {
  if (error != CL_SUCCESS) {
    error("OpenCL Error on function %s, id: %i", name, error);
  }
}

kernel_program* cl_load_file(char* filename) {
  
  char* source = asset_file_contents(filename);
  const char* source_const = source;
  int src_len = strlen(source);
  
  cl_program program = clCreateProgramWithSource(context, 1, &source_const, (const size_t*)&src_len, &error);
  kernels_check_error("clCreateProgramWithSource");

  error = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  
  char* build_log;
  int log_size;
  clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, (size_t*)&log_size);
  build_log = malloc(log_size+1);
  
  clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, (const size_t)log_size, build_log, NULL);
  build_log[log_size] = '\0';
  printf("%s\n", build_log);
  free(build_log);
  
  free(source);
  
  kernels_check_error("clBuildProgram");
  
  kernel_program* kp = malloc(sizeof(kernel_program));
  *kp = program;
  return kp;
  
}

void kernel_program_delete(kernel_program* k) {
  clReleaseProgram(*k);
  free(k);
}

kernel kernel_program_get_kernel(kernel_program* kp, char* kernel_name) {
  cl_kernel k = clCreateKernel(*kp, kernel_name, &error);
  kernels_check_error("clCreateKernel");
  return k;
}

void kernel_set_argument(kernel k, int arg_num, int arg_type_size, void* value) {
  error = clSetKernelArg(k, arg_num, arg_type_size, value);
  kernels_check_error("clSetKernelArg");
}

void kernel_delete(kernel k) {
  clReleaseKernel(k);
}

void kernel_run(kernel k, int worker_count) {
  error = clEnqueueNDRangeKernel(queue, k, 1, NULL, (const size_t*)&worker_count, NULL, 0, NULL, NULL);
  kernels_check_error("clEnqueueNDRangeKernel");
}

kernel_memory kernel_memory_allocate(int size) {
  kernel_memory mem = clCreateBuffer(context, CL_MEM_READ_WRITE, size, NULL, &error);
  kernels_check_error("clCreateBuffer");
  return mem;
}

kernel_memory kernel_memory_from_glbuffer(int buff_obj) {
  kernel_memory mem = clCreateFromGLBuffer(context, CL_MEM_READ_WRITE, buff_obj, &error);
  kernels_check_error("clCreateFromGLBuffer");
  return mem;
}

kernel_memory kernel_memory_from_gltexture2D(int tex_obj) {
  /* 3553 is for GL_TEXTURE_2D - avoids dependancies. */
  kernel_memory mem = clCreateFromGLTexture2D(context, CL_MEM_READ_WRITE, 3553, 0, tex_obj, &error);
  kernels_check_error("clCreateFromGLTexture2D");
  return mem;
}

kernel_memory kernel_memory_from_gltexture3D(int tex_obj) {
  /* 32879 is for GL_TEXTURE_3D - avoids dependancies. */
  kernel_memory mem = clCreateFromGLTexture3D(context, CL_MEM_READ_WRITE, 32879, 0, tex_obj, &error);
  kernels_check_error("clCreateFromGLTexture3D");
  return mem;
}

void kernel_memory_copy_to_texture(kernel_memory km, kernel_memory tex, int width, int height, int depth) {
  const size_t origin[3] = {0, 0, 0};
  const size_t range[3] = {width, height, depth};
  error = clEnqueueCopyBufferToImage(queue, km, tex, 0, origin, range, 0, NULL, NULL);
  kernels_check_error("clEnqueueCopyBufferToImage");
}

void kernel_memory_gl_aquire(kernel_memory km) {
  error = clEnqueueAcquireGLObjects(queue, 1, &km, 0, NULL, NULL);
  kernels_check_error("clEnqueueAcquireGLObjects");
}

void kernel_memory_gl_release(kernel_memory km) {
  error = clEnqueueReleaseGLObjects(queue, 1, &km, 0, NULL, NULL);
  kernels_check_error("clEnqueueReleaseGLObjects");
}

void kernel_memory_delete(kernel_memory km) {
  clReleaseMemObject(km);
}

void kernel_memory_write(kernel_memory km, int size, void* src) {
  error = clEnqueueWriteBuffer(queue, km, CL_TRUE, 0, size, src, 0, NULL, NULL);
  kernels_check_error("clEnqueueWriteBuffer");
}

void kernel_memory_read(kernel_memory km, int size, void* dst) {
  error = clEnqueueReadBuffer(queue, km, CL_TRUE, 0, size, dst, 0, NULL, NULL);
  kernels_check_error("clEnqueueReadBuffer");
}
