#ifndef wolf_renderer_h
#define wolf_renderer_h

#include "corange.h"

#include "wolf_renderable.h"

void wolf_renderer_init();
void wolf_renderer_finish();

void wolf_renderer_set_camera(camera* c);
void wolf_renderer_set_light(light* l);
void wolf_renderer_set_shadow_texture(texture* t);

void wolf_renderer_begin_drawing();
void wolf_renderer_draw(wolf_renderable* r);
void wolf_renderer_end_drawing();

void wolf_renderer_begin_painting();
void wolf_renderer_paint(wolf_renderable* r);
void wolf_renderer_end_painting();

#endif