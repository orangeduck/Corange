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
#include "assets/cmesh.h"
#include "assets/config.h"

#include "entities/camera.h"
#include "entities/light.h"
#include "entities/static_object.h"
#include "entities/instance_object.h"
#include "entities/animated_object.h"
#include "entities/particles.h"
#include "entities/projectile.h"
#include "entities/landscape.h"

#include "rendering/sky.h"

enum {
  DEFERRED_MAX_LIGHTS     = 16,
  DEFERRED_MAX_DYN_LIGHTS = 13,
};

enum {
  RO_TYPE_AXIS       = 0,
  RO_TYPE_STATIC     = 1,
  RO_TYPE_INSTANCE   = 2,
  RO_TYPE_ANIMATED   = 3,
  RO_TYPE_PARTICLES  = 4,
  RO_TYPE_LIGHT      = 5, 
  RO_TYPE_LANDSCAPE  = 6,
  RO_TYPE_PAINT      = 7,
  RO_TYPE_SPHERE     = 8,
  RO_TYPE_ELLIPSOID  = 9,
  RO_TYPE_CMESH      = 10,
  RO_TYPE_PROJECTILE = 11,
  RO_TYPE_FRUSTUM    = 12,
  RO_TYPE_PLANE      = 13,
  RO_TYPE_LINE       = 14,
  RO_TYPE_POINT      = 15,
};

typedef struct {
  int type;
  union {
  
    /* Geometry */
    mat4 axis;
    sphere sphere;
    ellipsoid ellipsoid;
    struct { cmesh* colmesh; mat4 colworld; };
    frustum frustum;
    plane plane;
    struct { vec3 line_start; vec3 line_end; vec3 line_color; float line_thickness; };
    struct { vec3 point_pos; vec3 point_color; float point_size; };
    
    /* Objects */
    static_object* static_object;
    instance_object* instance_object;
    animated_object* animated_object;
    landscape* landscape;
    particles* particles;
    projectile* projectile;
    
    /* UI */
    light* light;
    struct { mat4 paint_axis; float paint_radius; };
    
  };
} render_object;

render_object render_object_static(static_object* s);
render_object render_object_instance(instance_object* s);
render_object render_object_animated(animated_object* a);
render_object render_object_particles(particles* p);
render_object render_object_light(light* l);
render_object render_object_axis(mat4 a);
render_object render_object_sphere(sphere s);
render_object render_object_ellipsoid(ellipsoid e);
render_object render_object_frustum(frustum f);
render_object render_object_plane(plane p);
render_object render_object_cmesh(cmesh* cm, mat4 world);
render_object render_object_landscape(landscape* l);
render_object render_object_projectile(projectile* p);
render_object render_object_paint(mat4 paint_axis, float paint_radius);
render_object render_object_line(vec3 start, vec3 end, vec3 color, float thickness);
render_object render_object_point(vec3 pos, vec3 color, float size);

typedef struct {

  /* Options */
  asset_hndl options;

  /* Camera */
  camera* camera;
  
  /* Lights */
  int dyn_lights_num;
  light* dyn_light[DEFERRED_MAX_DYN_LIGHTS];

  /* Sky */
  sky* sky;
  
  /* Materials */
  asset_hndl mat_static;
  asset_hndl mat_skin;
  asset_hndl mat_instance;
  asset_hndl mat_animated;
  asset_hndl mat_vegetation;
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
  asset_hndl mat_depth_ins;
  asset_hndl mat_depth_ani;
  asset_hndl mat_depth_veg;
  asset_hndl mat_depth_ter;
  asset_hndl mat_sun;
  asset_hndl mat_clouds;
  asset_hndl mat_particles;
  asset_hndl mat_sea;

  /* Meshes */
  asset_hndl mesh_skydome;
  asset_hndl mesh_sphere;
  asset_hndl mesh_sea;

  /* Textures */
  asset_hndl tex_color_correction;
  asset_hndl tex_random;
  asset_hndl tex_random_perlin;
  asset_hndl tex_environment;
  asset_hndl tex_vignetting;
  asset_hndl tex_sea_bump0;
  asset_hndl tex_sea_bump1;
  asset_hndl tex_sea_bump2;
  asset_hndl tex_sea_bump3;
  asset_hndl tex_sea_env;
  asset_hndl tex_cube_sea;
  asset_hndl tex_cube_field;
  asset_hndl tex_white;
  asset_hndl tex_grey;
  asset_hndl tex_skin_lookup;
  
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
  int seed;
  float glitch;
  float time;
  float time_of_day;
  float exposure;
  float exposure_speed;
  float exposure_target;
  bool skydome_enabled;
  bool sea_enabled;
  
  /* Objects */
  int render_objects_num;
  render_object* render_objects;
  
  /* Preprocessed */
  
  mat4  camera_view;
  mat4  camera_proj;
  float camera_near;
  float camera_far;
  box   camera_frustum;
  
  mat4  shadow_view[3];
  mat4  shadow_proj[3];
  float shadow_near[3];
  float shadow_far[3];
  box   shadow_frustum[3];
  
} deferred_renderer;

deferred_renderer* deferred_renderer_new(asset_hndl options);
void deferred_renderer_delete(deferred_renderer* dr);

void deferred_renderer_set_camera(deferred_renderer* dr, camera* cam);
void deferred_renderer_set_color_correction(deferred_renderer* dr, asset_hndl t);
void deferred_renderer_set_vignetting(deferred_renderer* dr, asset_hndl v);
void deferred_renderer_set_glitch(deferred_renderer* dr, float glitch);
void deferred_renderer_set_skydome_enabled(deferred_renderer* dr, bool enabled);
void deferred_renderer_set_sea_enabled(deferred_renderer* dr, bool enabled);
void deferred_renderer_set_tod(deferred_renderer* dr, float tod, int seed);

void deferred_renderer_add(deferred_renderer* dr, render_object ro);
void deferred_renderer_add_dyn_light(deferred_renderer* dr, light* l);

void deferred_renderer_render(deferred_renderer* dr);

#endif
