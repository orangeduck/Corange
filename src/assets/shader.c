#include "assets/shader.h"
#include "assets/texture.h"

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

GLint shader_program_get_attribute(shader_program* p, char* name) {

  GLint attr = glGetAttribLocation(shader_program_handle(p), name);
  if (attr == -1) {
    error("Shader has no attribute called '%s'", name);
    return -1;
  } else {
    return attr;
  }
  
}

void shader_program_enable(shader_program* p) {
  glUseProgram(shader_program_handle(p));
}

void shader_program_disable(shader_program* p) {
  glActiveTexture(GL_TEXTURE0);
  glUseProgram(0);
}

void shader_program_set_int(shader_program* p, char* name, int val) {
  GLint location = glGetUniformLocation(shader_program_handle(p), name);
  if ( location == -1) {
    warning("Shader has no uniform called '%s'", name);
  } else {
    glUniform1i(location, val);
  }
}

void shader_program_set_float(shader_program* p, char* name, float val) {
  GLint location = glGetUniformLocation(shader_program_handle(p), name);
  if ( location == -1) {
    warning("Shader has no uniform called '%s'", name);
  } else {
    glUniform1f(location, val);
  }
}

void shader_program_set_vec2(shader_program* p, char* name, vec2 val) {
  GLint location = glGetUniformLocation(shader_program_handle(p), name);
  if ( location == -1) {
    warning("Shader has no uniform called '%s'", name);
  } else {
    glUniform2f(location, val.x, val.y);
  }
}

void shader_program_set_vec3(shader_program* p, char* name, vec3 val) {
  GLint location = glGetUniformLocation(shader_program_handle(p), name);
  if ( location == -1) {
    warning("Shader has no uniform called '%s'", name);
  } else {
    glUniform3f(location, val.x, val.y, val.z);
  }
}

void shader_program_set_vec4(shader_program* p, char* name, vec4 val) {
  GLint location = glGetUniformLocation(shader_program_handle(p), name);
  if ( location == -1) {
    warning("Shader has no uniform called '%s'", name);
  } else {
    glUniform4f(location, val.x, val.y, val.z, val.w);
  }
}

void shader_program_set_mat3(shader_program* p, char* name, mat3 val) {
  GLint location = glGetUniformLocation(shader_program_handle(p), name);
  if ( location == -1) {
    warning("Shader has no uniform called '%s'", name);
  } else {
    glUniformMatrix3fv(location, 1, GL_TRUE, (float*)&val);
  }
}

void shader_program_set_mat4(shader_program* p, char* name, mat4 val) {
  GLint location = glGetUniformLocation(shader_program_handle(p), name);
  if ( location == -1) {
    warning("Shader has no uniform called '%s'", name);
  } else {
    glUniformMatrix4fv(location, 1, GL_TRUE, (float*)&val);
  }
}

void shader_program_set_texture(shader_program* p, char* name, int index, asset_hndl t) {

  GLint location = glGetUniformLocation(shader_program_handle(p), name);
  if ( location == -1) {
    warning("Shader has no uniform called '%s'", name);
  } else {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(texture_type(asset_hndl_ptr(&t)), texture_handle(asset_hndl_ptr(&t)));
    glUniform1i(location, index);
  }
  
}

void shader_program_set_texture_id(shader_program* p, char* name, int index, GLint t) {

  GLint location = glGetUniformLocation(shader_program_handle(p), name);
  if ( location == -1) {
    warning("Shader has no uniform called '%s'", name);
  } else {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, t);
    glUniform1i(location, index);
  }

}

void shader_program_set_float_array(shader_program* p, char* name, float* vals, int count) {
  GLint location = glGetUniformLocation(shader_program_handle(p), name);
  if ( location == -1) {
    warning("Shader has no uniform called '%s'", name);
  } else {
    glUniform1fv(location, count, vals);
  }
}

void shader_program_set_vec2_array(shader_program* p, char* name, vec2* vals, int count) {
  GLint location = glGetUniformLocation(shader_program_handle(p), name);
  if ( location == -1) {
    warning("Shader has no uniform called '%s'", name);
  } else {
    glUniform2fv(location, count, (float*)vals);
  }
}

void shader_program_set_vec3_array(shader_program* p, char* name, vec3* vals, int count) {
  GLint location = glGetUniformLocation(shader_program_handle(p), name);
  if ( location == -1) {
    warning("Shader has no uniform called '%s'", name);
  } else {
    glUniform3fv(location, count, (float*)vals);
  }
}

void shader_program_set_vec4_array(shader_program* p, char* name, vec4* vals, int count) {
  GLint location = glGetUniformLocation(shader_program_handle(p), name);
  if ( location == -1) {
    warning("Shader has no uniform called '%s'", name);
  } else {
    glUniform4fv(location, count, (float*)vals);
  }
}

void shader_program_set_mat4_array(shader_program* p, char* name, mat4* vals, int count) {
  GLint location = glGetUniformLocation(shader_program_handle(p), name);
  if ( location == -1) {
    warning("Shader has no uniform called '%s'", name);
  } else {
    glUniformMatrix4fv(location, count, GL_TRUE, (float*)vals);
  }
}

void shader_program_enable_attribute(shader_program* p, char* name, int count, int stride, void* ptr) {
  GLint attr = glGetAttribLocation(shader_program_handle(p), name);
  if (attr == -1) {
    warning("Shader has no attribute called '%s'", name);
  } else {
    glEnableVertexAttribArray(attr);  
    glVertexAttribPointer(attr, count, GL_FLOAT, GL_FALSE, sizeof(float) * stride, ptr);
  }
}

void shader_program_enable_attribute_instance(shader_program* p, char* name, int count, int stride, void* ptr) {
  GLint attr = glGetAttribLocation(shader_program_handle(p), name);
  if (attr == -1) {
    warning("Shader has no attribute called '%s'", name);
  } else {
    glEnableVertexAttribArray(attr);  
    glVertexAttribPointer(attr, count, GL_FLOAT, GL_FALSE, sizeof(float) * stride, ptr);
    glVertexAttribDivisor(attr, 1);
  }
}

void shader_program_enable_attribute_instance_matrix(shader_program* p, char* name, void* ptr) {
  GLint attr = glGetAttribLocation(shader_program_handle(p), name);
  if (attr == -1) {
    warning("Shader has no attribute called '%s'", name);
  } else {
    glEnableVertexAttribArray(attr+0);  
    glEnableVertexAttribArray(attr+1);  
    glEnableVertexAttribArray(attr+2);  
    glEnableVertexAttribArray(attr+3);
    glVertexAttribPointer(attr+0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4 * 4, ptr);
    glVertexAttribPointer(attr+1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4 * 4, ptr + sizeof(float) * 4);
    glVertexAttribPointer(attr+2, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4 * 4, ptr + sizeof(float) * 8);
    glVertexAttribPointer(attr+3, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4 * 4, ptr + sizeof(float) * 12);
    glVertexAttribDivisor(attr+0, 1);
    glVertexAttribDivisor(attr+1, 1);
    glVertexAttribDivisor(attr+2, 1);
    glVertexAttribDivisor(attr+3, 1);
  }
}


void shader_program_disable_attribute(shader_program* p, char* name) {
  GLint attr = glGetAttribLocation(shader_program_handle(p), name);
  if (attr == -1) {
    warning("Shader has no attribute called '%s'", name);
  } else {
    glDisableVertexAttribArray(attr);  
  }
}

void shader_program_disable_attribute_matrix(shader_program* p, char* name) {
  GLint attr = glGetAttribLocation(shader_program_handle(p), name);
  if (attr == -1) {
    warning("Shader has no attribute called '%s'", name);
  } else {
    glVertexAttribDivisor(attr+0, 0);
    glVertexAttribDivisor(attr+1, 0);
    glVertexAttribDivisor(attr+2, 0);
    glVertexAttribDivisor(attr+3, 0);
    glDisableVertexAttribArray(attr+0);  
    glDisableVertexAttribArray(attr+1);  
    glDisableVertexAttribArray(attr+2);  
    glDisableVertexAttribArray(attr+3);
  }
}

