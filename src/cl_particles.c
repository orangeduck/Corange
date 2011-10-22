#include <stdio.h>
#include <stdlib.h>

#include "CL/cl.h"

#include "cl_particles.h"

/*
  Tutorial Link 

  http://opencl.codeplex.com/wikipage?title=OpenCL%20Tutorials%20-%201
  
*/

const char* hello_world = "Hello World!";

void check_cl_error(cl_int error, const char* name) {
    if (error != CL_SUCCESS) {
        printf("OpenCL Error: %s (%i)\n", name, error);
        exit(EXIT_FAILURE);
    }
}

cl_int error = 0;
cl_platform_id platform;
cl_context context;
cl_command_queue queue;
cl_device_id device;

void cl_particles_init() {

  error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  check_cl_error(error, "clGetDeviceIDs");
  
  context = clCreateContext(0, 1, &device, NULL, NULL, &error);
  check_cl_error(error, "clCreateContext");
  
  queue = clCreateCommandQueue(context, device, 0, &error);
  check_cl_error(error, "clCreateCommandQueue");
  
  const int size = 1024;
  float* src_a_h = malloc(sizeof(float) * size);
  float* src_b_h = malloc(sizeof(float) * size);
  float* res_h = malloc(sizeof(float) * size);
  // Initialize both vectors
  int i;
  for (i = 0; i < size; i++) {
     src_a_h[i] = src_b_h[i] = (float) i;
  }
  
  const int mem_size = sizeof(float)*size;
  // Allocates a buffer of size mem_size and copies mem_size bytes from src_a_h
  cl_mem src_a_d = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size, src_a_h, &error);
  cl_mem src_b_d = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size, src_b_h, &error);
  cl_mem res_d = clCreateBuffer(context, CL_MEM_WRITE_ONLY, mem_size, NULL, &error);
  
  
  // Creates the program
  // Uses NVIDIA helper functions to get the code string and it's size (in bytes)
  size_t src_size = 0;
  
  const char* source = ""; 
  
  cl_program program = clCreateProgramWithSource(context, 1, &source, &src_size, &error);
  check_cl_error(error, "clCreateProgramWithSource");

  // Builds the program
  error = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  check_cl_error(error, "clBuildProgram");

  // Shows the log
  char* build_log;
  size_t log_size;
  // First call to know the proper size
  clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
  build_log = malloc(log_size+1);
  // Second call to get the log
  clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
  build_log[log_size] = '\0';
  printf("%s\n", build_log);
  free(build_log);

  // Extracting the kernel
  cl_kernel vector_add_k = clCreateKernel(program, "vector_add_gpu", &error);
  check_cl_error(error, "clCreateKernel");
  
  // Enqueuing parameters
  // Note that we inform the size of the cl_mem object, not the size of the memory pointed by it
  error = clSetKernelArg(vector_add_k, 0, sizeof(cl_mem), &src_a_d);
  error |= clSetKernelArg(vector_add_k, 1, sizeof(cl_mem), &src_b_d);
  error |= clSetKernelArg(vector_add_k, 2, sizeof(cl_mem), &res_d);
  error |= clSetKernelArg(vector_add_k, 3, sizeof(size_t), &size);
  check_cl_error(error, "clSetKernelArg");

  // Launching kernel
  const size_t local_ws = 512;	// Number of work-items per work-group
  const size_t global_ws = 1024;	// Total number of work-items
  error = clEnqueueNDRangeKernel(queue, vector_add_k, 1, NULL, &global_ws, &local_ws, 0, NULL, NULL);
  check_cl_error(error, "clEnqueueNDRangeKernel");
  
  float* check = malloc(sizeof(float) * size);
  clEnqueueReadBuffer(queue, res_d, CL_TRUE, 0, mem_size, check, 0, NULL, NULL);
  
  free(src_a_h);
  free(src_b_h);
  free(res_h);
  free(check);
  
  clReleaseKernel(vector_add_k);
  clReleaseCommandQueue(queue);
  clReleaseContext(context);
  clReleaseMemObject(src_a_d);
  clReleaseMemObject(src_b_d);
  clReleaseMemObject(res_d);
}