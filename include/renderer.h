#ifndef renderer_h
#define renderer_h

#include "geometry.h"
#include "texture.h"

void forward_renderer_init();
void forward_renderer_finish();

void forward_renderer_set_camera(camera* c);
void forward_renderer_set_dimensions(int width, int height);

void forward_renderer_setup_camera();

void forward_renderer_begin();
void forward_renderer_end();

void forward_renderer_render_model(render_model* m);

void forward_renderer_render_quad(texture* quad_texture, vector2 pos, vector2 size);
void forward_renderer_render_screen_quad(texture* quad_texture);

#endif