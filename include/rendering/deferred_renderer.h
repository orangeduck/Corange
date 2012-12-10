/**
*** :: Deferred Renderer ::
***
***   A (fairly) simple deferred renderer.
***
***   Can render most of the built-in entities.
***   Does a HDR and tonemapping stage, composition stage
***   as well as various post-effects and also SSAO.   
***
**/

#ifndef deferred_renderer_h
#define deferred_renderer_h

#include "cengine.h"
#include "assets/texture.h"
#include "assets/shader.h"

#include "entities/camera.h"
#include "entities/light.h"
#include "entities/static_object.h"
#include "entities/animated_object.h"
#include "entities/landscape.h"

void deferred_renderer_init();
void deferred_renderer_finish();

void deferred_renderer_set_camera(camera* cam);
void deferred_renderer_set_shadow_light(light* l);
void deferred_renderer_set_shadow_texture(texture* t);
void deferred_renderer_set_color_correction(asset_hndl t);
void deferred_renderer_set_vignetting(asset_hndl v);
void deferred_renderer_set_glitch(float glitch);

void deferred_renderer_add_light(light* l);
void deferred_renderer_remove_light(light* l);

void deferred_renderer_enable_skydome();
void deferred_renderer_disable_skydome();

void deferred_renderer_begin();
void deferred_renderer_end();

void deferred_renderer_render_static(static_object* s);
void deferred_renderer_render_static_with(static_object* s, shader_program* p);
void deferred_renderer_render_animated(animated_object* ao);
void deferred_renderer_render_light(light* l);
void deferred_renderer_render_axis(mat4 world);
void deferred_renderer_render_landscape(landscape* l);
void deferred_renderer_render_paint_circle(vec3 position, vec3 normal, float radius);

#endif
