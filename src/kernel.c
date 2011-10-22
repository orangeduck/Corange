#include <stdlib.h>
#include <stdio.h>

#include "asset_manager.h"


#include "kernel.h"

static cl_int error;

kernel_program* cl_load_file(char* filename) {
  
  /*
  
  char* source = asset_load_file(filename);
  
  int src_size;
  cl_program program = clCreateProgramWithSource(context, 1, &source, &src_size, &error);
  check_cl_error(error, "clCreateProgramWithSource");

  error = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  check_cl_error(error, "clBuildProgram");
  
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
  return program;
  
  */
}

void kernel_program_delete(kernel_program* k) {
  clReleaseProgram(*k);
  free(k);
}

kernel kernel_program_get_kernel(kernel_program* kp, char* kernel_name) {
  cl_kernel k = clCreateKernel(*kp, kernel_name, &error);
  check_cl_error(error, "clCreateKernel");
  return k;
}

kernel kernel_set_argument(kernel k, int arg_num, int arg_type_size, void* value) {
  error = clSetKernelArg(k, arg_num, arg_type_size, value);
  check_cl_error(error, "clSetKernelArg");
}

void kernel_delete(kernel k) {
  clReleaseKernel(k);
}