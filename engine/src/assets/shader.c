#include "assets/shader.h"

static shader* load_shader_file(char* filename, GLenum type) {

  shader* new_shader = malloc(sizeof(shader));
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  if(file == NULL) {
    error("Cannot load file %s", filename);
  }
  
  long size = SDL_RWseek(file,0,SEEK_END);
  char* contents = malloc(size+1);
  contents[size] = '\0';
  
  SDL_RWseek(file, 0, SEEK_SET);
  SDL_RWread(file, contents, size, 1);
  SDL_RWclose(file);
  
  *new_shader = glCreateShader(type);
  
  glShaderSource(shader_handle(new_shader), 1, (const char**)&contents, NULL);
  glCompileShader(shader_handle(new_shader));
  
  free(contents);
  
  shader_print_log(new_shader);
  
  int compile_error = 0;
  glGetShaderiv(shader_handle(new_shader), GL_COMPILE_STATUS, &compile_error);
  if (compile_error == GL_FALSE) {
    error("Compiler Error on Shader %s.", filename);
  }
  
  return new_shader;
}

shader* vs_load_file(char* filename) {
  return load_shader_file(filename, GL_VERTEX_SHADER);
}

shader* fs_load_file(char* filename) {
  return load_shader_file(filename, GL_FRAGMENT_SHADER);
}

shader* gs_load_file(char* filename) {
  return load_shader_file(filename, GL_GEOMETRY_SHADER);
}

shader* tcs_load_file(char* filename) {
  return load_shader_file(filename, GL_TESS_CONTROL_SHADER);
}

shader* tes_load_file(char* filename) {
  return load_shader_file(filename, GL_TESS_EVALUATION_SHADER);
}

shader_program* shader_program_new() {

  shader_program* program = malloc(sizeof(shader_program));  
  *program = glCreateProgram();
  return program;

}

GLuint shader_program_handle(shader_program* p) {
  if (p == NULL) {
    error("Cannot get handle for NULL shader program");
  }
  if (!glIsProgram(*p)) {
    error("Not a shader program");
  }
  return *p;
}

GLuint shader_handle(shader* s) {
  if (s == NULL) {
    error("Cannot get handle for NULL shader");
  }
  if (!glIsShader(*s)) {
    error("Not a shader");
  }
  return *s;
}


void shader_program_attach_shader(shader_program* program, shader* shader) {
  
  if (shader_program_has_shader(program, shader)) {
    error("Shader already attached!");
  }
  
  glAttachShader(shader_program_handle(program), shader_handle(shader));
  
  shader_program_print_log(program);
  
}

void shader_program_link(shader_program* program) {

  GLint count = -1;
  glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &count);
  glProgramParameteri(shader_program_handle(program), GL_GEOMETRY_VERTICES_OUT, count);

  glLinkProgram(shader_program_handle(program));
  
  shader_program_print_log(program);
  
  GLint is_linked = false;
  glGetProgramiv(shader_program_handle(program), GL_LINK_STATUS, &is_linked);
  if (!is_linked) {
    error("Error linking shader program!");
  }
  
}

bool shader_program_has_shader(shader_program* p, shader* s) {

  GLuint shaders[128];
  int num_shaders = 0;
  glGetAttachedShaders(shader_program_handle(p), 128, &num_shaders, shaders);

  for(int i = 0; i < num_shaders; i++) {
    if (shaders[i] == shader_handle(s)) return true;
  }
  
  return false;
}

void shader_program_print_info(shader_program* p) {
  
  GLuint shaders[128];
  int num_shaders = 0;
  glGetAttachedShaders(shader_program_handle(p), 128, &num_shaders, shaders);
  
  debug("Program %i has %i shaders", shader_program_handle(p), num_shaders);
  for(int i = 0; i < num_shaders; i++) {
    debug("| Shader %i: %i", i, shaders[i]);
  }
  
}

void shader_program_print_log(shader_program* program) {
  char log[2048];
  int i;
  glGetProgramInfoLog(shader_program_handle(program), 2048, &i, log);
  log[i] = '\0';
  
  if (strcmp(log, "") != 0) {
    debug("%s", log);
  }
}

void shader_print_log(shader* shader) {
  char log[2048];
  int i;
  glGetShaderInfoLog(shader_handle(shader), 2048, &i, log);
  log[i] = '\0';
  
  if (strcmp(log, "") != 0) {
    debug("%s", log);
  }
}

void shader_program_delete(shader_program* program) {
  glDeleteProgram(shader_program_handle(program));
  free(program);
}

void shader_delete(shader* shader) {
  glDeleteShader(shader_handle(shader));
  free(shader);
}


