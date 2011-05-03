#ifndef renderer_h
#define renderer_h

#include "geometry.h"
#include "font.h"
#include "texture.h"

void renderer_setup();
void renderer_finish();

void renderer_set_camera(camera* c);

void renderer_set_dimensions(int width, int height);

void renderer_setup_camera();

void renderer_begin_render();
void renderer_end_render();

void renderer_render_model(render_model* m);

void renderer_render_quad(texture* quad_texture);
void renderer_render_char(char c, font* f, vector2 pos, float size);
void renderer_render_string(char* s, font* f, vector2 pos, float size);

void renderer_print_gl_error();

#endif