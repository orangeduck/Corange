#include "error.h"

#include "asset_manager.h"

#include "assets/shader.h"

static void trim(char * s) {
  char * p = s;
  int l = strlen(p);

  while(isspace(p[l - 1])) p[--l] = 0;
  while(* p && isspace(* p)) ++p, --l;

  memmove(s, p, l + 1);
}

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
  
  glShaderSource(*new_shader, 1, (const char**)&contents, NULL);
  glCompileShader(*new_shader);
  
  free(contents);
  
  shader_print_log(new_shader);
  
  int compile_error = 0;
  glGetShaderiv(*new_shader, GL_COMPILE_STATUS, &compile_error);
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
    error("Cannot get handle for NULL shader");
  }
  return *p;
}

void shader_program_attach_shader(shader_program* program, shader* shader) {
  glAttachShader(*program, *shader);
}


void shader_program_link(shader_program* program) {
  glLinkProgram(*program);
}

void shader_program_print_log(shader_program* program) {
  char log[2048];
  int i;
  glGetProgramInfoLog(*program, 2048, &i, log);
  log[i] = '\0';
  debug("%s", log);
}

void shader_print_log(shader* shader) {
  char log[2048];
  int i;
  glGetShaderInfoLog(*shader, 2048, &i, log);
  log[i] = '\0';
  debug("%s", log);
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
      
      if (strcmp(type, "geometry_shader") == 0) {
        GLint count = -1;
        glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &count);
        glProgramParameteri(shader_program_handle(sp), GL_GEOMETRY_VERTICES_OUT, count); 
      }
      
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
  
  int is_linked = false;
  glGetProgramiv(*sp, GL_LINK_STATUS, &is_linked);
  if (is_linked == GL_FALSE) {
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


