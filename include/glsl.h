#ifndef glsl_loader_h
#define glsl_loader_h

#include "SDL.h"
#include "SDL_opengl.h"

typedef GLenum glsl_shader;
typedef GLhandleARB glsl_program;

glsl_program* glsl_load_shaders(char* vs_filename, char* fs_filename);

glsl_shader glsl_load_vertex_shader(char* filename);
glsl_shader glsl_load_fragment_shader(char* filename);

glsl_program* glsl_load_program(glsl_shader vertex_shader, glsl_shader fragment_shader);
glsl_program* glsl_load_vertex_program(glsl_shader vertex_shader);
glsl_program* glsl_load_fragment_program(glsl_shader fragment_shader);

void glsl_shader_print_compile_log(glsl_shader shader);
void glsl_program_print_link_log(glsl_program program);

void glsl_program_delete(glsl_program* program);

#endif