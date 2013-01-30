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

render_object render_object_paint(vec3 paint_pos, vec3 paint_norm, float paint_radius) {
  render_object ro;
  ro.type = RO_TYPE_PAINT;
  ro.paint_pos = paint_pos;
  ro.paint_norm = paint_norm;
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

render_object render_object_projectile(projectile* p) {
  render_object ro;
  ro.type = RO_TYPE_PROJECTILE;
  ro.projectile = p;
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

deferred_renderer* deferred_renderer_new() {
  
  deferred_renderer* dr = malloc(sizeof(deferred_renderer));
  
  /* Camera */
  dr->camera = NULL;

  dr->dyn_lights_num = 0;
  for(int i = 0; i < DEFERRED_MAX_DYN_LIGHTS; i++) {
    dr->dyn_light[i] = NULL;
  }
  
  config* options = asset_get_load(P("./assets/options.cfg"));
  
  /* Materials */
  folder_load(P("$CORANGE/shaders/deferred/"));
  
  dr->mat_static     = asset_hndl_new(P("$CORANGE/shaders/deferred/static.mat"));
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
  dr->mat_depth_ani  = asset_hndl_new(P("$CORANGE/shaders/deferred/depth_animated.mat"));
  dr->mat_depth_veg  = asset_hndl_new(P("$CORANGE/shaders/deferred/depth_vegetation.mat"));
  dr->mat_sun        = asset_hndl_new(P("$CORANGE/shaders/deferred/sun.mat"));
  dr->mat_clouds     = asset_hndl_new(P("$CORANGE/shaders/deferred/clouds.mat"));
  dr->mat_particles  = asset_hndl_new(P("$CORANGE/shaders/deferred/particles.mat"));
  
  dr->mat_compose = option_graphics_asset(options, "graphics_lighting",
    asset_hndl_new(P("$CORANGE/shaders/deferred/compose.mat")),
    asset_hndl_new(P("$CORANGE/shaders/deferred/compose.mat")),
    asset_hndl_new(P("$CORANGE/shaders/deferred/compose_low.mat")));
  
  /* Meshes */
  dr->mesh_skydome  = asset_hndl_new_load(P("$CORANGE/resources/skydome.obj"));
  dr->mesh_sphere   = asset_hndl_new_load(P("$CORANGE/resources/sphere.obj"));
  
  /* Textures */
  dr->tex_color_correction  = asset_hndl_new_load(P("$CORANGE/resources/identity.lut"));
  dr->tex_random            = asset_hndl_new_load(P("$CORANGE/resources/random.dds"));
  dr->tex_random_perlin     = asset_hndl_new_load(P("$CORANGE/resources/random_perlin.dds"));
  dr->tex_environment       = asset_hndl_new_load(P("$CORANGE/resources/envmap.dds"));
  dr->tex_vignetting        = asset_hndl_new_load(P("$CORANGE/resources/vignetting.dds"));
  
  /* Buffers */
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  int gwidth  = width  * option_graphics_int(options, "graphics_msaa", 4, 2, 1);
  int gheight = height * option_graphics_int(options, "graphics_msaa", 4, 2, 1);
  
  glGenFramebuffers(1, &dr->gfbo);
  glBindFramebuffer(GL_FRAMEBUFFER, dr->gfbo);
  
  glGenRenderbuffers(1, &dr->gdiffuse_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, dr->gdiffuse_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, gwidth, gheight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, dr->gdiffuse_buffer);   
  
  glGenRenderbuffers(1, &dr->gpositions_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, dr->gpositions_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F, gwidth, gheight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, dr->gpositions_buffer);  
  
  glGenRenderbuffers(1, &dr->gnormals_buffer);  
  glBindRenderbuffer(GL_RENDERBUFFER, dr->gnormals_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F, gwidth, gheight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, dr->gnormals_buffer);  
  
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
  
  glGenTextures(1, &dr->gpositions_texture);
  glBindTexture(GL_TEXTURE_2D, dr->gpositions_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, gwidth, gheight, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, dr->gpositions_texture, 0);
  
  glGenTextures(1, &dr->gnormals_texture);
  glBindTexture(GL_TEXTURE_2D, dr->gnormals_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gwidth, gheight, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, dr->gnormals_texture, 0);
  
  glGenTextures(1, &dr->gdepth_texture);
  glBindTexture(GL_TEXTURE_2D, dr->gdepth_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, gwidth, gheight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dr->gdepth_texture, 0);
  
  SDL_GL_CheckFrameBuffer();
  
  /* SSAO Buffer */
  
  int ssaowidth  = width  / option_graphics_int(options, "graphics_ssao", 1, 2, 4);
  int ssaoheight = height / option_graphics_int(options, "graphics_ssao", 1, 2, 4);
  
  glGenFramebuffers(1, &dr->ssao_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, dr->ssao_fbo);
  
  glGenRenderbuffers(1, &dr->ssao_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, dr->ssao_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, ssaowidth, ssaoheight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, dr->ssao_buffer);   
  
  glGenTextures(1, &dr->ssao_texture);
  glBindTexture(GL_TEXTURE_2D, dr->ssao_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ssaowidth, ssaoheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dr->ssao_texture, 0);
  
  SDL_GL_CheckFrameBuffer();
  
  /* HDR Buffer */
  
  int hdrwidth  = width  * option_graphics_int(options, "graphics_msaa", 4, 2, 1);
  int hdrheight = height * option_graphics_int(options, "graphics_msaa", 4, 2, 1);
  
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
  
  SDL_GL_CheckFrameBuffer();
  
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
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dr->ldr_front_texture, 0);
  
  SDL_GL_CheckFrameBuffer();
  
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
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dr->ldr_back_texture, 0);
  
  /* Shadow Buffers */
  
  int shadow_width  = option_graphics_int(options, "graphics_shadows", 4096, 2048, 1024);
  int shadow_height = option_graphics_int(options, "graphics_shadows", 4096, 2048, 1024);
  
  dr->shadows_start[0] = 0.00; dr->shadows_end[0] = 0.05;
  dr->shadows_start[1] = 0.05; dr->shadows_end[1] = 0.20;
  dr->shadows_start[2] = 0.20; dr->shadows_end[2] = 0.50;
  
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
    
    SDL_GL_CheckError();
    SDL_GL_CheckFrameBuffer();

  }
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  /* Variables */
  dr->seed = 0;
  dr->glitch = 0.0;
  dr->time = 0.0;
  dr->time_of_day = TIME_MORNING;
  dr->exposure = 0.0;
  dr->exposure_speed = 1.0;
  dr->exposure_target = 0.4;
  dr->skydome_enabled = true;
  
  /* Objects */
  dr->render_objects_num = 0;
  dr->render_objects = NULL;
  
  return dr;
  
}

void deferred_renderer_delete(deferred_renderer* dr) {
  
  glDeleteFramebuffers(1, &dr->gfbo);
  
  glDeleteRenderbuffers(1, &dr->gdiffuse_buffer);
  glDeleteRenderbuffers(1, &dr->gpositions_buffer);
  glDeleteRenderbuffers(1, &dr->gnormals_buffer);
  glDeleteRenderbuffers(1, &dr->gdepth_buffer);
  
  glDeleteTextures(1, &dr->gdiffuse_texture);
  glDeleteTextures(1, &dr->gpositions_texture);
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
  
  SDL_GL_CheckError();
  
  folder_unload(P("$CORANGE/shaders/deferred/"));
  
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

static void shadow_mapper_transforms(deferred_renderer* dr, int i, mat4* view, mat4* proj, float* nearclip, float* farclip) {
    
  mat4 inv_view = mat4_inverse(camera_view_matrix(dr->camera));
  mat4 inv_proj = mat4_inverse(camera_proj_matrix(dr->camera));
  
  frustum f = frustum_new_clipbox();
  f = frustum_transform(f, inv_proj);
  f = frustum_transform(f, inv_view);
  f = frustum_slice(f, dr->shadows_start[i], dr->shadows_end[i]);
  
  vec3 center = frustum_center(f);
  if (sky_isday(dr->time_of_day)) {
    *view = mat4_view_look_at(center, vec3_add(center, sky_sun_direction(dr->time_of_day)), vec3_up());
  } else {
    *view = mat4_view_look_at(center, vec3_add(center, sky_moon_direction(dr->time_of_day)), vec3_up());
  }
  
  f = frustum_translate(f, vec3_neg(center));
  f = frustum_transform(f, *view);
  f = frustum_translate(f, vec3_neg(center));
  
  vec3 maximums = frustum_maximums(f);
  vec3 minimums = frustum_minimums(f);
  
  float rangex = max(maximums.x, -minimums.x);
  float rangey = max(maximums.y, -minimums.y);
  float rangez = max(maximums.z, -minimums.z);
  
  *nearclip = -rangey;
  *farclip = rangey;
  *proj = mat4_orthographic(-rangex, rangex, -rangez, rangez, -rangey, rangey);
  
}

static void render_shadows_vegetation(deferred_renderer* dr, int i, static_object* s) {

  mat4 world = mat4_world( s->position, s->scale, s->rotation );
  mat4 view, proj;
  float clip_near, clip_far;
  shadow_mapper_transforms(dr, i, &view, &proj, &clip_near, &clip_far);
  
  mat4 inv_view = mat4_inverse(view);
  mat4 inv_proj = mat4_inverse(proj);
  frustum frus = frustum_new_clipbox();
  frus = frustum_transform(frus, inv_proj);
  frus = frustum_transform(frus, inv_view);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_depth_veg));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", world);
  shader_program_set_mat4(shader, "view",  view);
  shader_program_set_mat4(shader, "proj",  proj);
  shader_program_set_float(shader, "clip_near", clip_near);
  shader_program_set_float(shader, "clip_far", clip_far);
  shader_program_set_float(shader, "time", dr->time);
  
  renderable* r = asset_hndl_ptr(s->renderable);

  if(r->is_rigged) { error("Static Object is rigged!"); }
  
  for(int j = 0; j < r->num_surfaces; j++) {
    
    renderable_surface* s = r->surfaces[j];
    
    if (sphere_outside_frustum(sphere_transform(s->bound, world), frus)) { continue; }
    
    material_entry* me = material_get_entry(asset_hndl_ptr(r->material), j);
    bool use_alpha =
      material_entry_has_item(me, "alpha_test") &&
      material_entry_has_item(me, "diffuse_map");
    
    if (use_alpha) {
      shader_program_set_float(shader, "alpha_test", material_entry_item(me, "alpha_test").as_float);
      shader_program_enable_texture(shader, "diffuse_map", 0, material_entry_item(me, "diffuse_map").as_asset);
    } else {
      shader_program_set_float(shader, "alpha_test", 0.0);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
    
    shader_program_enable_attribute(shader, "vPosition", 3, 18, (void*)0);
    shader_program_enable_attribute(shader, "vTexcoord", 2, 18, (void*)(sizeof(float) * 12));
    shader_program_enable_attribute(shader, "vColor",    4, 18, (void*)(sizeof(float) * 14));
    
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
    
    shader_program_disable_attribute(shader, "vPosition");
    shader_program_disable_attribute(shader, "vTexcoord");
    shader_program_disable_attribute(shader, "vColor");
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    if (use_alpha) {
      shader_program_disable_texture(shader, 0);
    }
    
  }
  
  shader_program_disable(shader);

}

static void render_shadows_static(deferred_renderer* dr, int i, static_object* s) {
  
  mat4 world = mat4_world( s->position, s->scale, s->rotation );
  mat4 view, proj;
  float clip_near, clip_far;
  shadow_mapper_transforms(dr, i, &view, &proj, &clip_near, &clip_far);
  
  mat4 inv_view = mat4_inverse(view);
  mat4 inv_proj = mat4_inverse(proj);
  frustum frus = frustum_new_clipbox();
  frus = frustum_transform(frus, inv_proj);
  frus = frustum_transform(frus, inv_view);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_depth));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", world);
  shader_program_set_mat4(shader, "view",  view);
  shader_program_set_mat4(shader, "proj",  proj);
  shader_program_set_float(shader, "clip_near", clip_near);
  shader_program_set_float(shader, "clip_far", clip_far);
  
  renderable* r = asset_hndl_ptr(s->renderable);

  if(r->is_rigged) { error("Static Object is rigged!"); }
  
  for(int j = 0; j < r->num_surfaces; j++) {
    
    renderable_surface* s = r->surfaces[j];
    
    if (sphere_outside_frustum(sphere_transform(s->bound, world), frus)) { continue; }
    
    material_entry* me = material_get_entry(asset_hndl_ptr(r->material), j);
    bool use_alpha =
      material_entry_has_item(me, "alpha_test") &&
      material_entry_has_item(me, "diffuse_map");
    
    if (use_alpha) {
      shader_program_set_float(shader, "alpha_test", material_entry_item(me, "alpha_test").as_float);
      shader_program_enable_texture(shader, "diffuse_map", 0, material_entry_item(me, "diffuse_map").as_asset);
    } else {
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
    
    if (use_alpha) {
      shader_program_disable_texture(shader, 0);
    }
    
  }
  
  shader_program_disable(shader);
  
}

#define MAX_BONES 32
static mat4 bone_matrices[MAX_BONES];

static void render_shadows_animated(deferred_renderer* dr, int i, animated_object* ao) {
  
  mat4 world = mat4_world( ao->position, ao->scale, ao->rotation );
  mat4 view, proj;
  float clip_near, clip_far;
  shadow_mapper_transforms(dr, i, &view, &proj, &clip_near, &clip_far);

  mat4 inv_view = mat4_inverse(view);
  mat4 inv_proj = mat4_inverse(proj);
  frustum frus = frustum_new_clipbox();
  frus = frustum_transform(frus, inv_proj);
  frus = frustum_transform(frus, inv_view);
  
  skeleton* skel = asset_hndl_ptr(ao->skeleton);

  if (skel->num_bones > MAX_BONES) { error("animated object skeleton has too many bones (over %i)", MAX_BONES); }
  if (ao->pose == NULL) { return; }
  
  for(int j = 0; j < skel->num_bones; j++) {
    mat4 base = bone_transform(skel->bones[j]);
    mat4 ani = bone_transform(ao->pose->bones[j]);
    bone_matrices[j] = mat4_mul_mat4(ani, mat4_inverse(base));
  }
  
  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_depth_ani));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", world);
  shader_program_set_mat4(shader, "view",  view);
  shader_program_set_mat4(shader, "proj",  proj);
  shader_program_set_mat4_array(shader, "world_bones", bone_matrices, skel->num_bones);
  shader_program_set_float(shader, "clip_near", clip_near);
  shader_program_set_float(shader, "clip_far", clip_far);
  
  renderable* r = asset_hndl_ptr(ao->renderable);
  
  if(!r->is_rigged) { error("animated object is not rigged"); }
  
  for(int j = 0; j < r->num_surfaces; j++) {
    renderable_surface* s = r->surfaces[j];
    
    if (sphere_outside_frustum(sphere_transform(s->bound, world), frus)) { continue; }
    
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

static void render_shadows_landscape(deferred_renderer* dr, int i, landscape* l) {

  mat4 view, proj;
  float clip_near, clip_far;
  shadow_mapper_transforms(dr, i, &view, &proj, &clip_near, &clip_far);

  terrain* terr = asset_hndl_ptr(l->heightmap);
  vec3 scale = vec3_new(-(1.0 / terr->width) * l->size_x, 0.25, -(1.0 / terr->height) * l->size_y);
  vec3 translation = vec3_new(l->size_x / 2, 0, l->size_y / 2);
  mat4 rotation = mat4_id();
  
  // This assumes that X or Z scale of a chunk will never exceed the height.
  float bound_scale_val = max(scale.x, scale.z);
  vec3 bound_scale = vec3_new(bound_scale_val, bound_scale_val, bound_scale_val);
  
  mat4 inv_view = mat4_inverse(view);
  mat4 inv_proj = mat4_inverse(proj);
  frustum frus = frustum_new_clipbox();
  frus = frustum_transform(frus, inv_proj);
  frus = frustum_transform(frus, inv_view);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_depth));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_world( translation, scale, rotation ));
  shader_program_set_mat4(shader, "view",  view);
  shader_program_set_mat4(shader, "proj",  proj);
  shader_program_set_float(shader, "clip_near", clip_near);
  shader_program_set_float(shader, "clip_far", clip_far);
  shader_program_set_float(shader, "alpha_test",  0.0);
  
  for(int j = 0; j < terr->num_chunks; j++) {
    terrain_chunk* tc = terr->chunks[j];
    
    float chunkx = (1.0 / (terr->num_rows-1)) *  l->size_x;
    float chunky = (1.0 / (terr->num_cols-1)) *  l->size_y;
    float posx = ((float)(j % terr->num_cols)) * chunkx - l->size_x / 2;
    float posy = ((float)(j / terr->num_cols)) * chunky - l->size_y / 2;
    
    sphere bound = sphere_transform(tc->bound, mat4_world(translation, bound_scale, mat4_id()));
    if (sphere_outside_frustum(bound, frus)) { continue; }
    
    float dist = vec2_dist_sqrd(
      vec2_new(dr->camera->position.x, dr->camera->position.z), 
      vec2_new(-posx, -posy)) / (100 * NUM_TERRAIN_BUFFERS);
    
    int buff_index = clamp((int)dist, 0, NUM_TERRAIN_BUFFERS-1);
    
    glBindBuffer(GL_ARRAY_BUFFER, tc->vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tc->index_buffers[buff_index]);
  
    shader_program_enable_attribute(shader, "vPosition", 3, 12, (void*)0);
      
      glDrawElements(GL_TRIANGLES, tc->num_indicies[buff_index], GL_UNSIGNED_INT, (void*)0);
    
    shader_program_disable_attribute(shader, "vPosition");
    
  }
  
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
  
  for (int i = 0; i < 3; i++) {
  
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
      if (dr->render_objects[j].type == RO_TYPE_STATIC) {
        renderable* r = asset_hndl_ptr(dr->render_objects[j].static_object->renderable);
        material* m = asset_hndl_ptr(r->material);        
        
        for (int k = 0; k < m->num_entries; k++) {
          if (material_entry_item(m->entries[k], "material").as_int == 6) {
            render_shadows_vegetation(dr, i, dr->render_objects[j].static_object);
            veg_found = true;
            break;
          }
        }
        
      }
      
      if (veg_found) continue;

    
      if (dr->render_objects[j].type == RO_TYPE_STATIC) { render_shadows_static(dr, i, dr->render_objects[j].static_object); }
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

static void render_clear(deferred_renderer* dr) {

  glBindFramebuffer(GL_FRAMEBUFFER, dr->gfbo);
  glDrawBuffers(3, (GLenum[]){ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 });
  glViewport( 0, 0, graphics_viewport_width(), graphics_viewport_height());
  glClearColor(0.2, 0.2, 0.2, 1.0f);
  glClearDepth(1.0f);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_clear));
  shader_program_enable(shader);
  shader_program_set_vec4(shader, "start", vec4_new(0.5, 0.5, 0.5, 1.0));
  shader_program_set_vec4(shader, "end", vec4_new(0.0, 0.0, 0.0, 1.0));
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", mat4_id());
  shader_program_set_mat4(shader, "proj", mat4_orthographic(-1, 1, -1, 1, -1, 1));
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
  
  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_static));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", world);
  shader_program_set_mat4(shader, "view", camera_view_matrix(dr->camera));
  shader_program_set_mat4(shader, "proj", camera_proj_matrix(dr->camera));
  shader_program_set_float(shader, "near", dr->camera->near_clip);
  shader_program_set_float(shader, "far", dr->camera->far_clip);
  
  material_entry* me = material_get_entry(asset_get_load(P("$CORANGE/shaders/basic.mat")), 0);
  
  shader_program_enable_texture(shader, "diffuse_map", 0, material_entry_item(me, "diffuse_map").as_asset);
  shader_program_enable_texture(shader, "bump_map", 1, material_entry_item(me, "bump_map").as_asset);
  shader_program_enable_texture(shader, "spec_map", 2, material_entry_item(me, "spec_map").as_asset);
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
  
  free(positions);
  free(normals);
  
  shader_program_disable_texture(shader, 2);
  shader_program_disable_texture(shader, 1);
  shader_program_disable_texture(shader, 0);
  
  shader_program_disable(shader);
    
}

static void render_static(deferred_renderer* dr, static_object* so) {
  
  mat4 world = mat4_world( so->position, so->scale, so->rotation );
  mat4 inv_view = mat4_inverse(camera_view_matrix(dr->camera));
  mat4 inv_proj = mat4_inverse(camera_proj_matrix(dr->camera));
  frustum frus = frustum_new_clipbox();
  frus = frustum_transform(frus, inv_proj);
  frus = frustum_transform(frus, inv_view);
  
  config* options = asset_get_load(P("./assets/options.cfg"));
  if (config_bool(options, "render_colmeshes")) {
    if (!file_isloaded(so->collision_body.path)) {
      file_load(so->collision_body.path);
    }
    render_cmesh(dr, asset_hndl_ptr(so->collision_body), world);
  }
  
  renderable* r = asset_hndl_ptr(so->renderable);
  
  if(r->is_rigged) { error("Static object is rigged!"); }
  
  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_static));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", world);
  shader_program_set_mat4(shader, "view", camera_view_matrix(dr->camera));
  shader_program_set_mat4(shader, "proj", camera_proj_matrix(dr->camera));
  shader_program_set_float(shader, "near", dr->camera->near_clip);
  shader_program_set_float(shader, "far", dr->camera->far_clip);
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    
    if (sphere_outside_frustum(sphere_transform(s->bound, world), frus)) { continue; }
    
    int mentry_id = min(i, ((material*)asset_hndl_ptr(r->material))->num_entries-1);
    material_entry* me = material_get_entry(asset_hndl_ptr(r->material), mentry_id);
    
    if (config_bool(options, "render_white")) {
      shader_program_enable_texture(shader, "diffuse_map", 0, asset_hndl_new_load(P("$CORANGE/resources/white.dds")));
    } else {
      shader_program_enable_texture(shader, "diffuse_map", 0, material_entry_item(me, "diffuse_map").as_asset);
    }
    shader_program_enable_texture(shader, "bump_map", 1, material_entry_item(me, "bump_map").as_asset);
    shader_program_enable_texture(shader, "spec_map", 2, material_entry_item(me, "spec_map").as_asset);
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
    
    shader_program_disable_texture(shader, 2);
    shader_program_disable_texture(shader, 1);
    shader_program_disable_texture(shader, 0);

  }
  
  shader_program_disable(shader);

}

static void render_vegetation(deferred_renderer* dr, static_object* so) {
  
  mat4 world = mat4_world( so->position, so->scale, so->rotation );
  mat4 inv_view = mat4_inverse(camera_view_matrix(dr->camera));
  mat4 inv_proj = mat4_inverse(camera_proj_matrix(dr->camera));
  frustum frus = frustum_new_clipbox();
  frus = frustum_transform(frus, inv_proj);
  frus = frustum_transform(frus, inv_view);
  
  config* options = asset_get_load(P("./assets/options.cfg"));
  if (config_bool(options, "render_colmeshes")) {
    if (!file_isloaded(so->collision_body.path)) {
      file_load(so->collision_body.path);
    }
    render_cmesh(dr, asset_hndl_ptr(so->collision_body), world);
  }
  
  renderable* r = asset_hndl_ptr(so->renderable);
  
  if(r->is_rigged) { error("Static object is rigged!"); }
  
  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_vegetation));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", world);
  shader_program_set_mat4(shader, "view", camera_view_matrix(dr->camera));
  shader_program_set_mat4(shader, "proj", camera_proj_matrix(dr->camera));
  shader_program_set_float(shader, "near", dr->camera->near_clip);
  shader_program_set_float(shader, "far", dr->camera->far_clip);
  shader_program_set_float(shader, "time", dr->time);
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    
    if (sphere_outside_frustum(sphere_transform(s->bound, world), frus)) { continue; }
    
    int mentry_id = min(i, ((material*)asset_hndl_ptr(r->material))->num_entries-1);
    material_entry* me = material_get_entry(asset_hndl_ptr(r->material), mentry_id);
    
    if (config_bool(options, "render_white")) {
      shader_program_enable_texture(shader, "diffuse_map", 0, asset_hndl_new_load(P("$CORANGE/resources/white.dds")));
    } else {
      shader_program_enable_texture(shader, "diffuse_map", 0, material_entry_item(me, "diffuse_map").as_asset);
    }
    shader_program_enable_texture(shader, "bump_map", 1, material_entry_item(me, "bump_map").as_asset);
    shader_program_enable_texture(shader, "spec_map", 2, material_entry_item(me, "spec_map").as_asset);
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
    
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
    
    shader_program_disable_attribute(shader, "vPosition");
    shader_program_disable_attribute(shader, "vNormal");
    shader_program_disable_attribute(shader, "vTangent");
    shader_program_disable_attribute(shader, "vBinormal");
    shader_program_disable_attribute(shader, "vTexcoord");
    shader_program_disable_attribute(shader, "vColor");
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    shader_program_disable_texture(shader, 2);
    shader_program_disable_texture(shader, 1);
    shader_program_disable_texture(shader, 0);

  }
  
  shader_program_disable(shader);

}

static void render_animated(deferred_renderer* dr, animated_object* ao) {
  
  config* options = asset_get_load(P("./assets/options.cfg"));
  
  skeleton* skel = asset_hndl_ptr(ao->skeleton);

  if (skel->num_bones > MAX_BONES) { error("animated object skeleton has too many bones (over %i)", MAX_BONES); }
  if (ao->pose == NULL) { return; }
  
  for(int i = 0; i < skel->num_bones; i++) {
    mat4 base = bone_transform(skel->bones[i]);
    mat4 ani = bone_transform(ao->pose->bones[i]);
    bone_matrices[i] = mat4_mul_mat4(ani, mat4_inverse(base));
  }
  
  mat4 world = mat4_world( ao->position, ao->scale, ao->rotation );
  mat4 inv_view = mat4_inverse(camera_view_matrix(dr->camera));
  mat4 inv_proj = mat4_inverse(camera_proj_matrix(dr->camera));
  frustum frus = frustum_new_clipbox();
  frus = frustum_transform(frus, inv_proj);
  frus = frustum_transform(frus, inv_view);  
  
  renderable* r = asset_hndl_ptr(ao->renderable);
  
  if(!r->is_rigged) { error("Animated object is not rigged!"); }
  
  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_animated));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", world);
  shader_program_set_mat4(shader, "view", camera_view_matrix(dr->camera));
  shader_program_set_mat4(shader, "proj", camera_proj_matrix(dr->camera));
  shader_program_set_float(shader, "near", dr->camera->near_clip);
  shader_program_set_float(shader, "far", dr->camera->far_clip);
  shader_program_set_mat4_array(shader, "world_bones", bone_matrices, skel->num_bones);
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    
    if (sphere_outside_frustum(sphere_transform(s->bound, world), frus)) { continue; }
    
    int mat_id = min(i, ((material*)asset_hndl_ptr(r->material))->num_entries-1);
    material_entry* me = material_get_entry(asset_hndl_ptr(r->material), mat_id);
    
    if (config_bool(options, "render_white")) {
      shader_program_enable_texture(shader, "diffuse_map", 0, asset_hndl_new_load(P("$CORANGE/resources/white.dds")));
    } else {
      shader_program_enable_texture(shader, "diffuse_map", 0, material_entry_item(me, "diffuse_map").as_asset);
    }
    shader_program_enable_texture(shader, "bump_map", 1, material_entry_item(me, "bump_map").as_asset);
    shader_program_enable_texture(shader, "spec_map", 2, material_entry_item(me, "spec_map").as_asset);
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
    
    shader_program_disable_texture(shader, 2);
    shader_program_disable_texture(shader, 1);
    shader_program_disable_texture(shader, 0);

  }
  
  shader_program_disable(shader);
  
}

void render_landscape(deferred_renderer* dr, landscape* l) {

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  terrain* terr = asset_hndl_ptr(l->heightmap);
  
  vec3 scale = vec3_new(-(1.0 / terr->width) * l->size_x, l->scale, -(1.0 / terr->height) * l->size_y);
  vec3 translation = vec3_new(l->size_x / 2, 0, l->size_y / 2);
  
  config* options = asset_get_load(P("./assets/options.cfg"));
  if (config_bool(options, "render_colmeshes")) {
  
    for(int i = 0; i < terr->num_chunks; i++) {
          
      terrain_chunk* tc = terr->chunks[i];
      render_cmesh(dr, tc->colmesh, landscape_world(l));  
      
    }
  
  }
  
  mat4 inv_view = mat4_inverse(camera_view_matrix(dr->camera));
  mat4 inv_proj = mat4_inverse(camera_proj_matrix(dr->camera));
  frustum frus = frustum_new_clipbox();
  frus = frustum_transform(frus, inv_proj);
  frus = frustum_transform(frus, inv_view);
  
  // This assumes that X or Z scale of a chunk will never exceed the height.
  float bound_scale_val = max(scale.x, scale.z);
  vec3 bound_scale = vec3_new(bound_scale_val, bound_scale_val, bound_scale_val);  
  
  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_terrain));
  
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", landscape_world(l));
  shader_program_set_mat4(shader, "view", camera_view_matrix(dr->camera));
  shader_program_set_mat4(shader, "proj", camera_proj_matrix(dr->camera));
  shader_program_set_float(shader, "near", dr->camera->near_clip);
  shader_program_set_float(shader, "far", dr->camera->far_clip);
  shader_program_set_float(shader, "size_x", l->size_x);
  shader_program_set_float(shader, "size_y", l->size_y);
  
  if (config_bool(options, "render_white")) {
    shader_program_enable_texture(shader, "ground0", 0, asset_hndl_new_load(P("$CORANGE/resources/white.dds")));
    shader_program_enable_texture(shader, "ground1", 1, asset_hndl_new_load(P("$CORANGE/resources/white.dds")));
    shader_program_enable_texture(shader, "ground2", 2, asset_hndl_new_load(P("$CORANGE/resources/white.dds")));
    shader_program_enable_texture(shader, "ground3", 3, asset_hndl_new_load(P("$CORANGE/resources/white.dds")));
  } else {
    shader_program_enable_texture(shader, "ground0", 0, l->ground0);
    shader_program_enable_texture(shader, "ground1", 1, l->ground1);
    shader_program_enable_texture(shader, "ground2", 2, l->ground2);
    shader_program_enable_texture(shader, "ground3", 3, l->ground3);
  }
  
  shader_program_enable_texture(shader, "ground0_nm", 4, l->ground0_nm);
  shader_program_enable_texture(shader, "ground1_nm", 5, l->ground1_nm);
  shader_program_enable_texture(shader, "ground2_nm", 6, l->ground2_nm);
  shader_program_enable_texture(shader, "ground3_nm", 7, l->ground3_nm);
  shader_program_enable_texture(shader, "attribmap", 8, l->attribmap);
  
  for(int i = 0; i < terr->num_chunks; i++) {
    
    terrain_chunk* tc = terr->chunks[i];
    
    float chunkx = (1.0 / (terr->num_rows-1)) *  l->size_x;
    float chunky = (1.0 / (terr->num_cols-1)) *  l->size_y;
    float posx = ((float)(i % terr->num_cols)) * chunkx - l->size_x / 2;
    float posy = ((float)(i / terr->num_cols)) * chunky - l->size_y / 2;
    
    sphere bound = sphere_transform(tc->bound, mat4_world(translation, bound_scale, mat4_id()));
    if (sphere_outside_frustum(bound, frus)) { continue; }
    
    float dist = vec2_dist_sqrd(
      vec2_new(dr->camera->position.x, dr->camera->position.z), 
      vec2_new(-posx, -posy)) / (100 * NUM_TERRAIN_BUFFERS);
    
    int buff_index = clamp((int)dist, 0, NUM_TERRAIN_BUFFERS-1);
    
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
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  }
  
  shader_program_disable_texture(shader, 0);
  shader_program_disable_texture(shader, 1);
  shader_program_disable_texture(shader, 2);
  shader_program_disable_texture(shader, 3);
  shader_program_disable_texture(shader, 4);
  shader_program_disable_texture(shader, 5);
  shader_program_disable_texture(shader, 6);
  shader_program_disable_texture(shader, 7);
  shader_program_disable_texture(shader, 8);
  shader_program_disable(shader);
  
  //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
}


void render_light(deferred_renderer* dr, light* l) {
  
  vec3 light_pos = l->position;
  light_pos = mat4_mul_vec3(camera_view_matrix(dr->camera), light_pos);
  light_pos = mat4_mul_vec3(camera_proj_matrix(dr->camera), light_pos);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_ui));
  shader_program_enable(shader);
  shader_program_enable_texture(shader, "diffuse", 0, asset_hndl_new_load(P("$CORANGE/ui/lightbulb.dds")));
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
  
  shader_program_disable_texture(shader, 0);
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

  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_ui));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", camera_view_matrix(dr->camera));
  shader_program_set_mat4(shader, "proj", camera_proj_matrix(dr->camera));
  shader_program_set_float(shader, "alpha_test", 0.0);
  shader_program_enable_texture(shader, "diffuse", 0, asset_hndl_new_load(P("$CORANGE/ui/white.dds")));
  
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

  shader_program_disable_texture(shader, 0);
  shader_program_disable(shader);
  
}

void render_ellipsoid(deferred_renderer* dr, ellipsoid e) {
  
  static_object so;
  so.position = e.center;
  so.rotation = mat4_id();
  so.scale = vec3_new(e.radiuses.x, e.radiuses.y, e.radiuses.z);
  so.renderable = dr->mesh_sphere;
  so.collision_body = asset_hndl_new(P("$CORANGE/resources/sphere.col"));
  
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

void render_paint_circle(deferred_renderer* dr, vec3 position, vec3 normal, float radius) {
  
  vec3 axis_x = vec3_cross(normal, vec3_new(1, 0, 0));
  vec3 axis_z = vec3_cross(normal, axis_x);
  
  mat4 world = mat4_new(
    axis_z.x, axis_z.y, axis_z.z, position.x,
    normal.x, normal.y, normal.z, position.y,
    axis_x.x, axis_x.y, axis_x.z, position.z,
           0,        0,        0,          1);
  
  render_axis(dr, world);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_ui));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", camera_view_matrix(dr->camera));
  shader_program_set_mat4(shader, "proj", camera_proj_matrix(dr->camera));
  shader_program_enable_texture(shader, "diffuse", 0, asset_hndl_new_load(P("$CORANGE/ui/white.dds")));
  shader_program_set_float(shader, "alpha_test", 0.0);
  
  glDisable(GL_DEPTH_TEST);
  glLineWidth(1.0);
  
  int circle_items = 0;
  float circle_positions[512];
  float circle_colors[512];
  
  for(float i = 0; i < M_PI * 2; i += 0.1) {
    
    vec3 point0 = vec3_mul(vec3_new(sin(i+0.0), 0, cos(i+0.0)), radius);
    vec3 point1 = vec3_mul(vec3_new(sin(i+0.1), 0, cos(i+0.1)), radius);
    
    point0 = mat4_mul_vec3(world, point0);
    point1 = mat4_mul_vec3(world, point1);
  
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
  
  shader_program_disable_texture(shader, 0);
  shader_program_disable(shader);
  
}

static void render_gbuffer(deferred_renderer* dr) {
  
  config* options = asset_get_load(P("./assets/options.cfg"));
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  int gwidth  = width  * option_graphics_int(options, "graphics_msaa", 4, 2, 1);
  int gheight = height * option_graphics_int(options, "graphics_msaa", 4, 2, 1);
  
  glBindFramebuffer(GL_FRAMEBUFFER, dr->gfbo);
  glDrawBuffers(3, (GLenum[]){ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 });
  glViewport( 0, 0, gwidth, gheight);
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  
  for ( int j = 0; j < dr->render_objects_num; j++) {
    
    // HACK ALERT
    bool veg_found = false;
    if (dr->render_objects[j].type == RO_TYPE_STATIC) {
      renderable* r = asset_hndl_ptr(dr->render_objects[j].static_object->renderable);
      material* m = asset_hndl_ptr(r->material);
      
      for (int i = 0; i < m->num_entries; i++) {
        if (material_entry_item(m->entries[i], "material").as_int == 6) {
          render_vegetation(dr, dr->render_objects[j].static_object);
          veg_found = true;
          break;
        }
      }
      
    }
    
    if (veg_found) continue;
    
    if (dr->render_objects[j].type == RO_TYPE_STATIC)     { render_static(dr, dr->render_objects[j].static_object); }
    if (dr->render_objects[j].type == RO_TYPE_ANIMATED)   { render_animated(dr, dr->render_objects[j].animated_object); }
    if (dr->render_objects[j].type == RO_TYPE_LANDSCAPE)  { render_landscape(dr, dr->render_objects[j].landscape); }
    if (dr->render_objects[j].type == RO_TYPE_LIGHT)      { render_light(dr, dr->render_objects[j].light); }
    if (dr->render_objects[j].type == RO_TYPE_AXIS)       { render_axis(dr, dr->render_objects[j].axis); }
    if (dr->render_objects[j].type == RO_TYPE_SPHERE)     { render_ellipsoid(dr, ellipsoid_of_sphere(dr->render_objects[j].sphere)); }
    if (dr->render_objects[j].type == RO_TYPE_ELLIPSOID)  { render_ellipsoid(dr, dr->render_objects[j].ellipsoid); }
    if (dr->render_objects[j].type == RO_TYPE_PROJECTILE) { render_projectile(dr, dr->render_objects[j].projectile); }
    
    if (dr->render_objects[j].type == RO_TYPE_CMESH) {
      render_cmesh(dr, 
        dr->render_objects[j].colmesh, 
        dr->render_objects[j].colworld); }
        
    if (dr->render_objects[j].type == RO_TYPE_PAINT) {
      render_paint_circle(dr, 
        dr->render_objects[j].paint_pos, 
        dr->render_objects[j].paint_norm, 
        dr->render_objects[j].paint_radius); }
  }

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
}

static void render_ssao(deferred_renderer* dr) {
  
  config* options = asset_get_load(P("./assets/options.cfg"));
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  int ssaowidth  = width  * option_graphics_int(options, "graphics_ssao", 1, 0.5, 0.25);
  int ssaoheight = height * option_graphics_int(options, "graphics_ssao", 1, 0.5, 0.25);
  
  glBindFramebuffer(GL_FRAMEBUFFER, dr->ssao_fbo);
  glViewport(0, 0, ssaowidth, ssaoheight);
  glClearColor(1, 1, 1, 1);
  glClear( GL_COLOR_BUFFER_BIT );
  
  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_ssao));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", mat4_id());
  shader_program_set_mat4(shader, "proj", mat4_orthographic(-1, 1, -1, 1, -1, 1));
  
  shader_program_enable_texture(shader, "random_texture", 0, dr->tex_random);
  shader_program_enable_texture_id(shader, "depth_texture", 1, dr->gdepth_texture);
  shader_program_enable_texture_id(shader, "normals_texture", 2, dr->gnormals_texture);  
  shader_program_enable_texture_id(shader, "positions_texture", 3, dr->gpositions_texture);

  shader_program_set_int(shader, "width", graphics_viewport_width());
  shader_program_set_int(shader, "height", graphics_viewport_height());
  shader_program_set_float(shader, "clip_far", dr->camera->far_clip);
  shader_program_set_float(shader, "clip_near", dr->camera->near_clip);
  
  shader_program_enable_attribute(shader, "vPosition",  3, 3, quad_position);
  shader_program_enable_attribute(shader, "vTexcoord",  2, 2, quad_texcoord);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
  
  shader_program_disable_attribute(shader, "vPosition");
  shader_program_disable_attribute(shader, "vTexcoord");
  
  shader_program_disable_texture(shader, 3);
  shader_program_disable_texture(shader, 2);
  shader_program_disable_texture(shader, 1);
  shader_program_disable_texture(shader, 0);
  shader_program_disable(shader);
  
  glViewport(0, 0, graphics_viewport_width(), graphics_viewport_height());
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, dr->ssao_texture);
  glEnable(GL_TEXTURE_2D);
  glGenerateMipmap(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_2D);
}

static void render_skies(deferred_renderer* dr) {
  
  if (!dr->skydome_enabled) { return; }
  
  config* options = asset_get_load(P("./assets/options.cfg"));
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  int hdrwidth  = width  * option_graphics_int(options, "graphics_msaa", 4, 2, 1);
  int hdrheight = height * option_graphics_int(options, "graphics_msaa", 4, 2, 1);
  
  glBindFramebuffer(GL_FRAMEBUFFER, dr->hdr_fbo);
  glViewport(0, 0, hdrwidth, hdrheight);
  glDepthMask(GL_FALSE);
  
  {
  
    shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_skydome));
    shader_program_enable(shader);
    shader_program_set_mat4(shader, "world", mat4_world(dr->camera->position, vec3_new(10, 10, 10), mat4_id()));
    shader_program_set_mat4(shader, "view", camera_view_matrix(dr->camera));
    shader_program_set_mat4(shader, "proj", camera_proj_matrix(dr->camera));
    shader_program_set_vec3(shader, "light_direction", sky_sun_direction(dr->time_of_day));
    //shader_program_set_vec3(shader, "camera_position", dr->camera->position);
    
    renderable* skybox_r = asset_hndl_ptr(dr->mesh_skydome);
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
  
    shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_sun));
    shader_program_enable(shader);
    shader_program_set_mat4(shader, "world", mat4_world(dr->camera->position, vec3_one(), sky_mesh_sun_world(dr->time_of_day)));
    shader_program_set_mat4(shader, "view", camera_view_matrix(dr->camera));
    shader_program_set_mat4(shader, "proj", camera_proj_matrix(dr->camera));
    shader_program_set_float(shader, "sun_brightness", 1.5w);
    shader_program_set_vec4(shader, "sun_color", vec4_one());
    shader_program_enable_texture(shader, "sun_texture", 0, sky_tex_sun(dr->time_of_day));
    
    renderable* sun_r = asset_hndl_ptr(sky_mesh_sun(dr->time_of_day));
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
    
    shader_program_disable_texture(shader, 0);
    shader_program_disable(shader);
    
    glDisable(GL_BLEND);
  
  }
  
  {
  
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
    shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_clouds));
    shader_program_enable(shader);
    shader_program_set_mat4(shader, "world", mat4_world(dr->camera->position, vec3_new(10, 10, 10), mat4_id()));
    shader_program_set_mat4(shader, "view", camera_view_matrix(dr->camera));
    shader_program_set_mat4(shader, "proj", camera_proj_matrix(dr->camera));
    shader_program_set_float(shader, "time", dr->time);
    shader_program_set_float(shader, "wind", vec3_length(sky_wind(dr->time_of_day, dr->seed)));
    shader_program_set_vec3(shader, "cloud_color", vec3_one());
    shader_program_set_vec3(shader, "cloud_light", vec3_mul(vec3_one(), 2));
    
    for (int i = 0; i < sky_clouds_num(); i++) {
    
      shader_program_enable_texture(shader, "cloud_texture", 0, sky_clouds_tex(i));
      shader_program_set_float(shader, "opacity", sky_clouds_opacity(i, dr->time_of_day, dr->seed));
      
      renderable* sun_r = asset_hndl_ptr(sky_clouds_mesh(i));
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
      
      shader_program_disable_texture(shader, 0);
    
    }
    
    shader_program_disable(shader);
    
    glDisable(GL_BLEND);
  
  }
  
  glDepthMask(GL_TRUE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);  
  
}

static void render_compose(deferred_renderer* dr) {
  
  config* options = asset_get_load(P("./assets/options.cfg"));
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  int hdrwidth  = width  * option_graphics_int(options, "graphics_msaa", 4, 2, 1);
  int hdrheight = height * option_graphics_int(options, "graphics_msaa", 4, 2, 1);
  
  glBindFramebuffer(GL_FRAMEBUFFER, dr->hdr_fbo);
  glViewport(0, 0, hdrwidth, hdrheight);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_compose));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", mat4_id());
  shader_program_set_mat4(shader, "proj", mat4_orthographic(-1, 1, -1, 1, -1, 1));

  shader_program_enable_texture(shader, "env_texture", 0, dr->tex_environment);  
  shader_program_enable_texture(shader, "random_texture", 1, dr->tex_random);
  shader_program_enable_texture_id(shader, "diffuse_texture", 2, dr->gdiffuse_texture);
  shader_program_enable_texture_id(shader, "positions_texture", 3, dr->gpositions_texture);
  shader_program_enable_texture_id(shader, "normals_texture", 4, dr->gnormals_texture);
  shader_program_enable_texture_id(shader, "ssao_texture", 5, dr->ssao_texture);
  shader_program_enable_texture_id(shader, "depth_texture", 6, dr->gdepth_texture);
  shader_program_enable_texture_id(shader, "shadows_texture0", 7, dr->shadows_texture[0]);
  shader_program_enable_texture_id(shader, "shadows_texture1", 8, dr->shadows_texture[1]);
  shader_program_enable_texture_id(shader, "shadows_texture2", 9, dr->shadows_texture[2]);
  
  mat4 light_proj[3];
  mat4 light_view[3];
  
  float light_clip_near[3];
  float light_clip_far[3];
  
  for (int i = 0; i < 3; i++) {
    shadow_mapper_transforms(dr, i, 
      &light_view[i], &light_proj[i], 
      &light_clip_near[i], &light_clip_far[i]);
  }
  
  shader_program_set_vec3(shader, "camera_position", dr->camera->position);
  //shader_program_set_float_array(shader, "light_clip_near", light_clip_near, 3);
  //shader_program_set_float_array(shader, "light_clip_far", light_clip_far, 3);
  shader_program_set_mat4_array(shader, "light_view", light_view, 3);
  shader_program_set_mat4_array(shader, "light_proj", light_proj, 3);
  shader_program_set_float_array(shader, "light_start", dr->shadows_start, 3);
  
  float light_power[DEFERRED_MAX_LIGHTS];
  float light_falloff[DEFERRED_MAX_LIGHTS];
  vec3 light_position[DEFERRED_MAX_LIGHTS];
  vec3 light_target[DEFERRED_MAX_LIGHTS];
  vec3 light_diffuse[DEFERRED_MAX_LIGHTS];
  vec3 light_ambient[DEFERRED_MAX_LIGHTS];
  vec3 light_specular[DEFERRED_MAX_LIGHTS];
    
  light_power[0]    = sky_sun_power(dr->time_of_day);
  light_falloff[0]  = 0;
  light_position[0] = vec3_neg(sky_sun_direction(dr->time_of_day));
  light_target[0]   = vec3_zero();
  light_diffuse[0]  = sky_sun_diffuse(dr->time_of_day);
  light_ambient[0]  = sky_sun_ambient(dr->time_of_day);
  light_specular[0] = sky_sun_specular(dr->time_of_day);
  
  light_power[1]    = sky_moon_power(dr->time_of_day);
  light_falloff[1]  = 0;
  light_position[1] = vec3_neg(sky_moon_direction(dr->time_of_day));
  light_target[1]   = vec3_zero();
  light_diffuse[1]  = sky_moon_diffuse(dr->time_of_day);
  light_ambient[1]  = sky_moon_ambient(dr->time_of_day);
  light_specular[1] = sky_moon_specular(dr->time_of_day);
  
  light_power[2]    = sky_sky_power(dr->time_of_day);
  light_falloff[2]  = 0;
  light_position[2] = vec3_neg(sky_sky_direction(dr->time_of_day));
  light_target[2]   = vec3_zero();
  light_diffuse[2]  = sky_sky_diffuse(dr->time_of_day);
  light_ambient[2]  = sky_sky_ambient(dr->time_of_day);
  light_specular[2] = sky_sky_specular(dr->time_of_day);
  
  light_power[3]    = sky_ground_power(dr->time_of_day);
  light_falloff[3]  = 0;
  light_position[3] = vec3_neg(sky_ground_direction(dr->time_of_day));
  light_target[3]   = vec3_zero();
  light_diffuse[3]  = sky_ground_diffuse(dr->time_of_day);
  light_ambient[3]  = sky_ground_ambient(dr->time_of_day);
  light_specular[3] = sky_ground_specular(dr->time_of_day);
  
  const int ln = 4;
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
  shader_program_set_int(shader, "light_shadows", sky_isday(dr->time_of_day) ? 0 : 1);
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
  
  shader_program_disable_texture(shader, 9);
  shader_program_disable_texture(shader, 8);
  shader_program_disable_texture(shader, 7);
  shader_program_disable_texture(shader, 6);
  shader_program_disable_texture(shader, 5);
  shader_program_disable_texture(shader, 4);
  shader_program_disable_texture(shader, 3);
  shader_program_disable_texture(shader, 2);
  shader_program_disable_texture(shader, 1);
  shader_program_disable_texture(shader, 0);
  shader_program_disable(shader);
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, width, height);
  
}

static void render_particles(deferred_renderer* dr) {
  
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  
  config* options = asset_get_load(P("./assets/options.cfg"));
  
  int width = graphics_viewport_width();
  int height = graphics_viewport_height();
  
  int hdrwidth  = width  * option_graphics_int(options, "graphics_msaa", 4, 2, 1);
  int hdrheight = height * option_graphics_int(options, "graphics_msaa", 4, 2, 1);
  
  glBindFramebuffer(GL_FRAMEBUFFER, dr->hdr_fbo);
  glViewport(0, 0, hdrwidth, hdrheight);
  
  glEnable(GL_BLEND);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_particles));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "view", camera_view_matrix(dr->camera));
  shader_program_set_mat4(shader, "proj", camera_proj_matrix(dr->camera));
  shader_program_set_float(shader, "clip_near", dr->camera->near_clip);
  shader_program_set_float(shader, "clip_far", dr->camera->far_clip);

  shader_program_set_float(shader, "light_power", sky_sun_power(dr->time_of_day));
  shader_program_set_vec3(shader, "light_direction", sky_sun_direction(dr->time_of_day));
  shader_program_set_vec3(shader, "light_diffuse", sky_sun_diffuse(dr->time_of_day));
  shader_program_set_vec3(shader, "light_ambient", sky_sky_ambient(dr->time_of_day));
  
  for (int i = 0; i < dr->render_objects_num; i++) {
    
    if (dr->render_objects[i].type != RO_TYPE_PARTICLES) { continue; }
    
    particles* p = dr->render_objects[i].particles;
    effect* e = asset_hndl_ptr(p->effect);
    
    glBlendFunc(e->blend_src, e->blend_dst);
    
    shader_program_set_float(shader, "particle_depth", e->depth);
    shader_program_set_float(shader, "particle_thickness", e->thickness);
    shader_program_set_float(shader, "particle_bumpiness", e->bumpiness);
    shader_program_set_float(shader, "particle_scattering", e->scattering);
    
    shader_program_set_mat4(shader, "world", mat4_world(p->position, p->scale, p->rotation));
    shader_program_enable_texture(shader, "particle_diffuse", 0, e->texture);
    shader_program_enable_texture(shader, "particle_normals", 1, e->texture_nm);
    shader_program_enable_texture_id(shader, "depth", 2, dr->gdepth_texture);
    
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

  shader_program_disable_texture(shader, 2);
  shader_program_disable_texture(shader, 1);
  shader_program_disable_texture(shader, 0);
  shader_program_disable(shader);
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, width, height);
  
  //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
}

static void render_tonemap(deferred_renderer* dr) {
  
  glBindFramebuffer(GL_FRAMEBUFFER, dr->ldr_back_fbo);
  glViewport(0, 0, graphics_viewport_width(), graphics_viewport_height());

  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_tonemap));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", mat4_id());
  shader_program_set_mat4(shader, "proj", mat4_orthographic(-1, 1, -1, 1, -1, 1));
  shader_program_set_float(shader, "exposure", dr->exposure);
  shader_program_enable_texture_id(shader, "hdr_texture", 0, dr->hdr_texture);
  
  shader_program_enable_attribute(shader, "vPosition",  3, 3, quad_position);
  shader_program_enable_attribute(shader, "vTexcoord",  2, 2, quad_texcoord);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
  
  shader_program_disable_attribute(shader, "vPosition");
  shader_program_disable_attribute(shader, "vTexcoord");
  
  shader_program_disable_texture(shader, 0);
  shader_program_disable(shader);
  
  /* Generate Mipmaps, adjust exposure */
  
  unsigned char color[4] = {0,0,0,0};
  int level = -1; int width = 0; int height = 0;
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, dr->ldr_back_texture);
  glEnable(GL_TEXTURE_2D);
  glGenerateMipmap(GL_TEXTURE_2D);
  
  do {
    level++;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &height);
    if (level > 50) { error("Unable to find lowest mip level. Perhaps mipmaps were not generated"); }
  } while ((width > 1) || (height > 1));
  
  glGetTexImage(GL_TEXTURE_2D, level, GL_RGBA, GL_UNSIGNED_BYTE, color);
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
  
  float average = (float)(color[0] + color[1] + color[2]) / (3.0 * 255.0);
  
  //EXPOSURE += (EXPOSURE_TARGET - average) * EXPOSURE_SPEED;
  dr->exposure = 4.0;
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, graphics_viewport_width(), graphics_viewport_height());
  
}

static void render_post0(deferred_renderer* dr) {
  
  glBindFramebuffer(GL_FRAMEBUFFER, dr->ldr_front_fbo);
  glViewport(0, 0, graphics_viewport_width(), graphics_viewport_height());
  glClearColor(1.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_post0));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", mat4_id());
  shader_program_set_mat4(shader, "proj", mat4_orthographic(-1, 1, -1, 1, -1, 1));
  shader_program_enable_texture_id(shader, "ldr_texture", 0, dr->ldr_back_texture);
  //shader_program_enable_texture(shader, "random_texture", 1, dr->tex_random);
  
  /*
  glActiveTexture(GL_TEXTURE0 + 2 );
  glBindTexture(GL_TEXTURE_2D, dr->gdepth_texture);
  glEnable(GL_TEXTURE_2D);
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
  
  glActiveTexture(GL_TEXTURE0 + 2 );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1000);
  glDisable(GL_TEXTURE_2D);
  
  //shader_program_disable_texture(shader, 1);
  shader_program_disable_texture(shader, 0);
  shader_program_disable(shader);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, dr->ldr_front_texture);
  glEnable(GL_TEXTURE_2D);
  glGenerateMipmap(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_2D);
  
}

static void render_post1(deferred_renderer* dr) {
  
  config* options = asset_get_load(P("./assets/options.cfg"));
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, graphics_viewport_width(), graphics_viewport_height());
  glClearDepth(1.0);
  glClearColor(1.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(dr->mat_post1));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", mat4_id());
  shader_program_set_mat4(shader, "proj", mat4_orthographic(-1, 1, -1, 1, -1, 1));
  
  shader_program_enable_texture_id(shader, "ldr_texture", 0, dr->ldr_front_texture);
  shader_program_enable_texture(shader, "random_perlin", 1, dr->tex_random_perlin);
  shader_program_enable_texture(shader, "vignetting_texture", 2, dr->tex_vignetting);
  
  glActiveTexture(GL_TEXTURE0 + 3 );
  glBindTexture(GL_TEXTURE_3D, texture_handle(asset_hndl_ptr(dr->tex_color_correction)));
  glEnable(GL_TEXTURE_3D);
  shader_program_set_int(shader, "lut", 3);
  
  shader_program_set_float(shader, "glitch", dr->glitch);
  shader_program_set_float(shader, "time", dr->time);
  shader_program_set_int(shader, "width", graphics_viewport_width());
  shader_program_set_int(shader, "height", graphics_viewport_height());
  shader_program_set_int(shader, "fxaa_quality", config_int(options, "graphics_fxaa"));
  
  shader_program_enable_attribute(shader, "vPosition",  3, 3, quad_position);
  shader_program_enable_attribute(shader, "vTexcoord",  2, 2, quad_texcoord);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
  
  shader_program_disable_attribute(shader, "vPosition");
  shader_program_disable_attribute(shader, "vTexcoord");
  
  glActiveTexture(GL_TEXTURE0 + 3 );
  glDisable(GL_TEXTURE_3D);
  
  shader_program_disable_texture(shader, 2);
  shader_program_disable_texture(shader, 1);
  shader_program_disable_texture(shader, 0);
  shader_program_disable(shader);
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, dr->ldr_back_texture);
  glEnable(GL_TEXTURE_2D);
  glGenerateMipmap(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_2D);
  
}

void deferred_renderer_render(deferred_renderer* dr) {
  
  dr->time += frame_time();
  
  //timer t = timer_start(0, "Rendering Start");
  
  render_shadows(dr);   //t = timer_split(t, "Shadow");
  render_clear(dr);     //t = timer_split(t, "Clear");
  render_gbuffer(dr);   //t = timer_split(t, "GBuffer");
  render_ssao(dr);      //t = timer_split(t, "SSAO");
  render_skies(dr);     //t = timer_split(t, "Skies");
  render_compose(dr);   //t = timer_split(t, "Compose");
  render_particles(dr); //t = timer_split(t, "Particles");
  render_tonemap(dr);   //t = timer_split(t, "Tonemap");
  render_post0(dr);     //t = timer_split(t, "Post0");
  render_post1(dr);     //t = timer_split(t, "Post1");
  
  //timer_stop(t, "Rendering End");
  
  dr->render_objects_num = 0;
  dr->dyn_lights_num = 0;
  
}
 
