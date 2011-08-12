
#define GLEW_STATIC
#include "GL/glew.h"

#define NO_SDL_GLEXT
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "asset_manager.h"
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
  
  char* vs_source = asset_load_file(filename);
  const char* vs = vs_source;
  
  *new_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
  
  glShaderSourceARB(*new_shader, 1, &vs, NULL);
  glCompileShaderARB(*new_shader);
  
  shader_print_log(new_shader);
  
  free(vs_source);
 
  return new_shader;
  
}

shader* fs_load_file(char* filename) {

  shader* new_shader = malloc(sizeof(shader));

  char* fs_source = asset_load_file(filename);
  const char* fs = fs_source;
  
  *new_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
  glShaderSourceARB(*new_shader, 1, &fs, NULL);
  glCompileShaderARB(*new_shader);
  
  shader_print_log(new_shader);
  
  free(fs_source);   
  
  return new_shader;

}


shader_program* shader_program_new() {

  shader_program* program = malloc(sizeof(shader_program));  
  *program = glCreateProgramObjectARB();
  return program;

}

void shader_program_attach_shader(shader_program* program, shader* shader) {
  glAttachObjectARB(*program, *shader);
}


void shader_program_link(shader_program* program) {
  glLinkProgramARB(*program);
  shader_program_print_log(program);
}

void shader_program_print_log(shader_program* program) {

  char* log = malloc(2048);
  int i;
  glGetInfoLogARB(*program, 2048, &i, log);
  log[i] = '\0';
  printf("\nShader Linker:\n %s\n", log);
  free(log);
  
}

void shader_print_log(shader* shader) {

  char* log = malloc(2048);
  int i;
  glGetInfoLogARB(*shader, 2048, &i, log);
  log[i] = '\0';
  printf("\nShader Compiler:\n %s\n", log);
  free(log);
  
}

shader_program* prog_load_file(char* filename) {

  shader_program* sp = shader_program_new();
  
  char* c = asset_load_file(filename);
  
  char* line = malloc(1024);
  
  int i = 0;
  int j = 0;
  
  while(1) {
  
    /* If end of string then exit. */
    if( c[i] == '\0') { break; }
    
    /* End of line reached */
    if( c[i] == '\n' ) {
    
      /* Null terminate line buffer */
      line[j-1] = '\0';
      
      shader_program_parse_line(sp, line);
      
      /* Reset line buffer index */
      j = 0;
      
    } else {
    
      /* Otherwise add character to line buffer */
      line[j] = c[i];
      j++;
    }
    i++;
  }
  
  free(line);
  
  free(c);
  
  shader_program_link(sp);

  return sp;
  
};

static char type[25];
static char path[256];
void shader_program_parse_line(shader_program* program, char* line) {
  
  char c;
  
  /* Find type */
  int i = 0;
  while(1) {
    c = line[i];
    if (c == ':') {
      strncpy(type, line, i);
      type[i] = '\0';
      trim(type);
      break;
    }
    i++;
  }
  
  /* Find path */
  int j = i;
  while(1) {
    c = line[j];
    if (c == '\0') {
      char* begin = line+i+1;
      int end = j-i-1;
      strncpy(path, begin, end); 
      path[end] = '\0';
      trim(path);
      break;
    }
    j++;
  }
  
  if (strcmp(type, "vertex_shader") == 0) {
    
    shader* vs;
    
    if(asset_loaded(path)) {
      vs = (shader*)asset_get(path);
    } else {
      load_file(path);
      vs = (shader*)asset_get(path);
    }
    
    shader_program_attach_shader(program, vs);
    return;
  }
  
  if (strcmp(type, "fragment_shader") == 0) {
    
    shader* fs;
    
    if(asset_loaded(path)) {
      fs = (shader*)asset_get(path);
    } else {
      load_file(path);
      fs = (shader*)asset_get(path);
    }
    
    shader_program_attach_shader(program, fs);
    return;
  }
  
  printf("Error: badly formed program file\n");
  return;
  
} 

void shader_program_delete(shader_program* program) {
  glDeleteObjectARB(*program);
  free(program);
};

void shader_delete(shader* shader) {
  
  glDeleteObjectARB(*shader);
  free(shader);
  
}


