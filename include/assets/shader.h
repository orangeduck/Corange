/**
*** :: Shader ::
***
***   GLSL shader program.
***
**/

#ifndef shader_h
#define shader_h

#include "cengine.h"
#include "casset.h"

typedef GLuint shader;
typedef GLuint shader_program;

shader* vs_load_file(char* filename);
shader* fs_load_file(char* filename);
shader* gs_load_file(char* filename);
shader* tcs_load_file(char* filename);
shader* tes_load_file(char* filename);

void shader_delete(shader* s);
void shader_print_log(shader* s);
GLuint shader_handle(shader* s);

shader_program* shader_program_new();
void shader_program_delete(shader_program* p);

bool shader_program_has_shader(shader_program* p, shader* s);
void shader_program_attach_shader(shader_program* p, shader* s);
void shader_program_link(shader_program* p);

void shader_program_print_info(shader_program* p);
void shader_program_print_log(shader_program* p);

GLuint shader_program_handle(shader_program* p);
GLint shader_program_get_attribute(shader_program* p, char* name);

void shader_program_enable(shader_program* p);
void shader_program_disable(shader_program* p);

void shader_program_set_int(shader_program* p, char* name, int val);
void shader_program_set_float(shader_program* p, char* name, float val);
void shader_program_set_vec2(shader_program* p, char* name, vec2 val);
void shader_program_set_vec3(shader_program* p, char* name, vec3 val);
void shader_program_set_vec4(shader_program* p, char* name, vec4 val);
void shader_program_set_mat4(shader_program* p, char* name, mat4 val);
void shader_program_set_float_array(shader_program* p, char* name, float* vals, int count);
void shader_program_set_vec2_array(shader_program* p, char* name, vec2* vals, int count);
void shader_program_set_vec3_array(shader_program* p, char* name, vec3* vals, int count);
void shader_program_set_vec4_array(shader_program* p, char* name, vec4* vals, int count);
void shader_program_set_mat4_array(shader_program* p, char* name, mat4* vals, int count);

void shader_program_enable_texture(shader_program* p, char* name, int index, asset_hndl t);
void shader_program_enable_texture_id(shader_program* p, char* name, int index, GLint t);
void shader_program_disable_texture(shader_program* p, int index);

void shader_program_enable_attribute(shader_program* p, char* name, int count, int stride, void* ptr);
void shader_program_disable_attribute(shader_program* p, char* name);

#endif
