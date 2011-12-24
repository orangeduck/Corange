#include "asset_manager.h"
#include "error.h"

#include "shader.h"

static void trim(char * s) {
    char * p = s;
    int l = strlen(p);

    while(isspace(p[l - 1])) p[--l] = 0;
    while(* p && isspace(* p)) ++p, --l;

    memmove(s, p, l + 1);
}

shader* vs_load_file(char* filename) {

  shader* new_shader = malloc(sizeof(shader));
  
  char* vs_source = asset_file_contents(filename);
  const char* vs = vs_source;
  
  *new_shader = glCreateShader(GL_VERTEX_SHADER);
  
  glShaderSource(*new_shader, 1, &vs, NULL);
  glCompileShader(*new_shader);
  
  free(vs_source);
  
  shader_print_log(new_shader);
  
  int error = 0;
  glGetShaderiv(*new_shader, GL_COMPILE_STATUS, &error);
  if (error == GL_FALSE) {
    error("Compiler Error on Shader %s.", filename);
  }
  
  return new_shader;
  
}

shader* fs_load_file(char* filename) {

  shader* new_shader = malloc(sizeof(shader));

  char* fs_source = asset_file_contents(filename);
  const char* fs = fs_source;
  
  *new_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(*new_shader, 1, &fs, NULL);
  glCompileShader(*new_shader);
  
  free(fs_source);
  
  shader_print_log(new_shader);
  
  int error = 0;
  glGetShaderiv(*new_shader, GL_COMPILE_STATUS, &error);
  if (error == GL_FALSE) {
    error("Compiler Error on Shader %s.", filename);
  }
  
  return new_shader;

}

shader_program* shader_program_new() {

  shader_program* program = malloc(sizeof(shader_program));  
  *program = glCreateProgram();
  return program;

}

void shader_program_attach_shader(shader_program* program, shader* shader) {
  glAttachShader(*program, *shader);
}


void shader_program_link(shader_program* program) {
  printf("BEFORE LINK\n");fflush(stdout);
  glLinkProgram(*program);
  printf("AFTER LINK\n");fflush(stdout);
}

void shader_program_print_log(shader_program* program) {

  char* log = malloc(2048);
  int i;
  glGetProgramInfoLog(*program, 2048, &i, log);
  log[i] = '\0';
  printf("%s", log);
  free(log);
  
}

void shader_print_log(shader* shader) {

  char* log = malloc(2048);
  int i;
  glGetShaderInfoLog(*shader, 2048, &i, log);
  log[i] = '\0';
  printf("%s", log);
  free(log);
  
}

shader_program* prog_load_file(char* filename) {

  shader_program* sp = shader_program_new();
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  
  if(file == NULL) {
    error("Could not load file %s", filename);
  }
  
  char line[1024];
  while(SDL_RWreadline(file, line, 1024)) {
    
    char type[256];
    char path[1024];
    if (sscanf(line, "%256s : %1024s", type, path) == 2) {
      if(!asset_loaded(path)) {
        load_file(path);
      }
      shader* s = asset_get(path);
      shader_program_attach_shader(sp, s);
    }
  
  }
  
  SDL_RWclose(file);
  
  shader_program_link(sp);
  shader_program_print_log(sp);
  
  int error = 0;
  glGetProgramiv(*sp, GL_LINK_STATUS, &error);
  if (error == GL_FALSE) {
    error("Linking Error on Shader Program %s.", filename);
  }
  
  return sp;
  
} 

void shader_program_delete(shader_program* program) {
  glDeleteProgram(*program);
  free(program);
}

void shader_delete(shader* shader) {
  glDeleteShader(*shader);
  free(shader);
}


