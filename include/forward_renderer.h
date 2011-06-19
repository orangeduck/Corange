#ifndef forward_renderer_h
#define forward_renderer_h

#include "camera.h"
#include "geometry.h"
#include "texture.h"
#include "material.h"
#include "renderable.h"

void forward_renderer_init(int width, int height);
void forward_renderer_finish();

void forward_renderer_set_camera(camera* c);
void forward_renderer_set_dimensions(int width, int height);

void forward_renderer_setup_camera();

void forward_renderer_begin();
void forward_renderer_end();

void forward_renderer_render_model(render_model* m, material* mat);
void forward_renderer_render_renderable(renderable* r);
void forward_renderer_use_material(material* mat);

#endif