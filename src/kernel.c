#include <stdlib.h>
#include <stdio.h>

#include "asset_manager.h"

#include "kernel.h"

static cl_int error = 0;
static cl_context context = 0;
static cl_device_id device = 0;

void kernel_set_context(cl_context new_context) {
  context = new_context;
}

void kernel_set_device(cl_device_id new_device) {
  device = new_device;
}

void kernel_check_error() {
  if (error != CL_SUCCESS) {
    printf("OpenCL Error: %i\n", error);
  }
}

kernel_program* cl_load_file(char* filename) {
  
  char* source = asset_load_file(filename);
  
  const char* source_const = source;
  
  int src_size;
  cl_program program = clCreateProgramWithSource(context, 1, &source_const, &src_size, &error);
  kernel_check_error();

  error = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  kernel_check_error();
  
  char* build_log;
  int log_size;
  clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
  build_log = malloc(log_size+1);
  
  clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
  build_log[log_size] = '\0';
  printf("%s\n", build_log);
  free(build_log);
  
  free(source);
  
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
  kernel_check_error();
  return k;
}

kernel kernel_set_argument(kernel k, int arg_num, int arg_type_size, void* value) {
  error = clSetKernelArg(k, arg_num, arg_type_size, value);
  kernel_check_error();
}

void kernel_delete(kernel k) {
  clReleaseKernel(k);
}