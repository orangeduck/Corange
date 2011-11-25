#ifndef kernel_h
#define kernel_h

#include "CL/cl.h"

typedef cl_program kernel_program;
typedef cl_kernel kernel;
typedef cl_mem kernel_memory;

void kernels_init();
void kernels_init_with_cpu();
void kernels_init_with_opengl();
void kernels_finish();

void kernels_check_error(const char* name);

kernel_program* cl_load_file(char* filename);
void kernel_program_delete(kernel_program* k);

kernel kernel_program_get_kernel(kernel_program* kp, char* kernel_name);

void kernel_set_argument(kernel k, int arg_num, int arg_type_size, void* value);
void kernel_run(kernel k, int worker_count);
void kernel_delete(kernel k);

kernel_memory kernel_memory_allocate(int size);
kernel_memory kernel_memory_from_glbuffer(int buff_obj);
kernel_memory kernel_memory_from_gltexture2D(int tex_obj);
kernel_memory kernel_memory_from_gltexture3D(int tex_obj);

void kernel_memory_gl_aquire(kernel_memory km);
void kernel_memory_gl_release(kernel_memory km);

void kernel_memory_copy_to_texture(kernel_memory km, kernel_memory tex, int width, int height, int depth);

void kernel_memory_delete(kernel_memory km);
void kernel_memory_write(kernel_memory km, int size, void* src);
void kernel_memory_read(kernel_memory km, int size, void* dst);

#endif