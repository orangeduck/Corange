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
  glLinkProgram(*program);
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
  
  char* c = asset_file_contents(filename);
  
  char* line = malloc(1024);
  
  int i = 0;
  int j = 0;
  
  while(1) {
  
    /* End of line reached */
    if(( c[i] == '\n' ) || ( c[i] == '\0')) {
    
      /* Null terminate line buffer */
      line[j] = '\0';
      
      shader_program_parse_line(sp, line);
      
      /* Reset line buffer index */
      j = 0;
      
      if( c[i] == '\0') { break; }
      
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
  shader_program_print_log(sp);
  
  int error = 0;
  glGetProgramiv(*sp, GL_LINK_STATUS, &error);
  if (error == GL_FALSE) {
    error("Linking Error on Shader Program %s.", filename);
  }
  
  return sp;
  
};

static char type[25];
static char path[512];
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
  
  if ((strcmp(type, "vertex_shader") == 0) || (strcmp(type, "fragment_shader") == 0))  {
    
    shader* s;
    
    if(asset_loaded(path)) {
      s = (shader*)asset_get(path);
    } else {
      load_file(path);
      s = (shader*)asset_get(path);
    }
    
    shader_program_attach_shader(program, s);
    return;
  }
  
  if (strcmp(type, "\r\n\0")) {
    return;
  }
  
  error("Error Reading shader file line: %s", line);
  
} 

void shader_program_delete(shader_program* program) {
  glDeleteProgram(*program);
  free(program);
};

void shader_delete(shader* shader) {
  glDeleteShader(*shader);
  free(shader);
}


