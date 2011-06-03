
#define GLEW_STATIC
#include "GL/glew.h"

#define NO_SDL_GLEXT
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "asset_manager.h"
#include "glsl.h"

glsl_program* glsl_load_shaders(char* vs_filename, char* fs_filename) {
  
  
  glsl_shader vertex_shader = glsl_load_vertex_shader(vs_filename);
  glsl_shader fragment_shader = glsl_load_fragment_shader(fs_filename);
  glsl_program* my_program = glsl_load_program(vertex_shader, fragment_shader);
  
  return my_program;
  
};

glsl_shader glsl_load_vertex_shader(char* filename) {
  
  char* vs_source = asset_load_file(filename);
  const char* vs = vs_source;
  
  glsl_shader vertex_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
  
  glShaderSourceARB(vertex_shader, 1, &vs, NULL);
  glCompileShaderARB(vertex_shader);
  
  glsl_shader_print_compile_log(vertex_shader);
  
  free(vs_source);
  
  return vertex_shader;

};

glsl_shader glsl_load_fragment_shader(char* filename) {

  char* fs_source = asset_load_file(filename);
  const char* fs = fs_source;
  
  glsl_shader fragment_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
  glShaderSourceARB(fragment_shader, 1, &fs, NULL);
  glCompileShaderARB(fragment_shader);
  
  glsl_shader_print_compile_log(fragment_shader);
  
  free(fs_source);
  
  return fragment_shader;

};

glsl_program* glsl_load_program(glsl_shader vertex_shader, glsl_shader fragment_shader) {
  
  glsl_program my_program = glCreateProgramObjectARB();
  
  glAttachObjectARB(my_program, vertex_shader);
  glAttachObjectARB(my_program, fragment_shader);
 
  glLinkProgramARB(my_program);
  glsl_program_print_link_log(my_program);
  
  glDeleteObjectARB(vertex_shader);
  glDeleteObjectARB(fragment_shader);
  
  glsl_program* prog = malloc(sizeof(glsl_program));
  *prog = my_program;
  
  return prog;
  
};

glsl_program* glsl_load_vertex_program(glsl_shader vertex_shader) {
  
  glsl_program my_program = glCreateProgramObjectARB();
  glAttachObjectARB(my_program, vertex_shader);
  
  glLinkProgramARB(my_program);
  glsl_program_print_link_log(my_program);
  
  glDeleteObjectARB(vertex_shader);
  
  glsl_program* prog = malloc(sizeof(glsl_program));
  *prog = my_program;
  
  return prog;
};

glsl_program* glsl_load_fragment_program(glsl_shader fragment_shader) {
  
  glsl_program my_program = glCreateProgramObjectARB();
  glAttachObjectARB(my_program, fragment_shader);
  
  glLinkProgramARB(my_program);
  glsl_program_print_link_log(my_program);
  
  glDeleteObjectARB(fragment_shader);
  
  glsl_program* prog = malloc(sizeof(glsl_program));
  *prog = my_program;
  
  return prog;
};

void glsl_program_print_link_log(glsl_program program) {

  char* log = malloc(2048);
  int i;
  glGetInfoLogARB(program, 2048, &i, log);
  log[i] = '\0';
  printf("\nShader Linker:\n %s\n", log);
  free(log);
  
}

void glsl_shader_print_compile_log(glsl_shader shader) {

  char* log = malloc(2048);
  int i;
  glGetInfoLogARB(shader, 2048, &i, log);
  log[i] = '\0';
  printf("\nShader Compiler:\n %s\n", log);
  free(log);
  
}

void glsl_program_delete(glsl_program* program) {
  glDeleteObjectARB(*program);
  free(program);
};