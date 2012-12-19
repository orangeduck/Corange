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

enum {
  DEFERRED_MAX_LIGHTS     = 16,
  DEFERRED_MAX_DYN_LIGHTS = 13,
};

enum {
  RO_TYPE_AXIS      = 0,
  RO_TYPE_STATIC    = 1,
  RO_TYPE_ANIMATED  = 2,
  RO_TYPE_LIGHT     = 3, 
  RO_TYPE_LANDSCAPE = 4,
  RO_TYPE_PAINT     = 5,
};

typedef struct {
  int type;
  union {
    mat4 axis;
    static_object* static_object;
    animated_object* animated_object;
    light* light;
    landscape* landscape;
    struct { vec3 paint_pos; vec3 paint_norm; float paint_radius; };
  };
} render_object;

render_object render_object_static(static_object* s);
render_object render_object_animated(animated_object* a);
render_object render_object_light(light* l);
render_object render_object_axis(mat4 a);
render_object render_object_landscape(landscape* l);
render_object render_object_paint(vec3 paint_pos, vec3 paint_norm, float paint_radius);

typedef struct {

  /* Camera */
  camera* camera;
  
  /* Lights */
  light* sky_light;
  light* sun_light;
  light* moon_light;
  
  int dyn_lights_num;
  light* dyn_light[DEFERRED_MAX_DYN_LIGHTS];

  /* Materials */
  asset_hndl mat_static;
  asset_hndl mat_animated;
  asset_hndl mat_terrain;
  asset_hndl mat_clear;
  asset_hndl mat_ui;
  asset_hndl mat_ssao;
  asset_hndl mat_compose;
  asset_hndl mat_tonemap;
  asset_hndl mat_post0;
  asset_hndl mat_post1;
  asset_hndl mat_skydome;
  asset_hndl mat_depth;
  asset_hndl mat_depth_ani;
  asset_hndl mat_sun;

  /* Meshes */
  asset_hndl mesh_skydome;

  /* Textures */
  asset_hndl tex_color_correction;
  asset_hndl tex_random;
  asset_hndl tex_random_perlin;
  asset_hndl tex_environment;
  asset_hndl tex_vignetting;
  
  /* Buffers */
  GLuint gfbo;
  GLuint gdepth_buffer;
  GLuint gdiffuse_buffer;
  GLuint gpositions_buffer;
  GLuint gnormals_buffer;

  GLuint gdiffuse_texture;
  GLuint gpositions_texture;
  GLuint gnormals_texture;
  GLuint gdepth_texture;

  GLuint ssao_fbo;
  GLuint ssao_buffer;
  GLuint ssao_texture;

  GLuint hdr_fbo;
  GLuint hdr_buffer;
  GLuint hdr_texture;

  GLuint ldr_front_fbo;
  GLuint ldr_front_buffer;
  GLuint ldr_front_texture;

  GLuint ldr_back_fbo;
  GLuint ldr_back_buffer;
  GLuint ldr_back_texture;
  
  GLuint shadows_fbo[3];
  GLuint shadows_buffer[3];
  GLuint shadows_texture[3];
  
  /* Shadows */
  float shadows_start[3];
  float shadows_end[3];
  int shadows_widths[3];
  int shadows_heights[3];

  /* Variables */
  float glitch;
  float time;
  float time_of_day;
  float exposure;
  float exposure_speed;
  float exposure_target;
  bool skydome_enabled;
  
  /* Objects */
  int render_objects_num;
  render_object* render_objects;
  
} deferred_renderer;

deferred_renderer* deferred_renderer_new();
void deferred_renderer_delete(deferred_renderer* dr);

void deferred_renderer_set_camera(deferred_renderer* dr, camera* cam);
void deferred_renderer_set_color_correction(deferred_renderer* dr, asset_hndl t);
void deferred_renderer_set_vignetting(deferred_renderer* dr, asset_hndl v);
void deferred_renderer_set_glitch(deferred_renderer* dr, float glitch);
void deferred_renderer_set_skydome_enabled(deferred_renderer* dr, bool enabled);

void deferred_renderer_set_sun_light(deferred_renderer* dr, light* l);
void deferred_renderer_set_sky_light(deferred_renderer* dr, light* l);
void deferred_renderer_set_moon_light(deferred_renderer* dr, light* l);
int  deferred_renderer_num_dyn_light(deferred_renderer* dr);
void deferred_renderer_add_dyn_light(deferred_renderer* dr, light* l);
void deferred_renderer_rem_dyn_light(deferred_renderer* dr, light* l);

void deferred_renderer_add(deferred_renderer* dr, render_object ro);
void deferred_renderer_render(deferred_renderer* dr);

#endif
