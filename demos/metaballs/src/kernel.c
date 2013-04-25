#include "kernel.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
#endif

#ifdef __linux__
  #include  <GL/glx.h>
#endif

static cl_int error = 0;
static cl_platform_id platforms[32];
static cl_uint num_platforms = 0;

static cl_context context;
static cl_command_queue queue;
static cl_device_id device;

void kernels_init() {
  
  error = clGetPlatformIDs(32, platforms, &num_platforms);
  kernels_check_error("clGetPlatformID");
  
  kernels_info();
  
  error = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  kernels_check_error("clGetDeviceIDs");
  
  context = clCreateContext(0, 1, &device, NULL, NULL, &error);
  kernels_check_error("clCreateContext");
  
  queue = clCreateCommandQueue(context, device, 0, &error);
  kernels_check_error("clCreateCommandQueue");
  
}

void kernels_init_with_cpu() {

  error = clGetPlatformIDs(32, platforms, &num_platforms);
  kernels_check_error("clGetPlatformID");
  
  kernels_info();
  
  error = clGetDeviceIDs(platforms[1], CL_DEVICE_TYPE_CPU, 1, &device, NULL);
  kernels_check_error("clGetDeviceIDs");
  
  context = clCreateContext(0, 1, &device, NULL, NULL, &error);
  kernels_check_error("clCreateContext");
  
  queue = clCreateCommandQueue(context, device, 0, &error);
  kernels_check_error("clCreateCommandQueue");

}

void kernels_init_with_opengl() {

  error = clGetPlatformIDs(32, platforms, &num_platforms);
  kernels_check_error("clGetPlatformID");

  kernels_info();
  
  error = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  kernels_check_error("clGetDeviceIDs");

#ifdef _WIN32

#define CL_GL_CONTEXT_KHR 0x2008
#define CL_WGL_HDC_KHR 0x200B

  cl_context_properties props[] = 
  {
    CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(), 
    CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(), 
    CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[0], 
    0
  };
  
#elif __linux__

  cl_context_properties props[] = 
  {
    CL_GL_CONTEXT_KHR, (intptr_t)glXGetCurrentContext(), 
    CL_GLX_DISPLAY_KHR, (intptr_t)glXGetCurrentDisplay(), 
    CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[0], 
    0
  };

#else
  
  error("Can't interlop CL with GL, unsupported platform!");

#endif

  context = clCreateContext(props, 1, &device, NULL, NULL, &error);
  kernels_check_error("clCreateContext");
  
  queue = clCreateCommandQueue(context, device, 0, &error);
  kernels_check_error("clCreateCommandQueue");

}

void kernels_finish() {
  
  clReleaseCommandQueue(queue);
  clReleaseContext(context);

}

void kernels_info() {
  
  static char platform_info[512];
  static size_t platform_info_size = 0;
  
  debug("OpenCL Info for %i Platforms", num_platforms)
  
  for(int i = 0; i < num_platforms; i++) {
    debug("Platform: %i", i);
    
    error = clGetPlatformInfo(platforms[i], CL_PLATFORM_PROFILE, 512, platform_info, &platform_info_size);
    kernels_check_error("clGetPlatformInfo");
    debug("Profile: %s", platform_info);
  
    error = clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, 512, platform_info, &platform_info_size);
    kernels_check_error("clGetPlatformInfo");
    debug("Version: %s", platform_info);
  
    error = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 512, platform_info, &platform_info_size);
    kernels_check_error("clGetPlatformInfo");
    debug("Name: %s", platform_info);
  
    error = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, 512, platform_info, &platform_info_size);
    kernels_check_error("clGetPlatformInfo");
    debug("Vendor: %s", platform_info);
  
    error = clGetPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS, 512, platform_info, &platform_info_size);
    kernels_check_error("clGetPlatformInfo");
    debug("Extensions: %s", platform_info);
  
  }
  
}

void kernels_check_error(const char* name) {
  
  if (error == CL_SUCCESS) return;
  
  switch (error) {
      case CL_DEVICE_NOT_FOUND: error("OpenCL error on %s: Device not found.", name);
      case CL_DEVICE_NOT_AVAILABLE: error("OpenCL error on %s: Device not available", name);
      case CL_COMPILER_NOT_AVAILABLE: error("OpenCL error on %s: Compiler not available", name);
      case CL_MEM_OBJECT_ALLOCATION_FAILURE: error("OpenCL error on %s: Memory object allocation failure", name);
      case CL_OUT_OF_RESOURCES: error("OpenCL error on %s: Out of resources", name);
      case CL_OUT_OF_HOST_MEMORY: error("OpenCL error on %s: Out of host memory", name);
      case CL_PROFILING_INFO_NOT_AVAILABLE: error("OpenCL error on %s: Profiling information not available", name);
      case CL_MEM_COPY_OVERLAP: error("OpenCL error on %s: Memory copy overlap", name);
      case CL_IMAGE_FORMAT_MISMATCH: error("OpenCL error on %s: Image format mismatch", name);
      case CL_IMAGE_FORMAT_NOT_SUPPORTED: error("OpenCL error on %s: Image format not supported", name);
      case CL_BUILD_PROGRAM_FAILURE: error("OpenCL error on %s: Program build failure", name);
      case CL_MAP_FAILURE: error("OpenCL error on %s: Map failure", name);
      case CL_INVALID_VALUE: error("OpenCL error on %s: Invalid value", name);
      case CL_INVALID_DEVICE_TYPE: error("OpenCL error on %s: Invalid device type", name);
      case CL_INVALID_PLATFORM: error("OpenCL error on %s: Invalid platform", name);
      case CL_INVALID_DEVICE: error("OpenCL error on %s: Invalid device", name);
      case CL_INVALID_CONTEXT: error("OpenCL error on %s: Invalid context", name);
      case CL_INVALID_QUEUE_PROPERTIES: error("OpenCL error on %s: Invalid queue properties", name);
      case CL_INVALID_COMMAND_QUEUE: error("OpenCL error on %s: Invalid command queue", name);
      case CL_INVALID_HOST_PTR: error("OpenCL error on %s: Invalid host pointer", name);
      case CL_INVALID_MEM_OBJECT: error("OpenCL error on %s: Invalid memory object", name);
      case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: error("OpenCL error on %s: Invalid image format descriptor", name);
      case CL_INVALID_IMAGE_SIZE: error("OpenCL error on %s: Invalid image size", name);
      case CL_INVALID_SAMPLER: error("OpenCL error on %s: Invalid sampler", name);
      case CL_INVALID_BINARY: error("OpenCL error on %s: Invalid binary", name);
      case CL_INVALID_BUILD_OPTIONS: error("OpenCL error on %s: Invalid build options", name);
      case CL_INVALID_PROGRAM: error("OpenCL error on %s: Invalid program", name);
      case CL_INVALID_PROGRAM_EXECUTABLE: error("OpenCL error on %s: Invalid program executable", name);
      case CL_INVALID_KERNEL_NAME: error("OpenCL error on %s: Invalid kernel name", name);
      case CL_INVALID_KERNEL_DEFINITION: error("OpenCL error on %s: Invalid kernel definition", name);
      case CL_INVALID_KERNEL: error("OpenCL error on %s: Invalid kernel", name);
      case CL_INVALID_ARG_INDEX: error("OpenCL error on %s: Invalid argument index", name);
      case CL_INVALID_ARG_VALUE: error("OpenCL error on %s: Invalid argument value", name);
      case CL_INVALID_ARG_SIZE: error("OpenCL error on %s: Invalid argument size", name);
      case CL_INVALID_KERNEL_ARGS: error("OpenCL error on %s: Invalid kernel arguments", name);
      case CL_INVALID_WORK_DIMENSION: error("OpenCL error on %s: Invalid work dimension", name);
      case CL_INVALID_WORK_GROUP_SIZE: error("OpenCL error on %s: Invalid work group size", name);
      case CL_INVALID_WORK_ITEM_SIZE: error("OpenCL error on %s: Invalid work item size", name);
      case CL_INVALID_GLOBAL_OFFSET: error("OpenCL error on %s: Invalid global offset", name);
      case CL_INVALID_EVENT_WAIT_LIST: error("OpenCL error on %s: Invalid event wait list", name);
      case CL_INVALID_EVENT: error("OpenCL error on %s: Invalid event", name);
      case CL_INVALID_OPERATION: error("OpenCL error on %s: Invalid operation", name);
      case CL_INVALID_GL_OBJECT: error("OpenCL error on %s: Invalid OpenGL object", name);
      case CL_INVALID_BUFFER_SIZE: error("OpenCL error on %s: Invalid buffer size", name);
      case CL_INVALID_MIP_LEVEL: error("OpenCL error on %s: Invalid mip-map level", name);
      case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR: error("OpenCL error on %s: Unaquired OpenGL resource", name);
      default: error("OpenCL error on %s id %i", name, error);
  }
  
}

kernel_program* cl_load_file(char* filename) {
  
  int size;
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  SDL_RWsize(file, &size);
  
  char* source = malloc(size+1);
  source[size] = '\0';
  SDL_RWread(file, source, size, 1);
  
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
  debug("%s", build_log);
  free(build_log);
  
  free(source);
  SDL_RWclose(file);
  
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

void kernel_run_flush() {
  error = clFlush(queue);
  kernels_check_error("clFlush");
}

void kernel_run_finish() {
  error = clFinish(queue);
  kernels_check_error("clFinish");
}
