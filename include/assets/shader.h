/**
*** :: Shader ::
***
***   GLSL shader program.
***
**/

#ifndef shader_h
#define shader_h

#include "cengine.h"

typedef GLuint shader;
typedef GLuint shader_program;

shader* vs_load_file(char* filename);
shader* fs_load_file(char* filename);
shader* gs_load_file(char* filename);
shader* tcs_load_file(char* filename);
shader* tes_load_file(char* filename);

void shader_delete(shader* s);
void shader_print_log(shader* s);

shader_program* shader_program_new();
void shader_program_delete(shader_program* p);

GLuint shader_program_handle(shader_program* p);

void shader_program_attach_shader(shader_program* p, shader* s);
void shader_program_link(shader_program* p);

void shader_program_print_log(shader_program* p);


#endif
