#ifndef kernel_h
#define kernel_h

#include "CL/cl.h"

typedef cl_program kernel_program;
typedef cl_kernel kernel;

kernel_program* cl_load_file(char* filename);
void kernel_program_delete(kernel_program* k);

kernel kernel_program_get_kernel(kernel_program* kp, char* kernel_name);

kernel kernel_set_argument(kernel k, int arg_num, int arg_type_size, void* value);
void kernel_run(kernel k, int worker_count, int work_group_size);
void kernel_delete(kernel k);

#endif