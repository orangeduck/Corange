#include "rendering/deferred_renderer.h"

#include "cgraphics.h"
#include "centity.h"

#include "assets/shader.h"
#include "assets/texture.h"
#include "assets/material.h"
#include "assets/renderable.h"
#include "assets/terrain.h"
#include "assets/cmesh.h"

#include "rendering/sky.h"

render_object render_object_instance(instance_object* i) {
  render_object ro;
  ro.type = RO_TYPE_INSTANCE;
  ro.instance_object = i;
  return ro;
}

render_object render_object_static(static_object* s) {
  render_object ro;
  ro.type = RO_TYPE_STATIC;
  ro.static_object = s;
  return ro;
}

render_object render_object_animated(animated_object* a) {
  render_object ro;
  ro.type = RO_TYPE_ANIMATED;
  ro.animated_object = a;
  return ro;
}

render_object render_object_light(light* l) {
  render_object ro;
  ro.type = RO_TYPE_LIGHT;
  ro.light = l;
  return ro;
}

render_object render_object_axis(mat4 a) {
  render_object ro;
  ro.type = RO_TYPE_AXIS;
  ro.axis = a;
  return ro;
}

render_object render_object_landscape(landscape* l) {
  render_object ro;
  ro.type = RO_TYPE_LANDSCAPE;
  ro.landscape = l;
  return ro;
}

render_object render_object_particles(particles* p) {
  render_object ro;
  ro.type = RO_TYPE_PARTICLES;
  ro.particles = p;
  return ro;
}

render_object render_object_paint(mat4 paint_axis, float paint_radius) {
  render_object ro;
  ro.type = RO_TYPE_PAINT;
  ro.paint_axis = paint_axis;
  ro.paint_radius = paint_radius;
  return ro;
}

render_object render_object_sphere(sphere s) {
  render_object ro;
  ro.type = RO_TYPE_SPHERE;
  ro.sphere = s;
  return ro;
}

render_object render_object_cmesh(cmesh* cm, mat4 world) {
  render_object ro;
  ro.type = RO_TYPE_CMESH;
  ro.colmesh = cm;
  ro.colworld = world;
  return ro;
}

render_object render_object_ellipsoid(ellipsoid e) {
  render_object ro;
  ro.type = RO_TYPE_ELLIPSOID;
  ro.ellipsoid = e;
  return ro;
}

render_object render_object_frustum(frustum f) {
  render_object ro;
  ro.type = RO_TYPE_FRUSTUM;
  ro.frustum = f;
  return ro;
}

render_object render_object_plane(plane p) {
  render_object ro;
  ro.type = RO_TYPE_PLANE;
  ro.plane = p;
  return ro;
}

render_object render_object_projectile(projectile* p) {
  render_object ro;
  ro.type = RO_TYPE_PROJECTILE;
  ro.projectile = p;
  return ro;
}

render_object render_object_line(vec3 start, vec3 end, vec3 color, float thickness) {
  render_object ro;
  ro.type = RO_TYPE_LINE;
  ro.line_start = start;
  ro.line_end = end;
  ro.line_color = color;
  ro.line_thickness = thickness;
  return ro;
}

render_object render_object_point(vec3 pos, vec3 color, float size) {
  render_object ro;
  ro.type = RO_TYPE_POINT;
  ro.point_pos = pos;
  ro.point_color = color;
  ro.point_size = size;
  return ro;
}


static float quad_position[] = {
  -1, -1, 0, 
   1, -1, 0, 
   1,  1, 0, 
  -1, -1, 0, 
  -1,  1, 0, 
   1,  1, 0
};

static float quad_texcoord[] = {
  0, 0,
  1, 0,
  1, 1,
  0, 0,
  0, 1,
  1, 1
};

deferred_renderer* deferred_renderer_new(asset_hndl options) {
  
  deferred_renderer* dr = malloc(sizeof(deferred_renderer));
  
  /* Options */
  dr->options = options;
  
  /* Camera */
  dr->camera = NULL;

  /* Lights */
  dr->dyn_lights_num = 0;
  for(int i = 0; i < DEFERRED_MAX_DYN_LIGHTS; i++) {
    dr->dyn_light[i] = NULL;
  }
  
  /* Sky */
  dr->sky = sky_new();
  
  /* Materials */
  folder_load(P("$CORANGE/shaders/deferred/"));
  
  dr->mat_static     = asset_hndl_new(P("$CORANGE/shaders/deferred/static.mat"));
  dr->mat_skin       = asset_hndl_new(P("$CORANGE/shaders/deferred/skin.mat"));
  dr->mat_instance   = asset_hndl_new(P("$CORANGE/shaders/deferred/instance.mat"));
  dr->mat_animated   = asset_hndl_new(P("$CORANGE/shaders/deferred/animated.mat"));
  dr->mat_vegetation = asset_hndl_new(P("$CORANGE/shaders/deferred/vegetation.mat"));
  dr->mat_terrain    = asset_hndl_new(P("$CORANGE/shaders/deferred/terrain.mat"));
  dr->mat_clear      = asset_hndl_new(P("$CORANGE/shaders/deferred/clear.mat"));
  dr->mat_ssao       = asset_hndl_new(P("$CORANGE/shaders/deferred/ssao.mat"));
  dr->mat_tonemap    = asset_hndl_new(P("$CORANGE/shaders/deferred/tonemap.mat"));
  dr->mat_post0      = asset_hndl_new(P("$CORANGE/shaders/deferred/post0.mat"));
  dr->mat_post1      = asset_hndl_new(P("$CORANGE/shaders/deferred/post1.mat"));
  dr->mat_ui         = asset_hndl_new(P("$CORANGE/shaders/deferred/ui.mat"));
  dr->mat_skydome    = asset_hndl_new(P("$CORANGE/shaders/deferred/skydome.mat"));
  dr->mat_depth      = asset_hndl_new(P("$CORANGE/shaders/deferred/depth.mat"));
  dr->mat_depth_ins  = asset_hndl_new(P("$CORANGE/shaders/deferred/depth_instance.mat"));
  dr->mat_depth_ani  = asset_hndl_new(P("$CORANGE/shaders/deferred/depth_animated.mat"));
  dr->mat_depth_veg  = asset_hndl_new(P("$CORANGE/shaders/deferred/depth_vegetation.mat"));
  dr->mat_depth_ter  = asset_hndl_new(P("$CORANGE/shaders/deferred/depth_terrain.mat"));
  dr->mat_sun        = asset_hndl_new(P("$CORANGE/shaders/deferred/sun.mat"));
  dr->mat_clouds     = asset_hndl_new(P("$CORANGE/shaders/deferred/clouds.mat"));
  dr->mat_particles  = asset_hndl_new(P("$CORANGE/shaders/deferred/particles.mat"));
  dr->mat_sea        = asset_hndl_new(P("$CORANGE/shaders/deferred/sea.mat"));
  
  dr->mat_compose = option_graphics_asset(asset_hndl_ptr(&dr->options), "lighting",
    asset_hndl_new(P("$CORANGE/shaders/deferred/compose.mat")),
    asset_hndl_new(P("$CORANGE/shaders/deferred/compose.mat")),
    asset_hndl_new(P("$CORANGE/shaders/deferred/compose_low.mat")));
  
  /* Meshes */
  dr->mesh_skydome  = asset_hndl_new_load(P("$CORANGE/sky/skydome.bmf"));
  dr->mesh_sphere   = asset_hndl_new_load(P("$CORANGE/objects/sphere.bmf"));
  dr->mesh_sea      = asset_hndl_new_load(P("$CORANGE/water/sea.bmf"));
  
  /* Textures */
  dr->tex_color_correction  = asset_hndl_new_load(P("$CORANGE/luts/identity.lut"));
  dr->tex_random            = asset_hndl_new_load(P("$CORANGE/textures/random.dds"));
  dr->tex_random_perlin     = asset_hndl_new_load(P("$CORANGE/textures/random_perlin.dds"));
  dr->tex_environment       = asset_hndl_new_load(P("$CORANGE/textures/envmap.dds"));
  dr->tex_vignetting        = asset_hndl_new_load(P("$CORANGE/ui/vignetting.dds"));
  dr->tex_sea_bump0         = asset_hndl_new_load(P("$CORANGE/water/bump0.dds"));
  dr->tex_sea_bump1         = asset_hndl_new_load(P("$CORANGE/water/bump1.dds"));
  dr->tex_sea_bump2         = asset_hndl_new_load(P("$CORANGE/water/bump2.dds"));
  dr->tex_sea_bump3         = asset_hndl_new_load(P("$CORANGE/water/bump3.dds"));
  dr->tex_sea_env           = asset_hndl_new_load(P("$CORANGE/water/envmap_sea.dds"));
  dr->tex_cube_sea          = asset_hndl_new_load(P("$CORANGE/water/cube_sea.dds"));
  dr->tex_cube_field        = asset_hndl_new_load(P("$CORANGE/textures/cube_field.dds"));
  dr->tex_white             = asset_hndl_new_load(P("$CORANGE/textures/white.dds"));
  dr->tex_grey              = asset_hndl_new_load(P("$CORANGE/textures/grey.dds"));
  dr->tex_skin_lookup       = asset_hndl_new_load(P("$CORANGE/textures/skin_lookup.dds"));
  
  /* Buffers */
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  int gwidth  = width  * option_graphics_int(asset_hndl_ptr(&dr->options), "msaa", 4, 2, 1);
  int gheight = height * option_graphics_int(asset_hndl_ptr(&dr->options), "msaa", 4, 2, 1);
  
  glGenFramebuffers(1, &dr->gfbo);
  glBindFramebuffer(GL_FRAMEBUFFER, dr->gfbo);
  
  glGenRenderbuffers(1, &dr->gdiffuse_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, dr->gdiffuse_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, gwidth, gheight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, dr->gdiffuse_buffer);   
  
  glGenRenderbuffers(1, &dr->gnormals_buffer);  
  glBindRenderbuffer(GL_RENDERBUFFER, dr->gnormals_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F, gwidth, gheight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, dr->gnormals_buffer);  
  
  glGenRenderbuffers(1, &dr->gdepth_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, dr->gdepth_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, gwidth, gheight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, dr->gdepth_buffer);  
  
  glGenTextures(1, &dr->gdiffuse_texture);
  glBindTexture(GL_TEXTURE_2D, dr->gdiffuse_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gwidth, gheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dr->gdiffuse_texture, 0);
  
  glGenTextures(1, &dr->gnormals_texture);
  glBindTexture(GL_TEXTURE_2D, dr->gnormals_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gwidth, gheight, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, dr->gnormals_texture, 0);
  
  glGenTextures(1, &dr->gdepth_texture);
  glBindTexture(GL_TEXTURE_2D, dr->gdepth_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, gwidth, gheight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dr->gdepth_texture, 0);
  
  /* SSAO Buffer */
  
  int ssaowidth  = width  / option_graphics_int(asset_hndl_ptr(&dr->options), "ssao", 1, 2, 4);
  int ssaoheight = height / option_graphics_int(asset_hndl_ptr(&dr->options), "ssao", 1, 2, 4);
  
  glGenFramebuffers(1, &dr->ssao_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, dr->ssao_fbo);
  
  glGenRenderbuffers(1, &dr->ssao_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, dr->ssao_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, ssaowidth, ssaoheight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, dr->ssao_buffer);   
  
  glGenTextures(1, &dr->ssao_texture);
  glBindTexture(GL_TEXTURE_2D, dr->ssao_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ssaowidth, ssaoheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dr->ssao_texture, 0);
  
  /* HDR Buffer */
  
  int hdrwidth  = width  * option_graphics_int(asset_hndl_ptr(&dr->options), "msaa", 4, 2, 1);
  int hdrheight = height * option_graphics_int(asset_hndl_ptr(&dr->options), "msaa", 4, 2, 1);
  
  glGenFramebuffers(1, &dr->hdr_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, dr->hdr_fbo);
  
  glGenRenderbuffers(1, &dr->hdr_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, dr->hdr_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F, hdrwidth, hdrheight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, dr->hdr_buffer);   
  
  glGenTextures(1, &dr->hdr_texture);
  glBindTexture(GL_TEXTURE_2D, dr->hdr_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, hdrwidth, hdrheight, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dr->hdr_texture, 0);
  
  /* LDR front buffer */
  
  glGenFramebuffers(1, &dr->ldr_front_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, dr->ldr_front_fbo);
  
  glGenRenderbuffers(1, &dr->ldr_front_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, dr->ldr_front_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, dr->ldr_front_buffer);   
  
  glGenTextures(1, &dr->ldr_front_texture);
  glBindTexture(GL_TEXTURE_2D, dr->ldr_front_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dr->ldr_front_texture, 0);
    
  /* LDR back buffer */
  
  glGenFramebuffers(1, &dr->ldr_back_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, dr->ldr_back_fbo);
  
  glGenRenderbuffers(1, &dr->ldr_back_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, dr->ldr_back_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, dr->ldr_back_buffer);   
  
  glGenTextures(1, &dr->ldr_back_texture);
  glBindTexture(GL_TEXTURE_2D, dr->ldr_back_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dr->ldr_back_texture, 0);
  
  /* Shadow Buffers */
  
  int shadow_width  = option_graphics_int(asset_hndl_ptr(&dr->options), "shadows", 4096, 2048, 1024);
  int shadow_height = option_graphics_int(asset_hndl_ptr(&dr->options), "shadows", 4096, 2048, 1024);
  
  dr->shadows_start[0] = 0.000; dr->shadows_end[0] = 0.060;
  dr->shadows_start[1] = 0.060; dr->shadows_end[1] = 0.070;
  dr->shadows_start[2] = 0.070; dr->shadows_end[2] = 0.200;
  
  dr->shadows_widths[0] = shadow_width; dr->shadows_heights[0] = shadow_height;
  dr->shadows_widths[1] = shadow_width; dr->shadows_heights[1] = shadow_height;
  dr->shadows_widths[2] = shadow_width; dr->shadows_heights[2] = shadow_height;
  
  for (int i = 0; i < 3; i++) {
    
    glGenFramebuffers(1, &dr->shadows_fbo[i]);
    glBindFramebuffer(GL_FRAMEBUFFER, dr->shadows_fbo[i]);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    glGenRenderbuffers(1, &dr->shadows_buffer[i]);
    glBindRenderbuffer(GL_RENDERBUFFER, dr->shadows_buffer[i]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, dr->shadows_widths[i], dr->shadows_heights[i]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, dr->shadows_buffer[i]);  
    
    glGenTextures(1, &dr->shadows_texture[i]);
    glBindTexture(GL_TEXTURE_2D, dr->shadows_texture[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, dr->shadows_widths[i], dr->shadows_heights[i], 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dr->shadows_texture[i], 0);
    
  }
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  /* Variables */
  dr->seed = 0;
  dr->glitch = 0.0;
  dr->time = 0.0;
  dr->time_of_day = 0;
  dr->exposure = 0.0;
  dr->exposure_speed = 1.0;
  dr->exposure_target = 0.4;
  dr->skydome_enabled = true;
  dr->sea_enabled = false;
  
  /* Objects */
  dr->render_objects_num = 0;
  dr->render_objects = NULL;
  
  glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, config_float(asset_hndl_ptr(&dr->options), "lod_bias"));
  
  SDL_GL_CheckError();
  SDL_GL_CheckFrameBuffer();
  
  return dr;
  
}

void deferred_renderer_delete(deferred_renderer* dr) {
  
  glDeleteFramebuffers(1, &dr->gfbo);
  
  glDeleteRenderbuffers(1, &dr->gdiffuse_buffer);
  glDeleteRenderbuffers(1, &dr->gnormals_buffer);
  glDeleteRenderbuffers(1, &dr->gdepth_buffer);
  
  glDeleteTextures(1, &dr->gdiffuse_texture);
  glDeleteTextures(1, &dr->gnormals_texture);
  glDeleteTextures(1, &dr->gdepth_texture);
  
  glDeleteFramebuffers(1, &dr->ssao_fbo);
  glDeleteRenderbuffers(1, &dr->ssao_buffer);
  glDeleteTextures(1, &dr->ssao_texture);
  
  glDeleteFramebuffers(1, &dr->hdr_fbo);
  glDeleteRenderbuffers(1, &dr->hdr_buffer);
  glDeleteTextures(1, &dr->hdr_texture);
  
  glDeleteFramebuffers(1, &dr->ldr_front_fbo);
  glDeleteRenderbuffers(1, &dr->ldr_front_buffer);
  glDeleteTextures(1, &dr->ldr_front_texture);
  
  glDeleteFramebuffers(1, &dr->ldr_back_fbo);
  glDeleteRenderbuffers(1, &dr->ldr_back_buffer);
  glDeleteTextures(1, &dr->ldr_back_texture);
  
  glDeleteFramebuffers(3, dr->shadows_fbo);
  glDeleteRenderbuffers(3, dr->shadows_buffer);
  glDeleteTextures(3, dr->shadows_texture);
  
  free(dr->render_objects);
    
  folder_unload(P("$CORANGE/shaders/deferred/"));
  
  sky_delete(dr->sky);
  
  free(dr);
}

void deferred_renderer_set_camera(deferred_renderer* dr, camera* cam) {
  dr->camera = cam;
}

void deferred_renderer_set_color_correction(deferred_renderer* dr, asset_hndl t) {
  dr->tex_color_correction = t;
}

void deferred_renderer_set_vignetting(deferred_renderer* dr, asset_hndl v) {
  dr->tex_vignetting = v;
}

void deferred_renderer_set_glitch(deferred_renderer* dr, float glitch) {
  dr->glitch = glitch;
}

void deferred_renderer_set_skydome_enabled(deferred_renderer* dr, bool enabled) {
  dr->skydome_enabled = enabled;
}

void deferred_renderer_set_sea_enabled(deferred_renderer* dr, bool enabled) {
  dr->sea_enabled = enabled;
}

void deferred_renderer_set_tod(deferred_renderer* dr, float tod, int seed) {
  dr->time_of_day = tod;
  sky_update(dr->sky, dr->time_of_day, 0);
}

void deferred_renderer_add_dyn_light(deferred_renderer* dr, light* l) {

  if (dr->dyn_lights_num == DEFERRED_MAX_DYN_LIGHTS) {
    warning("Cannot add extra light. Maxiumum lights reached!");
    return;
  }
  
  dr->dyn_light[dr->dyn_lights_num] = l;
  dr->dyn_lights_num++;

}

void deferred_renderer_add(deferred_renderer* dr, render_object ro) {
  dr->render_objects_num++;
  dr->render_objects = realloc(dr->render_objects, sizeof(render_object) * dr->render_objects_num);
  dr->render_objects[dr->render_objects_num-1] = ro;
}

static int round_to(float x, int multiple) {
  return (int)(x / multiple) * multiple;
}

static void shadow_mapper_transforms(deferred_renderer* dr, int i, mat4* view, mat4* proj, float* nearclip, float* farclip) {
  
  frustum f = frustum_new_clipbox();
  f = frustum_transform(f, dr->camera_inv_proj);
  f = frustum_transform(f, dr->camera_inv_view);
  f = frustum_slice(f, dr->shadows_start[i], dr->shadows_end[i]);
  
  vec3 center = frustum_center(f);
  if (dr->sky->is_day) {
    *view = mat4_view_look_at(center, vec3_add(center, dr->sky->sun_direction), vec3_up());
  } else {
    *view = mat4_view_look_at(center, vec3_add(center, dr->sky->moon_direction), vec3_up());
  }
  
  f = frustum_translate(f, vec3_neg(center));
  f = frustum_transform(f, *view);
  f = frustum_translate(f, vec3_neg(center));
  
  vec3 maximums = frustum_maximums(f);
  vec3 minimums = frustum_minimums(f);
  
  float rangex = max(maximums.x, -minimums.x);
  float rangey = max(maximums.y, -minimums.y);
  float rangez = max(maximums.z, -minimums.z);
  float range  = round_to(max(max(rangex, rangey), rangez), 10);
  
  float pixel = dr->shadows_widths[i] / range;
  
  vec3 offset = vec3_fmod(center, pixel);
  center = vec3_sub(center, offset);
  
  if (dr->sky->is_day) {
    *view = mat4_view_look_at(center, vec3_add(center, dr->sky->sun_direction), vec3_up());
  } else {
    *view = mat4_view_look_at(center, vec3_add(center, dr->sky->moon_direction), vec3_up());
  }
  
  *nearclip = -range;
  *farclip = range;
  *proj = mat4_orthographic(-range, range, -range, range, -range, range);
  
}

static void render_shadows_vegetation(deferred_renderer* dr, int i, instance_object* io) {
  
  if (sphere_outside_box(io->bound, dr->camera_frustum)) { return; }
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_depth_veg));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "view", dr->shadow_view[i]);
  shader_program_set_mat4(shader, "proj", dr->shadow_proj[i]);
  shader_program_set_float(shader, "clip_near", dr->shadow_near[i]);
  shader_program_set_float(shader, "clip_far",  dr->shadow_far[i]);
  shader_program_set_float(shader, "time", dr->time);
  
  renderable* r = asset_hndl_ptr(&io->renderable);

  if(r->is_rigged) { error("Static Object is rigged!"); }
  
  for(int j = 0; j < r->num_surfaces; j++) {
    
    renderable_surface* s = r->surfaces[j];
    
    material_entry* me = material_get_entry(asset_hndl_ptr(&r->material), j);
    
    if (material_entry_has_item(me, "alpha_test")) {
      shader_program_set_texture(shader, "diffuse_map", 0, material_entry_item(me, "diffuse_map").as_asset);
      shader_program_set_float(shader, "alpha_test", material_entry_item(me, "alpha_test").as_float);
    } else {
      shader_program_set_texture(shader, "diffuse_map", 0, dr->tex_white);
      shader_program_set_float(shader, "alpha_test", 0.0);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    
    shader_program_enable_attribute(shader, "vPosition", 3, 18, (void*)0);
    shader_program_enable_attribute(shader, "vTexcoord", 2, 18, (void*)(sizeof(float) * 12));
    shader_program_enable_attribute(shader, "vColor",    4, 18, (void*)(sizeof(float) * 14));
    
    glBindBuffer(GL_ARRAY_BUFFER, io->world_buffer);
    
    shader_program_enable_attribute_instance_matrix(shader, "vWorld", (void*)0);
    
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      glDrawElementsInstanced(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0, io->num_instances);
    
    shader_program_disable_attribute(shader, "vPosition");
    shader_program_disable_attribute(shader, "vTexcoord");
    shader_program_disable_attribute(shader, "vColor");
    shader_program_disable_attribute_matrix(shader, "vWorld");
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
  }
  
  shader_program_disable(shader);

}

static void render_shadows_static(deferred_renderer* dr, int i, static_object* s) {
  
  mat4 world = mat4_world(s->position, s->scale, s->rotation);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_depth));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", world);
  shader_program_set_mat4(shader, "view",  dr->shadow_view[i]);
  shader_program_set_mat4(shader, "proj",  dr->shadow_proj[i]);
  shader_program_set_float(shader, "clip_near", dr->shadow_near[i]);
  shader_program_set_float(shader, "clip_far",  dr->shadow_far[i]);
  
  renderable* r = asset_hndl_ptr(&s->renderable);

  if(r->is_rigged) { error("Static Object is rigged!"); }
  
  for(int j = 0; j < r->num_surfaces; j++) {
    
    renderable_surface* s = r->surfaces[j];
    
    if (sphere_outside_box(sphere_transform(s->bound, world), dr->shadow_frustum[i])) { continue; }
    
    material_entry* me = material_get_entry(asset_hndl_ptr(&r->material), j);
    
    if (material_entry_has_item(me, "alpha_test")) {
      shader_program_set_texture(shader, "diffuse_map", 0, material_entry_item(me, "diffuse_map").as_asset);
      shader_program_set_float(shader, "alpha_test", material_entry_item(me, "alpha_test").as_float);
    } else {
      shader_program_set_texture(shader, "diffuse_map", 0, dr->tex_white);
      shader_program_set_float(shader, "alpha_test", 0.0);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
    
    shader_program_enable_attribute(shader, "vPosition", 3, 18, (void*)0);
    shader_program_enable_attribute(shader, "vTexcoord", 2, 18, (void*)(sizeof(float) * 12));
    
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
    
    shader_program_disable_attribute(shader, "vPosition");
    shader_program_disable_attribute(shader, "vTexcoord");
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  
  shader_program_disable(shader);
  
}

static void render_shadows_instance(deferred_renderer* dr, int i, instance_object* io) {
  
  if (sphere_outside_box(io->bound, dr->camera_frustum)) { return; }
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_depth_ins));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "view",  dr->shadow_view[i]);
  shader_program_set_mat4(shader, "proj",  dr->shadow_proj[i]);
  shader_program_set_float(shader, "clip_near", dr->shadow_near[i]);
  shader_program_set_float(shader, "clip_far",  dr->shadow_far[i]);
  
  renderable* r = asset_hndl_ptr(&io->renderable);

  if(r->is_rigged) { error("Static Object is rigged!"); }
  
  for(int j = 0; j < r->num_surfaces; j++) {
    
    renderable_surface* s = r->surfaces[j];
    
    material_entry* me = material_get_entry(asset_hndl_ptr(&r->material), j);
    
    if (material_entry_has_item(me, "alpha_test")) {
      shader_program_set_texture(shader, "diffuse_map", 0, material_entry_item(me, "diffuse_map").as_asset);
      shader_program_set_float(shader, "alpha_test", material_entry_item(me, "alpha_test").as_float);
    } else {
      shader_program_set_texture(shader, "diffuse_map", 0, dr->tex_white);
      shader_program_set_float(shader, "alpha_test", 0.0);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    
    shader_program_enable_attribute(shader, "vPosition", 3, 18, (void*)0);
    shader_program_enable_attribute(shader, "vTexcoord", 2, 18, (void*)(sizeof(float) * 12));
    
    glBindBuffer(GL_ARRAY_BUFFER, io->world_buffer);
    
    shader_program_enable_attribute_instance_matrix(shader, "vWorld", (void*)0);
    
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      glDrawElementsInstanced(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0, io->num_instances);
    
    shader_program_disable_attribute(shader, "vPosition");
    shader_program_disable_attribute(shader, "vTexcoord");
    shader_program_disable_attribute_matrix(shader, "vWorld");
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
  }
  
  shader_program_disable(shader);
  
}

enum {
  MAX_BONES = 64,
};

static mat4 bone_matrices[MAX_BONES];

static void render_shadows_animated(deferred_renderer* dr, int i, animated_object* ao) {
  
  mat4 world = mat4_world( ao->position, ao->scale, ao->rotation );
  
  skeleton* skel = asset_hndl_ptr(&ao->skeleton);

  if (skel->joint_count > MAX_BONES) { error("animated object skeleton has too many bones (over %i)", MAX_BONES); }
  if (ao->pose == NULL) { return; }
  
  for(int j = 0; j < skel->joint_count; j++) {
    mat4 base = skel->rest_pose->transforms_inv[j];
    mat4 ani = ao->pose->transforms[j];
    bone_matrices[j] = mat4_mul_mat4(ani, base);
  }
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_depth_ani));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", world);
  shader_program_set_mat4(shader, "view",  dr->shadow_view[i]);
  shader_program_set_mat4(shader, "proj",  dr->shadow_proj[i]);
  shader_program_set_mat4_array(shader, "world_bones", bone_matrices, skel->joint_count);
  shader_program_set_float(shader, "clip_near", dr->shadow_near[i]);
  shader_program_set_float(shader, "clip_far",  dr->shadow_far[i]);
  
  renderable* r = asset_hndl_ptr(&ao->renderable);
  
  if(!r->is_rigged) { error("animated object is not rigged"); }
  
  for(int j = 0; j < r->num_surfaces; j++) {
    renderable_surface* s = r->surfaces[j];
    
    //if (sphere_outside_box(sphere_transform(s->bound, world), dr->shadow_frustum[i])) { continue; }
    
    material_entry* me = material_get_entry(asset_hndl_ptr(&r->material), j);
    
    if (material_entry_has_item(me, "alpha_test")) {
      shader_program_set_texture(shader, "diffuse_map", 0, material_entry_item(me, "diffuse_map").as_asset);
      shader_program_set_float(shader, "alpha_test", material_entry_item(me, "alpha_test").as_float);
    } else {
      shader_program_set_texture(shader, "diffuse_map", 0, dr->tex_white);
      shader_program_set_float(shader, "alpha_test", 0.0);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
    
    shader_program_enable_attribute(shader, "vPosition",      3, 24, (void*)0);
    shader_program_enable_attribute(shader, "vTexcoord",      2, 24, (void*)(sizeof(float) * 12));
    shader_program_enable_attribute(shader, "vBone_indicies", 3, 24, (void*)(sizeof(float) * 18));
    shader_program_enable_attribute(shader, "vBone_weights",  3, 24, (void*)(sizeof(float) * 21));
    
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
      
    shader_program_disable_attribute(shader, "vPosition");
    shader_program_disable_attribute(shader, "vTexcoord");
    shader_program_disable_attribute(shader, "vBone_indicies");
    shader_program_disable_attribute(shader, "vBone_weights");
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
  }
  
  shader_program_disable(shader);

}

static void render_shadows_landscape_blobtree(deferred_renderer* dr, int i, shader* shader, landscape_blobtree* lbt, terrain* terr) {

  if (sphere_outside_box(lbt->bound, dr->shadow_frustum[i])) { return; }
  
  if (!lbt->is_leaf) {
    render_shadows_landscape_blobtree(dr, i, shader, lbt->child0, terr);
    render_shadows_landscape_blobtree(dr, i, shader, lbt->child1, terr);
    render_shadows_landscape_blobtree(dr, i, shader, lbt->child2, terr);
    render_shadows_landscape_blobtree(dr, i, shader, lbt->child3, terr);
    return;
  }
  
  terrain_chunk* tc = terr->chunks[lbt->chunk_index];
    
  float dist = vec3_dist_sqrd(dr->camera->position, lbt->bound.center) / (100 * NUM_TERRAIN_BUFFERS);
  int buff_index = clamp(dist, 0, NUM_TERRAIN_BUFFERS-1);
  
  glBindBuffer(GL_ARRAY_BUFFER, tc->vertex_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tc->index_buffers[buff_index]);
  
  shader_program_enable_attribute(shader, "vPosition", 3, 12, (void*)0);
    
    glDrawElements(GL_TRIANGLES, tc->num_indicies[buff_index], GL_UNSIGNED_INT, (void*)0);
  
  shader_program_disable_attribute(shader, "vPosition");

}

static void render_shadows_landscape(deferred_renderer* dr, int i, landscape* l) {
  
  terrain* terr = asset_hndl_ptr(&l->heightmap);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_depth_ter));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", landscape_world(l));
  shader_program_set_mat4(shader, "view",  dr->shadow_view[i]);
  shader_program_set_mat4(shader, "proj",  dr->shadow_proj[i]);
  shader_program_set_float(shader, "clip_near", dr->shadow_near[i]);
  shader_program_set_float(shader, "clip_far",  dr->shadow_far[i]);
  
  if (unlikely(l->blobtree == NULL)) { error("Blobtree must be generated for landscape"); }
  
  render_shadows_landscape_blobtree(dr, i, shader, l->blobtree, terr);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  
  shader_program_disable(shader);

}

void render_shadows_projectile(deferred_renderer* dr, int i, projectile* p) {

  static_object so;
  so.position = p->position;
  so.rotation = p->rotation;
  so.scale = vec3_new(1, 1, 1);
  so.renderable = p->mesh;
  so.collision_body = asset_hndl_null();
  
  render_shadows_static(dr, i, &so);

}

static void render_shadows(deferred_renderer* dr) {
  
  dr->camera_inv_view = mat4_inverse(camera_view_matrix(dr->camera));
  dr->camera_inv_proj = mat4_inverse(camera_proj_matrix(dr->camera));
  
  if (config_int(asset_hndl_ptr(&dr->options), "shadows") == 0) return;
  
  for (int i = 0; i < 3; i++) {
    
    shadow_mapper_transforms(dr, i,
      &dr->shadow_view[i], &dr->shadow_proj[i],
      &dr->shadow_near[i], &dr->shadow_far[i]);
    
    dr->shadow_frustum[i] = box_invert_depth(box_invert(frustum_box(frustum_new_camera(dr->shadow_view[i], dr->shadow_proj[i]))));
    
    glBindFramebuffer(GL_FRAMEBUFFER, dr->shadows_fbo[i]);  
    glViewport( 0, 0, dr->shadows_widths[i], dr->shadows_heights[i]);
    glClearDepth(1.0f);  
    glClear(GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    
    for ( int j = 0; j < dr->render_objects_num; j++) {
      
      // HACK ALERT
      bool veg_found = false;
      if (dr->render_objects[j].type == RO_TYPE_INSTANCE) {
        renderable* r = asset_hndl_ptr(&dr->render_objects[j].instance_object->renderable);
        material* m = asset_hndl_ptr(&r->material);        
        
        for (int k = 0; k < m->num_entries; k++) {
          if (material_entry_item(m->entries[k], "material").as_int == 6) {
            render_shadows_vegetation(dr, i, dr->render_objects[j].instance_object);
            veg_found = true;
            break;
          }
        }
        
      }
      
      if (veg_found) continue;
      
      if (dr->render_objects[j].type == RO_TYPE_STATIC) { render_shadows_static(dr, i, dr->render_objects[j].static_object); }
      if (dr->render_objects[j].type == RO_TYPE_INSTANCE) { render_shadows_instance(dr, i, dr->render_objects[j].instance_object); }
      if (dr->render_objects[j].type == RO_TYPE_ANIMATED) { render_shadows_animated(dr, i, dr->render_objects[j].animated_object); }
      if (dr->render_objects[j].type == RO_TYPE_LANDSCAPE) { render_shadows_landscape(dr, i, dr->render_objects[j].landscape); }
      if (dr->render_objects[j].type == RO_TYPE_PROJECTILE) { render_shadows_projectile(dr, i, dr->render_objects[j].projectile); }
      
    }
    
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    
    glViewport( 0, 0, graphics_viewport_width(), graphics_viewport_height());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  }
    
}

void render_ellipsoid(deferred_renderer* dr, ellipsoid e);
void render_plane(deferred_renderer* dr, plane p);

static void render_clear(deferred_renderer* dr) {

  glBindFramebuffer(GL_FRAMEBUFFER, dr->gfbo);
  glDrawBuffers(2, (GLenum[]){ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 });
  glViewport( 0, 0, graphics_viewport_width(), graphics_viewport_height());
  glClearColor(0.2, 0.2, 0.2, 1.0f);
  glClearDepth(1.0f);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_clear));
  shader_program_enable(shader);
  shader_program_set_vec4(shader, "start", vec4_new(0.5, 0.5, 0.5, 1.0));
  shader_program_set_vec4(shader, "end",   vec4_new(0.0, 0.0, 0.0, 1.0));
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view",  mat4_id());
  shader_program_set_mat4(shader, "proj",  mat4_orthographic(-1, 1, -1, 1, -1, 1));
  shader_program_enable_attribute(shader, "vPosition", 3, 3, quad_position);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
  
  shader_program_disable_attribute(shader, "vPosition");
  shader_program_disable(shader);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);  
  
}

static void render_cmesh(deferred_renderer* dr, cmesh* cm, mat4 world) {
  
  if (!cm->is_leaf) {
    render_cmesh(dr, cm->front, world);
    render_cmesh(dr, cm->back,  world);
    return;
  }
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_static));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", world);
  shader_program_set_mat4(shader, "view", dr->camera_view);
  shader_program_set_mat4(shader, "proj", dr->camera_proj);
  shader_program_set_float(shader, "clip_near", dr->camera_near);
  shader_program_set_float(shader, "clip_far",  dr->camera_far);
  
  material_entry* me = material_get_entry(asset_get_load(P("$CORANGE/shaders/basic.mat")), 0);
  
  shader_program_set_texture(shader, "diffuse_map", 0, material_entry_item(me, "diffuse_map").as_asset);
  shader_program_set_texture(shader, "bump_map", 1, material_entry_item(me, "bump_map").as_asset);
  shader_program_set_texture(shader, "spec_map", 2, material_entry_item(me, "spec_map").as_asset);
  shader_program_set_float(shader, "glossiness", material_entry_item(me, "glossiness").as_float);
  shader_program_set_float(shader, "bumpiness", material_entry_item(me, "bumpiness").as_float);
  shader_program_set_float(shader, "specular_level", material_entry_item(me, "specular_level").as_float);
  shader_program_set_float(shader, "alpha_test", 0);
  shader_program_set_int(shader, "material", material_entry_item(me, "material").as_int);
  
  vec3* positions = malloc(sizeof(vec3) * cm->triangles_num * 3);
  vec3* normals   = malloc(sizeof(vec3) * cm->triangles_num * 3);
  
  for (int i = 0; i < cm->triangles_num * 3; i += 3) {
    ctri t = cm->triangles[i / 3];
    
    positions[i+0] = t.a;
    positions[i+1] = t.b;
    positions[i+2] = t.c;
    
    normals[i+0] = t.norm;
    normals[i+1] = t.norm;
    normals[i+2] = t.norm;
  }
  
  //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  
  shader_program_enable_attribute(shader, "vPosition",  3, 3, positions);
  shader_program_enable_attribute(shader, "vNormal",    3, 3, normals);
  shader_program_enable_attribute(shader, "vTangent",   3, 3, normals);
  shader_program_enable_attribute(shader, "vBinormal",  3, 3, normals);
  shader_program_enable_attribute(shader, "vTexcoord",  2, 2, normals);
    
    glDrawArrays(GL_TRIANGLES, 0, cm->triangles_num * 3);
  
  shader_program_disable_attribute(shader, "vPosition");
  shader_program_disable_attribute(shader, "vNormal");
  shader_program_disable_attribute(shader, "vTangent");
  shader_program_disable_attribute(shader, "vBinormal");
  shader_program_disable_attribute(shader, "vTexcoord");
  
  //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  
  free(positions);
  free(normals);
  
  shader_program_disable(shader);
    
}

static void render_static(deferred_renderer* dr, static_object* so) {
  
  mat4 world = mat4_world( so->position, so->scale, so->rotation );
  
  if (config_bool(asset_hndl_ptr(&dr->options), "render_colmeshes")) {
    if (!file_isloaded(so->collision_body.path)) {
      file_load(so->collision_body.path);
    }
    render_cmesh(dr, asset_hndl_ptr(&so->collision_body), world);
  }
  
  renderable* r = asset_hndl_ptr(&so->renderable);
  
  if(r->is_rigged) { error("Static object is rigged!"); }
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_static));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", world);
  shader_program_set_mat4(shader, "view", dr->camera_view);
  shader_program_set_mat4(shader, "proj", dr->camera_proj);
  shader_program_set_float(shader, "clip_near", dr->camera_near);
  shader_program_set_float(shader, "clip_far",  dr->camera_far);
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    
    if (sphere_outside_box(sphere_transform(s->bound, world), dr->camera_frustum)) { continue; }
    
    material_entry* me = material_get_entry(asset_hndl_ptr(&r->material), i);
    
    if (config_bool(asset_hndl_ptr(&dr->options), "render_white")) {
      shader_program_set_texture(shader, "diffuse_map", 0, dr->tex_grey);
    } else {
      shader_program_set_texture(shader, "diffuse_map", 0, material_entry_item(me, "diffuse_map").as_asset);
    }
    shader_program_set_texture(shader, "bump_map", 1, material_entry_item(me, "bump_map").as_asset);
    shader_program_set_texture(shader, "spec_map", 2, material_entry_item(me, "spec_map").as_asset);
    shader_program_set_float(shader, "glossiness", material_entry_item(me, "glossiness").as_float);
    shader_program_set_float(shader, "bumpiness", material_entry_item(me, "bumpiness").as_float);
    shader_program_set_float(shader, "specular_level", material_entry_item(me, "specular_level").as_float);
    shader_program_set_float(shader, "alpha_test", material_entry_item(me, "alpha_test").as_float);
    shader_program_set_int(shader, "material", material_entry_item(me, "material").as_int);
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    
    shader_program_enable_attribute(shader, "vPosition",  3, 18, (void*)0);
    shader_program_enable_attribute(shader, "vNormal",    3, 18, (void*)(sizeof(float) * 3));
    shader_program_enable_attribute(shader, "vTangent",   3, 18, (void*)(sizeof(float) * 6));
    shader_program_enable_attribute(shader, "vBinormal",  3, 18, (void*)(sizeof(float) * 9));
    shader_program_enable_attribute(shader, "vTexcoord",  2, 18, (void*)(sizeof(float) * 12));
    
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
    
    shader_program_disable_attribute(shader, "vPosition");
    shader_program_disable_attribute(shader, "vNormal");
    shader_program_disable_attribute(shader, "vTangent");
    shader_program_disable_attribute(shader, "vBinormal");
    shader_program_disable_attribute(shader, "vTexcoord");
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

  }
  
  shader_program_disable(shader);

}

static void render_skin(deferred_renderer* dr, instance_object* io) {
  
  if (sphere_outside_box(io->bound, dr->camera_frustum)) { return; }
  
  if (config_bool(asset_hndl_ptr(&dr->options), "render_colmeshes")) {
    if (!file_isloaded(io->collision_body.path)) {
      file_load(io->collision_body.path);
    }
    //render_cmesh(dr, asset_hndl_ptr(io->collision_body), world);
  }
  
  renderable* r = asset_hndl_ptr(&io->renderable);
  
  if(r->is_rigged) { error("Static object is rigged!"); }
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_skin));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "view", dr->camera_view);
  shader_program_set_mat4(shader, "proj", dr->camera_proj);
  shader_program_set_float(shader, "clip_near", dr->camera_near);
  shader_program_set_float(shader, "clip_far",  dr->camera_far);
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    
    material_entry* me = material_get_entry(asset_hndl_ptr(&r->material), i);
    
    if (config_bool(asset_hndl_ptr(&dr->options), "render_white")) {
      shader_program_set_texture(shader, "diffuse_map", 0, dr->tex_grey);
    } else {
      shader_program_set_texture(shader, "diffuse_map", 0, material_entry_item(me, "diffuse_map").as_asset);
    }
    shader_program_set_texture(shader, "bump_map", 1, material_entry_item(me, "bump_map").as_asset);
    shader_program_set_texture(shader, "spec_map", 2, material_entry_item(me, "spec_map").as_asset);
    shader_program_set_texture(shader, "curvature_map", 3, material_entry_item(me, "curvature_map").as_asset);
    shader_program_set_float(shader, "bumpiness", material_entry_item(me, "bumpiness").as_float);
    shader_program_set_float(shader, "specular_level", material_entry_item(me, "specular_level").as_float);
    shader_program_set_float(shader, "alpha_test", material_entry_item(me, "alpha_test").as_float);
    shader_program_set_int(shader, "material", material_entry_item(me, "material").as_int);
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    
    shader_program_enable_attribute(shader, "vPosition",  3, 18, (void*)0);
    shader_program_enable_attribute(shader, "vNormal",    3, 18, (void*)(sizeof(float) * 3));
    shader_program_enable_attribute(shader, "vTangent",   3, 18, (void*)(sizeof(float) * 6));
    shader_program_enable_attribute(shader, "vBinormal",  3, 18, (void*)(sizeof(float) * 9));
    shader_program_enable_attribute(shader, "vTexcoord",  2, 18, (void*)(sizeof(float) * 12));
    
    glBindBuffer(GL_ARRAY_BUFFER, io->world_buffer);
    
    shader_program_enable_attribute_instance_matrix(shader, "vWorld", (void*)0);
    
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      glDrawElementsInstanced(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0, io->num_instances);
    
    shader_program_disable_attribute(shader, "vPosition");
    shader_program_disable_attribute(shader, "vNormal");
    shader_program_disable_attribute(shader, "vTangent");
    shader_program_disable_attribute(shader, "vBinormal");
    shader_program_disable_attribute(shader, "vTexcoord");
    shader_program_disable_attribute_matrix(shader, "vWorld");
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

  }
  
  shader_program_disable(shader);

}

static void render_instance(deferred_renderer* dr, instance_object* io) {
  
  if (sphere_outside_box(io->bound, dr->camera_frustum)) { return; }
  
  if (config_bool(asset_hndl_ptr(&dr->options), "render_colmeshes")) {
    if (!file_isloaded(io->collision_body.path)) {
      file_load(io->collision_body.path);
    }
    for (int i = 0; i < io->num_instances; i++) {
      render_cmesh(dr, asset_hndl_ptr(&io->collision_body), io->instances[i].world);
    }
  }
  
  renderable* r = asset_hndl_ptr(&io->renderable);
  
  if(r->is_rigged) { error("Static object is rigged!"); }
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_instance));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "view", dr->camera_view);
  shader_program_set_mat4(shader, "proj", dr->camera_proj);
  shader_program_set_float(shader, "clip_near", dr->camera_near);
  shader_program_set_float(shader, "clip_far",  dr->camera_far);
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    
    material_entry* me = material_get_entry(asset_hndl_ptr(&r->material), i);
    
    if (config_bool(asset_hndl_ptr(&dr->options), "render_white")) {
      shader_program_set_texture(shader, "diffuse_map", 0, dr->tex_grey);
    } else {
      shader_program_set_texture(shader, "diffuse_map", 0, material_entry_item(me, "diffuse_map").as_asset);
    }
    shader_program_set_texture(shader, "bump_map", 1, material_entry_item(me, "bump_map").as_asset);
    shader_program_set_texture(shader, "spec_map", 2, material_entry_item(me, "spec_map").as_asset);
    shader_program_set_float(shader, "glossiness", material_entry_item(me, "glossiness").as_float);
    shader_program_set_float(shader, "bumpiness", material_entry_item(me, "bumpiness").as_float);
    shader_program_set_float(shader, "specular_level", material_entry_item(me, "specular_level").as_float);
    shader_program_set_float(shader, "alpha_test", material_entry_item(me, "alpha_test").as_float);
    shader_program_set_int(shader, "material", material_entry_item(me, "material").as_int);
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    
    shader_program_enable_attribute(shader, "vPosition",  3, 18, (void*)0);
    shader_program_enable_attribute(shader, "vNormal",    3, 18, (void*)(sizeof(float) * 3));
    shader_program_enable_attribute(shader, "vTangent",   3, 18, (void*)(sizeof(float) * 6));
    shader_program_enable_attribute(shader, "vBinormal",  3, 18, (void*)(sizeof(float) * 9));
    shader_program_enable_attribute(shader, "vTexcoord",  2, 18, (void*)(sizeof(float) * 12));
    
    glBindBuffer(GL_ARRAY_BUFFER, io->world_buffer);
    
    shader_program_enable_attribute_instance_matrix(shader, "vWorld", (void*)0);
    
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      glDrawElementsInstanced(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0, io->num_instances);
    
    shader_program_disable_attribute(shader, "vPosition");
    shader_program_disable_attribute(shader, "vNormal");
    shader_program_disable_attribute(shader, "vTangent");
    shader_program_disable_attribute(shader, "vBinormal");
    shader_program_disable_attribute(shader, "vTexcoord");
    shader_program_disable_attribute_matrix(shader, "vWorld");
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

  }
  
  shader_program_disable(shader);

}

static void render_vegetation(deferred_renderer* dr, instance_object* io) {
  
  if (config_int(asset_hndl_ptr(&dr->options), "vegetation") == 0) return;  
  
  if (sphere_outside_box(io->bound, dr->camera_frustum)) { return; }  
  
  if (config_bool(asset_hndl_ptr(&dr->options), "render_colmeshes")) {
    if (!file_isloaded(io->collision_body.path)) {
      file_load(io->collision_body.path);
    }
    //render_cmesh(dr, asset_hndl_ptr(io->collision_body), world);
  }
  
  renderable* r = asset_hndl_ptr(&io->renderable);
  
  if(r->is_rigged) { error("Static object is rigged!"); }
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_vegetation));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "view", dr->camera_view);
  shader_program_set_mat4(shader, "proj", dr->camera_proj);
  shader_program_set_float(shader, "clip_near", dr->camera_near);
  shader_program_set_float(shader, "clip_far",  dr->camera_far);
  shader_program_set_float(shader, "time", dr->time);
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    
    material_entry* me = material_get_entry(asset_hndl_ptr(&r->material), i);
    
    if (config_bool(asset_hndl_ptr(&dr->options), "render_white")) {
      shader_program_set_texture(shader, "diffuse_map", 0, dr->tex_grey);
    } else {
      shader_program_set_texture(shader, "diffuse_map", 0, material_entry_item(me, "diffuse_map").as_asset);
    }
    shader_program_set_texture(shader, "bump_map", 1, material_entry_item(me, "bump_map").as_asset);
    shader_program_set_texture(shader, "spec_map", 2, material_entry_item(me, "spec_map").as_asset);
    shader_program_set_float(shader, "glossiness", material_entry_item(me, "glossiness").as_float);
    shader_program_set_float(shader, "bumpiness", material_entry_item(me, "bumpiness").as_float);
    shader_program_set_float(shader, "specular_level", material_entry_item(me, "specular_level").as_float);
    shader_program_set_float(shader, "alpha_test", material_entry_item(me, "alpha_test").as_float);
    shader_program_set_int(shader, "material", material_entry_item(me, "material").as_int);
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    
    shader_program_enable_attribute(shader, "vPosition",  3, 18, (void*)0);
    shader_program_enable_attribute(shader, "vNormal",    3, 18, (void*)(sizeof(float) * 3));
    shader_program_enable_attribute(shader, "vTangent",   3, 18, (void*)(sizeof(float) * 6));
    shader_program_enable_attribute(shader, "vBinormal",  3, 18, (void*)(sizeof(float) * 9));
    shader_program_enable_attribute(shader, "vTexcoord",  2, 18, (void*)(sizeof(float) * 12));
    shader_program_enable_attribute(shader, "vColor",     4, 18, (void*)(sizeof(float) * 14));
    
    glBindBuffer(GL_ARRAY_BUFFER, io->world_buffer);
    
    shader_program_enable_attribute_instance_matrix(shader, "vWorld", (void*)0);
    
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      glDrawElementsInstanced(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0, io->num_instances);
    
    shader_program_disable_attribute(shader, "vPosition");
    shader_program_disable_attribute(shader, "vNormal");
    shader_program_disable_attribute(shader, "vTangent");
    shader_program_disable_attribute(shader, "vBinormal");
    shader_program_disable_attribute(shader, "vTexcoord");
    shader_program_disable_attribute(shader, "vColor");
    shader_program_disable_attribute_matrix(shader, "vWorld");
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

  }
  
  shader_program_disable(shader);

}

static void render_animated(deferred_renderer* dr, animated_object* ao) {
    
  renderable* r = asset_hndl_ptr(&ao->renderable);
  skeleton* skel = asset_hndl_ptr(&ao->skeleton);
  
  if (!r->is_rigged) { error("Animated object is not rigged!"); }
  if (skel->joint_count > MAX_BONES) { error("animated object skeleton has too many bones (over %i)", MAX_BONES); }
  if (ao->pose == NULL) { return; }
  
  for(int i = 0; i < skel->joint_count; i++) {
    mat4 base = skel->rest_pose->transforms_inv[i];
    mat4 ani = ao->pose->transforms[i];
    bone_matrices[i] = mat4_mul_mat4(ani, base);
  }
  
  mat4 world = mat4_world(ao->position, ao->scale, ao->rotation);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_animated));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", world);
  shader_program_set_mat4(shader, "view", dr->camera_view);
  shader_program_set_mat4(shader, "proj", dr->camera_proj);
  shader_program_set_float(shader, "clip_near", dr->camera_near);
  shader_program_set_float(shader, "clip_far",  dr->camera_far);
  shader_program_set_mat4_array(shader, "world_bones", bone_matrices, skel->joint_count);
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    
    //if (sphere_outside_box(sphere_transform(s->bound, world), dr->camera_frustum)) { continue; }
    
    material_entry* me = material_get_entry(asset_hndl_ptr(&r->material), i);
    
    if (config_bool(asset_hndl_ptr(&dr->options), "render_white")) {
      shader_program_set_texture(shader, "diffuse_map", 0, dr->tex_grey);
    } else {
      shader_program_set_texture(shader, "diffuse_map", 0, material_entry_item(me, "diffuse_map").as_asset);
    }
    shader_program_set_texture(shader, "bump_map", 1, material_entry_item(me, "bump_map").as_asset);
    shader_program_set_texture(shader, "spec_map", 2, material_entry_item(me, "spec_map").as_asset);
    shader_program_set_float(shader, "glossiness", material_entry_item(me, "glossiness").as_float);
    shader_program_set_float(shader, "bumpiness", material_entry_item(me, "bumpiness").as_float);
    shader_program_set_float(shader, "specular_level", material_entry_item(me, "specular_level").as_float);
    shader_program_set_int(shader, "material", material_entry_item(me, "material").as_int);
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
    
    shader_program_enable_attribute(shader, "vPosition",        3, 24, (void*)0);
    shader_program_enable_attribute(shader, "vNormal",          3, 24, (void*)(sizeof(float) * 3));
    shader_program_enable_attribute(shader, "vTangent",         3, 24, (void*)(sizeof(float) * 6));
    shader_program_enable_attribute(shader, "vBinormal",        3, 24, (void*)(sizeof(float) * 9));
    shader_program_enable_attribute(shader, "vTexcoord",        2, 24, (void*)(sizeof(float) * 12));
    shader_program_enable_attribute(shader, "vBone_indicies",   3, 24, (void*)(sizeof(float) * 18));
    shader_program_enable_attribute(shader, "vBone_weights",    3, 24, (void*)(sizeof(float) * 21));
    
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
    
    shader_program_disable_attribute(shader, "vPosition");
    shader_program_disable_attribute(shader, "vNormal");
    shader_program_disable_attribute(shader, "vTangent");
    shader_program_disable_attribute(shader, "vBinormal");
    shader_program_disable_attribute(shader, "vTexcoord"); 
    shader_program_disable_attribute(shader, "vBone_indicies");
    shader_program_disable_attribute(shader, "vBone_weights");
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

  }
  
  shader_program_disable(shader);
  
}

static void render_landscape_blobtree(deferred_renderer* dr, shader* shader, landscape_blobtree* lbt, terrain* terr) {
  
  if (sphere_outside_box(lbt->bound, dr->camera_frustum)) { return; }
  
  if (!lbt->is_leaf) {
    render_landscape_blobtree(dr, shader, lbt->child0, terr);
    render_landscape_blobtree(dr, shader, lbt->child1, terr);
    render_landscape_blobtree(dr, shader, lbt->child2, terr);
    render_landscape_blobtree(dr, shader, lbt->child3, terr);
    return;
  }
  
  //render_ellipsoid(dr, ellipsoid_of_sphere(lbt->bound));
  
  int quality = config_int(asset_hndl_ptr(&dr->options), "terrain");
  
  float dist = vec3_dist_sqrd(dr->camera->position, lbt->bound.center) / (100 * NUM_TERRAIN_BUFFERS);
  int buff_index = clamp((int)dist, (3-quality), NUM_TERRAIN_BUFFERS-1);
  
  terrain_chunk* tc = terr->chunks[lbt->chunk_index];
  
  glBindBuffer(GL_ARRAY_BUFFER, tc->vertex_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tc->index_buffers[buff_index]);
  
  shader_program_enable_attribute(shader, "vPosition",  3, 12, (void*)0);
  shader_program_enable_attribute(shader, "vNormal",    3, 12, (void*)(sizeof(float) * 3));
  shader_program_enable_attribute(shader, "vTangent",   3, 12, (void*)(sizeof(float) * 6));
  shader_program_enable_attribute(shader, "vBinormal",  3, 12, (void*)(sizeof(float) * 9));
  
    glDrawElements(GL_TRIANGLES, tc->num_indicies[buff_index], GL_UNSIGNED_INT, (void*)0);
  
  shader_program_disable_attribute(shader, "vPosition");
  shader_program_disable_attribute(shader, "vNormal");
  shader_program_disable_attribute(shader, "vTangent");
  shader_program_disable_attribute(shader, "vBinormal");
 
}

static void render_landscape(deferred_renderer* dr, landscape* l) {
  
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  
  terrain* terr = asset_hndl_ptr(&l->heightmap);
  
  if (config_bool(asset_hndl_ptr(&dr->options), "render_colmeshes")) {
    for(int i = 0; i < terr->num_chunks; i++) {
      terrain_chunk* tc = terr->chunks[i];
      render_cmesh(dr, tc->colmesh, landscape_world(l));  
    }
  }
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_terrain));
  
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", landscape_world(l));
  shader_program_set_mat4(shader, "view", dr->camera_view);
  shader_program_set_mat4(shader, "proj", dr->camera_proj);
  shader_program_set_float(shader, "clip_near", dr->camera_near);
  shader_program_set_float(shader, "clip_far",  dr->camera_far);
  shader_program_set_float(shader, "size_x", l->size_x);
  shader_program_set_float(shader, "size_y", l->size_y);
  
  if (config_bool(asset_hndl_ptr(&dr->options), "render_white")) {
    shader_program_set_texture(shader, "ground0", 0, dr->tex_grey);
    shader_program_set_texture(shader, "ground1", 1, dr->tex_grey);
    shader_program_set_texture(shader, "ground2", 2, dr->tex_grey);
    shader_program_set_texture(shader, "ground3", 3, dr->tex_grey);
  } else {
    shader_program_set_texture(shader, "ground0", 0, l->ground0);
    shader_program_set_texture(shader, "ground1", 1, l->ground1);
    shader_program_set_texture(shader, "ground2", 2, l->ground2);
    shader_program_set_texture(shader, "ground3", 3, l->ground3);
  }
  
  shader_program_set_texture(shader, "ground0_nm", 4, l->ground0_nm);
  shader_program_set_texture(shader, "ground1_nm", 5, l->ground1_nm);
  shader_program_set_texture(shader, "ground2_nm", 6, l->ground2_nm);
  shader_program_set_texture(shader, "ground3_nm", 7, l->ground3_nm);
  shader_program_set_texture(shader, "attribmap", 8, l->attribmap);
  
  if (unlikely(l->blobtree == NULL)) { error("Landscape blobtree must be generated!"); }
  
  render_landscape_blobtree(dr, shader, l->blobtree, terr);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  shader_program_disable(shader);
  
  //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
}


void render_light(deferred_renderer* dr, light* l) {
  
  vec3 light_pos = l->position;
  light_pos = mat4_mul_vec3(camera_view_matrix(dr->camera), light_pos);
  light_pos = mat4_mul_vec3(camera_proj_matrix(dr->camera), light_pos);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_ui));
  shader_program_enable(shader);
  shader_program_set_texture(shader, "diffuse", 0, asset_hndl_new_load(P("$CORANGE/ui/lightbulb.dds")));
  shader_program_set_float(shader, "alpha_test", 0.5);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", mat4_id());
  shader_program_set_mat4(shader, "proj", mat4_orthographic(0, graphics_viewport_width(), graphics_viewport_height(), 0, -1, 1));
  
  float top = ((-light_pos.y + 1) / 2) * graphics_viewport_height() - 8;
  float bot = ((-light_pos.y + 1) / 2) * graphics_viewport_height() + 8;
  float left = ((light_pos.x + 1) / 2) * graphics_viewport_width() - 8;
  float right = ((light_pos.x + 1) / 2) * graphics_viewport_width() + 8;
  
  // TODO: Implement
  
  /*
  float light_position[] = {};
  float light_texcoord[] = {};
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(left, top, -light_pos.z);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(left,  bot, -light_pos.z);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(right,  bot, -light_pos.z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(right, top, -light_pos.z);
	glEnd();
  */
  
  shader_program_disable(shader);

}

void render_axis(deferred_renderer* dr, mat4 world) {

  vec4 x_pos = mat4_mul_vec4(world, vec4_new(2,0,0,1));
  vec4 y_pos = mat4_mul_vec4(world, vec4_new(0,2,0,1));
  vec4 z_pos = mat4_mul_vec4(world, vec4_new(0,0,2,1));
  vec4 base_pos = mat4_mul_vec4(world, vec4_new(0,0,0,1));
  
  x_pos = vec4_div(x_pos, x_pos.w);
  y_pos = vec4_div(y_pos, y_pos.w);
  z_pos = vec4_div(z_pos, z_pos.w);
  base_pos = vec4_div(base_pos, base_pos.w);

  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_ui));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", dr->camera_view);
  shader_program_set_mat4(shader, "proj", dr->camera_proj);
  shader_program_set_float(shader, "alpha_test", 0.0);
  shader_program_set_texture(shader, "diffuse", 0, dr->tex_white);
  
  float axis_line_color[] = {1,0,0, 1,0,0, 0,1,0, 0,1,0, 0,0,1, 0,0,1};
  float axis_line_position[] = {
    x_pos.x, x_pos.y, x_pos.z, base_pos.x, base_pos.y, base_pos.z,
    y_pos.x, y_pos.y, y_pos.z, base_pos.x, base_pos.y, base_pos.z,
    z_pos.x, z_pos.y, z_pos.z, base_pos.x, base_pos.y, base_pos.z
  };
  
  float axis_point_color[] = {1,1,1, 1,0,0, 0,1,0, 0,0,1};
  float axis_point_position[] = {
    base_pos.x, base_pos.y, base_pos.z,
    x_pos.x, x_pos.y, x_pos.z, 
    y_pos.x, y_pos.y, y_pos.z,
    z_pos.x, z_pos.y, z_pos.z
  };
  
  glDisable(GL_DEPTH_TEST);
  glLineWidth(5.0);
  glPointSize(10.0);
  
    shader_program_enable_attribute(shader, "vPosition",  3, 3, axis_line_position);
    shader_program_enable_attribute(shader, "vColor",     3, 3, axis_line_color);
    
    glDrawArrays(GL_LINES, 0, 6);
    
    shader_program_enable_attribute(shader, "vPosition",  3, 3, axis_point_position);
    shader_program_enable_attribute(shader, "vColor",     3, 3, axis_point_color);
    
    glDrawArrays(GL_POINTS, 0, 4);
  
  shader_program_disable_attribute(shader, "vPosition");
  shader_program_disable_attribute(shader, "vColor");
  
  glPointSize(1.0);
  glLineWidth(1.0);  
  glEnable(GL_DEPTH_TEST);

  shader_program_disable(shader);
  
}

void render_frustum(deferred_renderer* dr, frustum f) {

  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_ui));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", dr->camera_view);
  shader_program_set_mat4(shader, "proj", dr->camera_proj);
  shader_program_set_float(shader, "alpha_test", 0.0);
  shader_program_set_texture(shader, "diffuse", 0, dr->tex_white);
  
  float frustum_colors[] = {
    1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0,
    1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0,
    1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0};
  vec3 frustum_positions[] = {
    f.ntr, f.ntl, f.ntl, f.nbl, f.nbl, f.nbr, f.nbr, f.ntr,
    f.ftr, f.ftl, f.ftl, f.fbl, f.fbl, f.fbr, f.fbr, f.ftr,
    f.nbr, f.fbr, f.nbl, f.fbl, f.ntr, f.ftr, f.ntl, f.ftl};
  
  glDisable(GL_DEPTH_TEST);
  glLineWidth(5.0);
  
    shader_program_enable_attribute(shader, "vPosition",  3, 3, frustum_positions);
    shader_program_enable_attribute(shader, "vColor",     3, 3, frustum_colors);
    
    glDrawArrays(GL_LINES, 0, 24);
  
  shader_program_disable_attribute(shader, "vPosition");
  shader_program_disable_attribute(shader, "vColor");
  
  glLineWidth(1.0);  
  glEnable(GL_DEPTH_TEST);
  
  shader_program_disable(shader);
  
}

void render_plane(deferred_renderer* dr, plane p) {

  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_ui));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", dr->camera_view);
  shader_program_set_mat4(shader, "proj", dr->camera_proj);
  shader_program_set_float(shader, "alpha_test", 0.0);
  shader_program_set_texture(shader, "diffuse", 0, dr->tex_white);
  
  vec3 left = vec3_cross(vec3_normalize(p.direction), vec3_up());
  vec3 right = vec3_cross(vec3_normalize(p.direction), left);
  vec3 color = vec3_div(vec3_add(p.direction, vec3_new(1.0, 1.0, 1.0)), 2.0);
  
  if (vec3_dot(vec3_cross(left, right), p.direction) < 0) {
    vec3 tmp = left;
    left = right;
    right = tmp;
  }  
  
  vec3 plane_colors[] = {color, color, color};
  vec3 plane_positions[] = {p.position, vec3_add(p.position, left), vec3_add(p.position, right)};
  
  glDisable(GL_DEPTH_TEST);
  
    shader_program_enable_attribute(shader, "vPosition",  3, 3, plane_positions);
    shader_program_enable_attribute(shader, "vColor",     3, 3, plane_colors);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);
  
  shader_program_disable_attribute(shader, "vPosition");
  shader_program_disable_attribute(shader, "vColor");
  
  glEnable(GL_DEPTH_TEST);
  
  shader_program_disable(shader);
  
}

void render_line(deferred_renderer* dr, vec3 line_start, vec3 line_end, vec3 line_color, float line_thickness) {

  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_ui));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", dr->camera_view);
  shader_program_set_mat4(shader, "proj", dr->camera_proj);
  shader_program_set_float(shader, "alpha_test", 0.0);
  shader_program_set_texture(shader, "diffuse", 0, dr->tex_white);
  
  vec3 line_positions[] = { line_start, line_end };
  vec3 line_colors[] = { line_color, line_color };
  
  glDisable(GL_DEPTH_TEST);
  glLineWidth(line_thickness);
  
  shader_program_enable_attribute(shader, "vPosition",  3, 3, line_positions);
  shader_program_enable_attribute(shader, "vColor",     3, 3, line_colors);
  
  glDrawArrays(GL_LINES, 0, 2);
  
  shader_program_disable_attribute(shader, "vPosition");
  shader_program_disable_attribute(shader, "vColor");
  
  glLineWidth(1.0);  
  glEnable(GL_DEPTH_TEST);

  shader_program_disable(shader);
  
}

void render_point(deferred_renderer* dr, vec3 position, vec3 color, float size) {

  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_ui));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", dr->camera_view);
  shader_program_set_mat4(shader, "proj", dr->camera_proj);
  shader_program_set_float(shader, "alpha_test", 0.0);
  shader_program_set_texture(shader, "diffuse", 0, dr->tex_white);
  
  glDisable(GL_DEPTH_TEST);
  glPointSize(size);
  
  shader_program_enable_attribute(shader, "vPosition",  3, 3, &position);
  shader_program_enable_attribute(shader, "vColor",     3, 3, &color);
  
  glDrawArrays(GL_POINTS, 0, 1);
  
  shader_program_disable_attribute(shader, "vPosition");
  shader_program_disable_attribute(shader, "vColor");
  
  glPointSize(1.0);  
  glEnable(GL_DEPTH_TEST);

  shader_program_disable(shader);
  
}

void render_ellipsoid(deferred_renderer* dr, ellipsoid e) {
  
  static_object so;
  so.position = e.center;
  so.rotation = quat_id();
  so.scale = vec3_new(e.radiuses.x, e.radiuses.y, e.radiuses.z);
  so.renderable = dr->mesh_sphere;
  so.collision_body = asset_hndl_new(P("$CORANGE/objects/sphere.col"));
  
  render_static(dr, &so);
  
}

void render_projectile(deferred_renderer* dr, projectile* p) {

  static_object so;
  so.position = p->position;
  so.rotation = p->rotation;
  so.scale = vec3_new(1, 1, 1);
  so.renderable = p->mesh;
  so.collision_body = asset_hndl_null();
  
  render_static(dr, &so);

}

void render_paint_circle(deferred_renderer* dr, mat4 axis, float radius) {
  
  render_axis(dr, axis);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_ui));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", dr->camera_view);
  shader_program_set_mat4(shader, "proj", dr->camera_proj);
  shader_program_set_texture(shader, "diffuse", 0, dr->tex_white);
  shader_program_set_float(shader, "alpha_test", 0.0);
  
  glDisable(GL_DEPTH_TEST);
  glLineWidth(1.0);
  
  int circle_items = 0;
  float circle_positions[512];
  float circle_colors[512];
  
  for(float i = 0; i < M_PI * 2; i += 0.1) {
    
    vec3 point0 = vec3_mul(vec3_new(sin(i+0.0), 0, cos(i+0.0)), radius);
    vec3 point1 = vec3_mul(vec3_new(sin(i+0.1), 0, cos(i+0.1)), radius);
    
    point0 = mat4_mul_vec3(axis, point0);
    point1 = mat4_mul_vec3(axis, point1);
  
    circle_positions[circle_items] = point0.x; circle_colors[circle_items] = 0.25; circle_items++;
    circle_positions[circle_items] = point0.y; circle_colors[circle_items] = 0.25; circle_items++;
    circle_positions[circle_items] = point0.z; circle_colors[circle_items] = 0.25; circle_items++;
    circle_positions[circle_items] = point1.x; circle_colors[circle_items] = 0.25; circle_items++;
    circle_positions[circle_items] = point1.y; circle_colors[circle_items] = 0.25; circle_items++;
    circle_positions[circle_items] = point1.z; circle_colors[circle_items] = 0.25; circle_items++;
    
  }
  
  shader_program_enable_attribute(shader, "vPosition",  3, 3, circle_positions);
  shader_program_enable_attribute(shader, "vColor",     3, 3, circle_colors);
  
    glDrawArrays(GL_LINES, 0, circle_items/3);
  
  shader_program_disable_attribute(shader, "vPosition");
  shader_program_disable_attribute(shader, "vColor");
  
  glLineWidth(1.0);
  glEnable(GL_DEPTH_TEST);
  
  shader_program_disable(shader);
  
}

static camera* compare_cam = NULL;

static float render_object_cost(const render_object* ro) {
  switch (ro->type) {
    case RO_TYPE_STATIC:     return vec3_dist_sqrd(compare_cam->position, ro->static_object->position);
    case RO_TYPE_INSTANCE:   return vec3_dist_sqrd(compare_cam->position, ro->instance_object->instances[0].position);
    case RO_TYPE_ANIMATED:   return vec3_dist_sqrd(compare_cam->position, ro->animated_object->position);
    case RO_TYPE_PARTICLES:  return vec3_dist_sqrd(compare_cam->position, ro->particles->position);
    case RO_TYPE_LANDSCAPE:  return vec3_dist_sqrd(compare_cam->position, vec3_zero());
    case RO_TYPE_PROJECTILE: return vec3_dist_sqrd(compare_cam->position, ro->projectile->position);
    default: return FLT_MAX;
  }
}

static int render_object_sort(const void* ro0, const void* ro1) {
  
  float cost0 = render_object_cost(ro0);
  float cost1 = render_object_cost(ro1);
  
  if (cost0 == cost1) {
    return  0;
  } else if (cost0 > cost1) {
    return  1;
  } else {
    return -1;
  }
  
}

static void render_gbuffer(deferred_renderer* dr) {
  
  SDL_GL_CheckError();
  
  /*
  compare_cam = dr->camera;
  qsort(dr->render_objects, 
        dr->render_objects_num, 
        sizeof(render_object), 
        render_object_sort);
  */
  
  dr->camera_view = camera_view_matrix(dr->camera);
  dr->camera_proj = camera_proj_matrix(dr->camera);
  dr->camera_near = dr->camera->near_clip;
  dr->camera_far  = dr->camera->far_clip;
  dr->camera_frustum = box_invert_depth(frustum_box(frustum_new_camera(dr->camera_view, dr->camera_proj)));
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  int gwidth  = width  * option_graphics_int(asset_hndl_ptr(&dr->options), "msaa", 4, 2, 1);
  int gheight = height * option_graphics_int(asset_hndl_ptr(&dr->options), "msaa", 4, 2, 1);
  
  glBindFramebuffer(GL_FRAMEBUFFER, dr->gfbo);
  glDrawBuffers(2, (GLenum[]){ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 });
  glViewport( 0, 0, gwidth, gheight);
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  
  for ( int j = 0; j < dr->render_objects_num; j++) {
    
    // HACK ALERT
    bool veg_found = false;
    if (dr->render_objects[j].type == RO_TYPE_INSTANCE) {
      renderable* r = asset_hndl_ptr(&dr->render_objects[j].instance_object->renderable);
      material* m = asset_hndl_ptr(&r->material);
      
      for (int i = 0; i < m->num_entries; i++) {
        if (material_entry_item(m->entries[i], "material").as_int == 6) {
          render_vegetation(dr, dr->render_objects[j].instance_object);
          veg_found = true;
          break;
        }
      }
      
    }
    
    bool skin_found = false;
    if (dr->render_objects[j].type == RO_TYPE_INSTANCE) {
      renderable* r = asset_hndl_ptr(&dr->render_objects[j].instance_object->renderable);
      material* m = asset_hndl_ptr(&r->material);
      
      for (int i = 0; i < m->num_entries; i++) {
        if (material_entry_item(m->entries[i], "material").as_int == 7) {
          render_skin(dr, dr->render_objects[j].instance_object);
          skin_found = true;
          break;
        }
      }
      
    }
    
    if (skin_found) continue;
    if (veg_found) continue;
    
    if (dr->render_objects[j].type == RO_TYPE_STATIC)     { render_static(dr, dr->render_objects[j].static_object); continue; }
    if (dr->render_objects[j].type == RO_TYPE_INSTANCE)   { render_instance(dr, dr->render_objects[j].instance_object); continue; }
    if (dr->render_objects[j].type == RO_TYPE_ANIMATED)   { render_animated(dr, dr->render_objects[j].animated_object); continue; }
    if (dr->render_objects[j].type == RO_TYPE_LANDSCAPE)  { render_landscape(dr, dr->render_objects[j].landscape); }
    if (dr->render_objects[j].type == RO_TYPE_LIGHT)      { render_light(dr, dr->render_objects[j].light); continue; }
    if (dr->render_objects[j].type == RO_TYPE_AXIS)       { render_axis(dr, dr->render_objects[j].axis); continue; }
    if (dr->render_objects[j].type == RO_TYPE_SPHERE)     { render_ellipsoid(dr, ellipsoid_of_sphere(dr->render_objects[j].sphere)); continue; }
    if (dr->render_objects[j].type == RO_TYPE_ELLIPSOID)  { render_ellipsoid(dr, dr->render_objects[j].ellipsoid); continue; }
    if (dr->render_objects[j].type == RO_TYPE_FRUSTUM)    { render_frustum(dr, dr->render_objects[j].frustum); continue; }
    if (dr->render_objects[j].type == RO_TYPE_PLANE)      { render_plane(dr, dr->render_objects[j].plane); continue; }
    if (dr->render_objects[j].type == RO_TYPE_PROJECTILE) { render_projectile(dr, dr->render_objects[j].projectile); continue; }
    
    if (dr->render_objects[j].type == RO_TYPE_LINE) {
      render_line(dr, 
        dr->render_objects[j].line_start, 
        dr->render_objects[j].line_end, 
        dr->render_objects[j].line_color,
        dr->render_objects[j].line_thickness);
      continue;
    }
    
    if (dr->render_objects[j].type == RO_TYPE_POINT) {
      render_point(dr, 
        dr->render_objects[j].point_pos,
        dr->render_objects[j].point_color,
        dr->render_objects[j].point_size);
      continue;
    }
    
    if (dr->render_objects[j].type == RO_TYPE_CMESH) {
      render_cmesh(dr, 
        dr->render_objects[j].colmesh, 
        dr->render_objects[j].colworld);
      continue;
    }
        
    if (dr->render_objects[j].type == RO_TYPE_PAINT) {
      render_paint_circle(dr, 
        dr->render_objects[j].paint_axis, 
        dr->render_objects[j].paint_radius);
      continue;
    }
  }
  
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  SDL_GL_CheckError();
  
}

static void render_ssao(deferred_renderer* dr) {
  
  if (config_int(asset_hndl_ptr(&dr->options), "ssao") == 0) return;
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  int ssaowidth  = width  * option_graphics_int(asset_hndl_ptr(&dr->options), "ssao", 1, 0.5, 0.25);
  int ssaoheight = height * option_graphics_int(asset_hndl_ptr(&dr->options), "ssao", 1, 0.5, 0.25);
  
  glBindFramebuffer(GL_FRAMEBUFFER, dr->ssao_fbo);
  glViewport(0, 0, ssaowidth, ssaoheight);
  glClearColor(1, 1, 1, 1);
  glClear( GL_COLOR_BUFFER_BIT );
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_ssao));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", mat4_id());
  shader_program_set_mat4(shader, "proj", mat4_orthographic(-1, 1, -1, 1, -1, 1));
  
  shader_program_set_texture(shader, "random_texture", 0, dr->tex_random);
  shader_program_set_texture_id(shader, "depth_texture", 1, dr->gdepth_texture);
  shader_program_set_texture_id(shader, "normals_texture", 2, dr->gnormals_texture);  

  shader_program_set_int(shader, "width", graphics_viewport_width());
  shader_program_set_int(shader, "height", graphics_viewport_height());
  shader_program_set_float(shader, "clip_far", dr->camera->far_clip);
  shader_program_set_float(shader, "clip_near", dr->camera->near_clip);
  
  shader_program_enable_attribute(shader, "vPosition",  3, 3, quad_position);
  shader_program_enable_attribute(shader, "vTexcoord",  2, 2, quad_texcoord);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
  
  shader_program_disable_attribute(shader, "vPosition");
  shader_program_disable_attribute(shader, "vTexcoord");
  shader_program_disable(shader);
  
  glViewport(0, 0, graphics_viewport_width(), graphics_viewport_height());
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void render_skies(deferred_renderer* dr) {
  
  if (!dr->skydome_enabled) { return; }
    
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  int hdrwidth  = width  * option_graphics_int(asset_hndl_ptr(&dr->options), "msaa", 4, 2, 1);
  int hdrheight = height * option_graphics_int(asset_hndl_ptr(&dr->options), "msaa", 4, 2, 1);
  
  glBindFramebuffer(GL_FRAMEBUFFER, dr->hdr_fbo);
  glViewport(0, 0, hdrwidth, hdrheight);
  glDepthMask(GL_FALSE);
  
  {
  
    shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_skydome));
    shader_program_enable(shader);
    shader_program_set_mat4(shader, "world", mat4_world(dr->camera->position, vec3_new(200, 200, 200), quat_id()));
    shader_program_set_mat4(shader, "view", dr->camera_view);
    shader_program_set_mat4(shader, "proj", dr->camera_proj);
    shader_program_set_vec3(shader, "light_direction", dr->sky->sun_direction);
    //shader_program_set_vec3(shader, "camera_position", dr->camera->position);
    
    renderable* skybox_r = asset_hndl_ptr(&dr->mesh_skydome);
    renderable_surface* s = skybox_r->surfaces[0];
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
    
    shader_program_enable_attribute(shader, "vPosition",  3, 18, (void*)0);
    
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
    
    shader_program_disable_attribute(shader, "vPosition");
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    shader_program_disable(shader);
  
  }
  
  {
  
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
  
    shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_sun));
    shader_program_enable(shader);
    shader_program_set_mat4(shader, "world", mat4_world(dr->camera->position, vec3_one(), mat4_to_quat(dr->sky->world_sun)));
    shader_program_set_mat4(shader, "view", dr->camera_view);
    shader_program_set_mat4(shader, "proj", dr->camera_proj);
    shader_program_set_float(shader, "sun_brightness", 1.5);
    shader_program_set_vec4(shader, "sun_color", vec4_one());
    shader_program_set_texture(shader, "sun_texture", 0, dr->sky->sun_tex);
    
    renderable* sun_r = asset_hndl_ptr(&dr->sky->sun_sprite);
    renderable_surface* s = sun_r->surfaces[0];
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
    
    shader_program_enable_attribute(shader, "vPosition",  3, 18, (void*)0);
    shader_program_enable_attribute(shader, "vTexcoord",  2, 18, (void*)(sizeof(float) * 12));
    
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
    
    shader_program_disable_attribute(shader, "vPosition");
    shader_program_disable_attribute(shader, "vTexcoord");
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    shader_program_disable(shader);
    
    glDisable(GL_BLEND);
  
  }
  
  {
  
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
    shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_clouds));
    shader_program_enable(shader);
    shader_program_set_mat4(shader, "world", mat4_world(dr->camera->position, vec3_new(10, 10, 10), quat_id()));
    shader_program_set_mat4(shader, "view", dr->camera_view);
    shader_program_set_mat4(shader, "proj", dr->camera_proj);
    shader_program_set_float(shader, "time", dr->time);
    shader_program_set_float(shader, "wind", vec3_length(dr->sky->wind));
    shader_program_set_vec3(shader, "cloud_color", vec3_one());
    shader_program_set_vec3(shader, "cloud_light", vec3_mul(vec3_one(), 2));
    
    for (int i = 0; i < 14; i++) {
    
      shader_program_set_texture(shader, "cloud_texture", 0, dr->sky->cloud_tex[i]);
      shader_program_set_float(shader, "opacity", dr->sky->cloud_opacity[i]);
      
      renderable* sun_r = asset_hndl_ptr(&dr->sky->cloud_mesh[i]);
      renderable_surface* s = sun_r->surfaces[0];
      
      glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      
      shader_program_enable_attribute(shader, "vPosition",  3, 18, (void*)0);
      shader_program_enable_attribute(shader, "vTexcoord",  2, 18, (void*)(sizeof(float) * 12));
      
        glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
      
      shader_program_disable_attribute(shader, "vPosition");
      shader_program_disable_attribute(shader, "vTexcoord");
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    }
    
    shader_program_disable(shader);
    
    glDisable(GL_BLEND);
  
  }
  
  glDepthMask(GL_TRUE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);  
  
}

static void render_sea(deferred_renderer* dr) {
  
  if (!dr->sea_enabled) { return; } 
  
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  int hdrwidth  = width  * option_graphics_int(asset_hndl_ptr(&dr->options), "msaa", 4, 2, 1);
  int hdrheight = height * option_graphics_int(asset_hndl_ptr(&dr->options), "msaa", 4, 2, 1);
  
  glBindFramebuffer(GL_FRAMEBUFFER, dr->hdr_fbo);
  glViewport(0, 0, hdrwidth, hdrheight);
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  vec3 position = vec3_new(dr->camera->position.x, 0, dr->camera->position.z);
  vec3 scale = vec3_new(1, 1, 1);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_sea));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_world(position, scale, quat_id()));
  shader_program_set_mat4(shader, "view", dr->camera_view);
  shader_program_set_mat4(shader, "proj", dr->camera_proj);
  shader_program_set_float(shader, "clip_near", dr->camera_near);
  shader_program_set_float(shader, "clip_far",  dr->camera_far);
  shader_program_set_float(shader, "time", dr->time);
  
  shader_program_set_float(shader, "light_power", dr->sky->sun_power);
  shader_program_set_vec3(shader, "light_direction", dr->sky->sun_direction);
  shader_program_set_vec3(shader, "light_diffuse", dr->sky->sun_diffuse);
  shader_program_set_vec3(shader, "light_ambient", dr->sky->sun_ambient);
  shader_program_set_vec3(shader, "light_specular", dr->sky->sun_specular);
  shader_program_set_vec3(shader, "camera_position", dr->camera->position);
  
  shader_program_set_texture_id(shader, "depth", 0, dr->gdepth_texture);
  shader_program_set_texture(shader, "bump0", 1, dr->tex_sea_bump0);
  shader_program_set_texture(shader, "bump1", 2, dr->tex_sea_bump1);
  shader_program_set_texture(shader, "bump2", 3, dr->tex_sea_bump2);
  shader_program_set_texture(shader, "bump3", 4, dr->tex_sea_bump3);
  shader_program_set_texture(shader, "cube_sea", 5, dr->tex_cube_sea);
  
  renderable* sea_r = asset_hndl_ptr(&dr->mesh_sea);
  renderable_surface* s = sea_r->surfaces[0];
  
  glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
  
  shader_program_enable_attribute(shader, "vPosition",  3, 18, (void*)0);
  
    glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
  
  shader_program_disable_attribute(shader, "vPosition");
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  shader_program_disable(shader);
  
  glDisable(GL_BLEND);
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);  
  
  //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
}

static void render_compose_low(deferred_renderer* dr) {
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  int hdrwidth  = width  * option_graphics_int(asset_hndl_ptr(&dr->options), "msaa", 4, 2, 1);
  int hdrheight = height * option_graphics_int(asset_hndl_ptr(&dr->options), "msaa", 4, 2, 1);
  
  glBindFramebuffer(GL_FRAMEBUFFER, dr->hdr_fbo);
  glViewport(0, 0, hdrwidth, hdrheight);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_compose));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", mat4_id());
  shader_program_set_mat4(shader, "proj", mat4_orthographic(-1, 1, -1, 1, -1, 1));
  shader_program_set_mat4(shader, "inv_view", dr->camera_inv_view);
  shader_program_set_mat4(shader, "inv_proj", dr->camera_inv_proj);
  shader_program_set_float(shader, "clip_near", dr->camera_near);
  shader_program_set_float(shader, "clip_far", dr->camera_far);
  
  shader_program_set_texture_id(shader, "diffuse_texture", 2, dr->gdiffuse_texture);
  shader_program_set_texture_id(shader, "depth_texture", 3, dr->gdepth_texture);
  shader_program_set_texture_id(shader, "normals_texture", 4, dr->gnormals_texture);
  
  shader_program_set_vec3(shader, "camera_position", dr->camera->position);

  shader_program_set_float(shader, "light_power", dr->sky->sun_power + dr->sky->sky_power);
  shader_program_set_vec3(shader, "light_position", vec3_neg(dr->sky->sun_direction));
  shader_program_set_vec3(shader, "light_target", vec3_zero());
  shader_program_set_vec3(shader, "light_diffuse", dr->sky->sun_diffuse);
  shader_program_set_vec3(shader, "light_ambient", vec3_add(dr->sky->sky_ambient, dr->sky->sun_ambient));
  shader_program_set_vec3(shader, "light_specular", dr->sky->sun_specular);
  
  shader_program_enable_attribute(shader, "vPosition",  3, 3, quad_position);
  shader_program_enable_attribute(shader, "vTexcoord",  2, 2, quad_texcoord);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
  
  shader_program_disable_attribute(shader, "vPosition");
  shader_program_disable_attribute(shader, "vTexcoord");
  shader_program_disable(shader);
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, width, height);
  
}

static void render_compose_high(deferred_renderer* dr) {
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  int hdrwidth  = width  * option_graphics_int(asset_hndl_ptr(&dr->options), "msaa", 4, 2, 1);
  int hdrheight = height * option_graphics_int(asset_hndl_ptr(&dr->options), "msaa", 4, 2, 1);
  
  glBindFramebuffer(GL_FRAMEBUFFER, dr->hdr_fbo);
  glViewport(0, 0, hdrwidth, hdrheight);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_compose));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", mat4_id());
  shader_program_set_mat4(shader, "proj", mat4_orthographic(-1, 1, -1, 1, -1, 1));
  shader_program_set_mat4(shader, "inv_view", dr->camera_inv_view);
  shader_program_set_mat4(shader, "inv_proj", dr->camera_inv_proj);
  shader_program_set_float(shader, "clip_near", dr->camera_near);
  shader_program_set_float(shader, "clip_far", dr->camera_far);
  
  shader_program_set_texture(shader, "env_texture", 0, dr->tex_cube_field);  
  shader_program_set_texture(shader, "random_texture", 1, dr->tex_random);
  shader_program_set_texture_id(shader, "diffuse_texture", 2, dr->gdiffuse_texture);
  shader_program_set_texture_id(shader, "depth_texture", 3, dr->gdepth_texture);
  shader_program_set_texture_id(shader, "normals_texture", 4, dr->gnormals_texture);
  shader_program_set_texture_id(shader, "ssao_texture", 5, dr->ssao_texture);
  shader_program_set_texture_id(shader, "shadows_texture0", 6, dr->shadows_texture[0]);
  shader_program_set_texture_id(shader, "shadows_texture1", 7, dr->shadows_texture[1]);
  shader_program_set_texture_id(shader, "shadows_texture2", 8, dr->shadows_texture[2]);
  shader_program_set_texture(shader, "skin_lookup", 9, dr->tex_skin_lookup);
  
  shader_program_set_vec3(shader, "camera_position", dr->camera->position);
  //shader_program_set_float_array(shader, "light_clip_near", dr->shadow_near, 3);
  //shader_program_set_float_array(shader, "light_clip_far", dr->shadow_far, 3);
  shader_program_set_mat4_array(shader, "light_view", dr->shadow_view, 3);
  shader_program_set_mat4_array(shader, "light_proj", dr->shadow_proj, 3);
  shader_program_set_float_array(shader, "light_start", dr->shadows_start, 3);
  
  float light_power[DEFERRED_MAX_LIGHTS];
  float light_falloff[DEFERRED_MAX_LIGHTS];
  vec3 light_position[DEFERRED_MAX_LIGHTS];
  vec3 light_target[DEFERRED_MAX_LIGHTS];
  vec3 light_diffuse[DEFERRED_MAX_LIGHTS];
  vec3 light_ambient[DEFERRED_MAX_LIGHTS];
  vec3 light_specular[DEFERRED_MAX_LIGHTS];
  
  if (dr->sky->is_day) {
    light_power[0]    = dr->sky->sun_power;
    light_falloff[0]  = 0;
    light_position[0] = vec3_neg(dr->sky->sun_direction);
    light_target[0]   = vec3_zero();
    light_diffuse[0]  = dr->sky->sun_diffuse;
    light_ambient[0]  = dr->sky->sun_ambient;
    light_specular[0] = dr->sky->sun_specular;
  } else {
    light_power[0]    = dr->sky->moon_power;
    light_falloff[0]  = 0;
    light_position[0] = vec3_neg(dr->sky->moon_direction);
    light_target[0]   = vec3_zero();
    light_diffuse[0]  = dr->sky->moon_diffuse;
    light_ambient[0]  = dr->sky->moon_ambient;
    light_specular[0] = dr->sky->moon_specular;
  }
  
  light_power[1]    = dr->sky->sky_power;
  light_falloff[1]  = 0;
  light_position[1] = vec3_neg(dr->sky->sky_direction);
  light_target[1]   = vec3_zero();
  light_diffuse[1]  = dr->sky->sky_diffuse;
  light_ambient[1]  = dr->sky->sky_ambient;
  light_specular[1] = dr->sky->sky_specular;
  
  light_power[2]    = dr->sky->ground_power;
  light_falloff[2]  = 0;
  light_position[2] = vec3_neg(dr->sky->ground_direction);
  light_target[2]   = vec3_zero();
  light_diffuse[2]  = dr->sky->ground_diffuse;
  light_ambient[2]  = dr->sky->ground_ambient;
  light_specular[2] = dr->sky->ground_specular;
  
  const int ln = 3;
  for(int i = 0; i < dr->dyn_lights_num; i++) {
    light_power[i+ln]    = dr->dyn_light[i]->power;
    light_falloff[i+ln]  = dr->dyn_light[i]->falloff;
    light_position[i+ln] = dr->dyn_light[i]->position;
    light_target[i+ln]   = dr->dyn_light[i]->target;
    light_diffuse[i+ln]  = dr->dyn_light[i]->diffuse_color;
    light_ambient[i+ln]  = dr->dyn_light[i]->ambient_color;
    light_specular[i+ln] = dr->dyn_light[i]->specular_color;
  }
  
  shader_program_set_int(shader, "lights_num", dr->dyn_lights_num + ln);
  shader_program_set_float_array(shader, "light_power", light_power, dr->dyn_lights_num + ln);
  shader_program_set_float_array(shader, "light_falloff", light_falloff, dr->dyn_lights_num + ln);
  shader_program_set_vec3_array(shader, "light_position", light_position, dr->dyn_lights_num + ln);
  shader_program_set_vec3_array(shader, "light_target", light_target, dr->dyn_lights_num + ln);
  shader_program_set_vec3_array(shader, "light_diffuse", light_diffuse, dr->dyn_lights_num + ln);
  shader_program_set_vec3_array(shader, "light_ambient", light_ambient, dr->dyn_lights_num + ln);
  shader_program_set_vec3_array(shader, "light_specular", light_specular, dr->dyn_lights_num + ln);
  
  shader_program_enable_attribute(shader, "vPosition",  3, 3, quad_position);
  shader_program_enable_attribute(shader, "vTexcoord",  2, 2, quad_texcoord);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
  
  shader_program_disable_attribute(shader, "vPosition");
  shader_program_disable_attribute(shader, "vTexcoord");
  shader_program_disable(shader);
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, width, height);
  
}

static void render_compose(deferred_renderer* dr) {

  int lighting = config_int(asset_hndl_ptr(&dr->options), "lighting");
  if (lighting == 0) {
    render_compose_low(dr);
  } else {
    render_compose_high(dr);
  }

  SDL_GL_CheckError();

}

static void render_particles(deferred_renderer* dr) {
  
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  int hdrwidth  = width  * option_graphics_int(asset_hndl_ptr(&dr->options), "msaa", 4, 2, 1);
  int hdrheight = height * option_graphics_int(asset_hndl_ptr(&dr->options), "msaa", 4, 2, 1);
  
  glBindFramebuffer(GL_FRAMEBUFFER, dr->hdr_fbo);
  glViewport(0, 0, hdrwidth, hdrheight);
  
  glEnable(GL_BLEND);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_particles));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "view", dr->camera_view);
  shader_program_set_mat4(shader, "proj", dr->camera_proj);
  shader_program_set_float(shader, "clip_near", dr->camera_near);
  shader_program_set_float(shader, "clip_far",  dr->camera_far);

  shader_program_set_float(shader, "light_power", dr->sky->sun_power);
  shader_program_set_vec3(shader, "light_direction", dr->sky->sun_direction);
  shader_program_set_vec3(shader, "light_diffuse", dr->sky->sun_diffuse);
  shader_program_set_vec3(shader, "light_ambient", dr->sky->sun_ambient);
  
  for (int i = 0; i < dr->render_objects_num; i++) {
    
    if (dr->render_objects[i].type != RO_TYPE_PARTICLES) { continue; }
    
    particles* p = dr->render_objects[i].particles;
    effect* e = asset_hndl_ptr(&p->effect);
    
    glBlendFunc(e->blend_src, e->blend_dst);
    
    shader_program_set_float(shader, "particle_depth", e->depth);
    shader_program_set_float(shader, "particle_thickness", e->thickness);
    shader_program_set_float(shader, "particle_bumpiness", e->bumpiness);
    shader_program_set_float(shader, "particle_scattering", e->scattering);
    
    shader_program_set_mat4(shader, "world", mat4_world(p->position, p->scale, p->rotation));
    shader_program_set_texture(shader, "particle_diffuse", 0, e->texture);
    shader_program_set_texture(shader, "particle_normals", 1, e->texture_nm);
    shader_program_set_texture_id(shader, "depth", 2, dr->gdepth_texture);
    
    glBindBuffer(GL_ARRAY_BUFFER, p->vertex_buff);
    
    shader_program_enable_attribute(shader, "vPosition",  3, 18, (void*)(0));
    shader_program_enable_attribute(shader, "vNormal",    3, 18, (void*)(sizeof(float) * 3));
    shader_program_enable_attribute(shader, "vTangent",   3, 18, (void*)(sizeof(float) * 6));
    shader_program_enable_attribute(shader, "vBinormal",  3, 18, (void*)(sizeof(float) * 9));
    shader_program_enable_attribute(shader, "vTexcoord",  2, 18, (void*)(sizeof(float) * 12));
    shader_program_enable_attribute(shader, "vColor",     4, 18, (void*)(sizeof(float) * 14));
    
      glDrawArrays(GL_TRIANGLES, 0, p->count * 6);
    
    shader_program_disable_attribute(shader, "vPosition");
    shader_program_disable_attribute(shader, "vNormal");
    shader_program_disable_attribute(shader, "vTangent");
    shader_program_disable_attribute(shader, "vBinormal");
    shader_program_disable_attribute(shader, "vTexcoord");
    shader_program_disable_attribute(shader, "vColor");
  
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  }

  glDisable(GL_BLEND);

  shader_program_disable(shader);
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, width, height);
  
  //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
  SDL_GL_CheckError();
  
}

static void render_tonemap(deferred_renderer* dr) {
  
  glBindFramebuffer(GL_FRAMEBUFFER, dr->ldr_back_fbo);
  glViewport(0, 0, graphics_viewport_width(), graphics_viewport_height());

  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_tonemap));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", mat4_id());
  shader_program_set_mat4(shader, "proj", mat4_orthographic(-1, 1, -1, 1, -1, 1));
  shader_program_set_float(shader, "exposure", dr->exposure);
  shader_program_set_texture_id(shader, "hdr_texture", 0, dr->hdr_texture);
  
  shader_program_enable_attribute(shader, "vPosition",  3, 3, quad_position);
  shader_program_enable_attribute(shader, "vTexcoord",  2, 2, quad_texcoord);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
  
  shader_program_disable_attribute(shader, "vPosition");
  shader_program_disable_attribute(shader, "vTexcoord");
  shader_program_disable(shader);
  
  /* Generate Mipmaps, adjust exposure */
  
  /*
  unsigned char color[4] = {0,0,0,0};
  int level = -1; int width = 0; int height = 0;
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, dr->ldr_back_texture);
  glGenerateMipmap(GL_TEXTURE_2D);
  
  do {
    level++;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &height);
    if (level > 50) { error("Unable to find lowest mip level. Perhaps mipmaps were not generated"); }
  } while ((width > 1) || (height > 1));
  
  glGetTexImage(GL_TEXTURE_2D, level, GL_RGBA, GL_UNSIGNED_BYTE, color);
  
  float average = (float)(color[0] + color[1] + color[2]) / (3.0 * 255.0);
  */
  
  //EXPOSURE += (EXPOSURE_TARGET - average) * EXPOSURE_SPEED;
  dr->exposure = 3.0;
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, graphics_viewport_width(), graphics_viewport_height());
  
  SDL_GL_CheckError();
  
}

static void render_post0(deferred_renderer* dr) {
  
  glBindFramebuffer(GL_FRAMEBUFFER, dr->ldr_front_fbo);
  glViewport(0, 0, graphics_viewport_width(), graphics_viewport_height());
  glClearColor(1.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_post0));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", mat4_id());
  shader_program_set_mat4(shader, "proj", mat4_orthographic(-1, 1, -1, 1, -1, 1));
  shader_program_set_texture_id(shader, "ldr_texture", 0, dr->ldr_back_texture);
  //shader_program_set_texture(shader, "random_texture", 1, dr->tex_random);
  
  /*
  glActiveTexture(GL_TEXTURE0 + 2 );
  glBindTexture(GL_TEXTURE_2D, dr->gdepth_texture);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 3);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
  shader_program_set_int(shader, "depth_texture", 2);
  */
  
  //shader_program_set_int(shader, "width", graphics_viewport_width());
  //shader_program_set_int(shader, "height", graphics_viewport_height());
  
  shader_program_enable_attribute(shader, "vPosition",  3, 3, quad_position);
  shader_program_enable_attribute(shader, "vTexcoord",  2, 2, quad_texcoord);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
  
  shader_program_disable_attribute(shader, "vPosition");
  shader_program_disable_attribute(shader, "vTexcoord");
  
  shader_program_disable(shader);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  SDL_GL_CheckError();
  
}

static void render_post1(deferred_renderer* dr) {
    
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, graphics_viewport_width(), graphics_viewport_height());
  glClearDepth(1.0);
  glClearColor(1.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(&dr->mat_post1));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", mat4_id());
  shader_program_set_mat4(shader, "proj", mat4_orthographic(-1, 1, -1, 1, -1, 1));
  
  shader_program_set_texture_id(shader, "ldr_texture", 0, dr->ldr_front_texture);
  shader_program_set_texture(shader, "random_perlin", 1, dr->tex_random_perlin);
  shader_program_set_texture(shader, "vignetting_texture", 2, dr->tex_vignetting);
  shader_program_set_texture(shader, "lut", 3, dr->tex_color_correction);
  
  shader_program_set_float(shader, "glitch", dr->glitch);
  shader_program_set_float(shader, "time", dr->time);
  shader_program_set_int(shader, "width", graphics_viewport_width());
  shader_program_set_int(shader, "height", graphics_viewport_height());
  shader_program_set_int(shader, "fxaa_quality", config_int(asset_hndl_ptr(&dr->options), "fxaa"));
  
  shader_program_enable_attribute(shader, "vPosition",  3, 3, quad_position);
  shader_program_enable_attribute(shader, "vTexcoord",  2, 2, quad_texcoord);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
  
  shader_program_disable_attribute(shader, "vPosition");
  shader_program_disable_attribute(shader, "vTexcoord");
  shader_program_disable(shader);
  
  SDL_GL_CheckError();
  SDL_GL_CheckFrameBuffer();
  
}



void deferred_renderer_render(deferred_renderer* dr) {
  
  dr->time += frame_time();
  
  //timer t = timer_start(0, "Rendering Start");
  
  render_shadows(dr);   //glFlush(); t = timer_split(t, "Shadow");
  render_clear(dr);     //glFlush(); t = timer_split(t, "Clear");
  render_gbuffer(dr);   //glFlush(); t = timer_split(t, "GBuffer");
  render_ssao(dr);      //glFlush(); t = timer_split(t, "SSAO");
  render_skies(dr);     //glFlush(); t = timer_split(t, "Skies");
  render_compose(dr);   //glFlush(); t = timer_split(t, "Compose");
  render_sea(dr);       //glFlush(); t = timer_split(t, "Sea");
  render_particles(dr); //glFlush(); t = timer_split(t, "Particles");
  render_tonemap(dr);   //glFlush(); t = timer_split(t, "Tonemap");
  render_post0(dr);     //glFlush(); t = timer_split(t, "Post0");
  render_post1(dr);     //glFlush(); t = timer_split(t, "Post1");
  
  //timer_stop(t, "Rendering End");
  
  dr->render_objects_num = 0;
  dr->dyn_lights_num = 0;
  
}
 
