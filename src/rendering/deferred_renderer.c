#include "rendering/deferred_renderer.h"

#include "graphics_manager.h"

#include "assets/shader.h"
#include "assets/texture.h"
#include "assets/material.h"
#include "assets/renderable.h"

static float PROJ_MATRIX[16];
static float VIEW_MATRIX[16];
static float WORLD_MATRIX[16];

static float LIGHT_VIEW_MATRIX[16];
static float LIGHT_PROJ_MATRIX[16];

static asset_hndl MAT_STATIC;
static asset_hndl MAT_ANIMATED;
static asset_hndl MAT_CLEAR;
static asset_hndl MAT_UI;
static asset_hndl MAT_SSAO;
static asset_hndl MAT_COMPOSE;
static asset_hndl MAT_TONEMAP;
static asset_hndl MAT_POST;

static int NORMAL;
static int TANGENT;
static int BINORMAL;

static int NORMAL_ANIMATED;
static int TANGENT_ANIMATED;
static int BINORMAL_ANIMATED;
static int BONE_INDICIES;
static int BONE_WEIGHTS;

static GLuint fbo;
static GLuint depth_buffer;
static GLuint diffuse_buffer;
static GLuint positions_buffer;
static GLuint normals_buffer;

static GLuint diffuse_texture;
static GLuint positions_texture;
static GLuint normals_texture;
static GLuint depth_texture;

static GLuint ssao_fbo;
static GLuint ssao_buffer;
static GLuint ssao_texture;

static GLuint hdr_fbo;
static GLuint hdr_buffer;
static GLuint hdr_texture;

static GLuint ldr_fbo;
static GLuint ldr_buffer;
static GLuint ldr_texture;

static camera* CAMERA = NULL;
static light* SHADOW_LIGHT = NULL;
static texture* SHADOW_TEX = NULL;

static asset_hndl COLOR_CORRECTION;
static asset_hndl RANDOM;
static asset_hndl ENVIRONMENT;
static asset_hndl VIGNETTING;

#define DEFERRED_MAX_LIGHTS 32

static int num_lights;

static light* lights[DEFERRED_MAX_LIGHTS];
static float light_power[DEFERRED_MAX_LIGHTS];
static float light_falloff[DEFERRED_MAX_LIGHTS];
static vec3 light_position[DEFERRED_MAX_LIGHTS];
static vec3 light_target[DEFERRED_MAX_LIGHTS];
static vec3 light_diffuse[DEFERRED_MAX_LIGHTS];
static vec3 light_ambient[DEFERRED_MAX_LIGHTS];
static vec3 light_specular[DEFERRED_MAX_LIGHTS];

static float EXPOSURE;
static float EXPOSURE_SPEED;
static float EXPOSURE_TARGET;

void deferred_renderer_init() {
  
  num_lights = 0;
  
  EXPOSURE = 0.0;
  EXPOSURE_SPEED = 1.0;
  EXPOSURE_TARGET = 0.4;
  
  COLOR_CORRECTION = asset_hndl_new(P("$CORANGE/resources/identity.lut"));
  RANDOM = asset_hndl_new(P("$CORANGE/resources/random.dds"));
  ENVIRONMENT = asset_hndl_new(P("$CORANGE/resources/envmap.dds"));
  VIGNETTING = asset_hndl_new(P("$CORANGE/resources/vignetting.dds"));
  
  folder_load(P("$CORANGE/shaders/deferred/"));
  
  MAT_STATIC = asset_hndl_new(P("$CORANGE/shaders/deferred/static.mat"));
  MAT_ANIMATED = asset_hndl_new(P("$CORANGE/shaders/deferred/animated.mat"));
  MAT_CLEAR = asset_hndl_new(P("$CORANGE/shaders/deferred/clear.mat"));
  MAT_SSAO = asset_hndl_new(P("$CORANGE/shaders/deferred/ssao.mat"));
  MAT_TONEMAP = asset_hndl_new(P("$CORANGE/shaders/deferred/tonemap.mat"));
  MAT_COMPOSE = asset_hndl_new(P("$CORANGE/shaders/deferred/compose.mat"));
  MAT_POST = asset_hndl_new(P("$CORANGE/shaders/deferred/post.mat"));
  MAT_UI = asset_hndl_new(P("$CORANGE/shaders/deferred/ui.mat"));
  
  
  shader_program* program_static = material_get_entry(asset_hndl_ptr(MAT_STATIC), 0)->program;
  shader_program* program_animated = material_get_entry(asset_hndl_ptr(MAT_ANIMATED), 0)->program;
  
  NORMAL = glGetAttribLocation(shader_program_handle(program_static), "normal");
  TANGENT = glGetAttribLocation(shader_program_handle(program_static), "tangent");
  BINORMAL = glGetAttribLocation(shader_program_handle(program_static), "binormal");  
  
  NORMAL_ANIMATED = glGetAttribLocation(shader_program_handle(program_animated), "normal");
  TANGENT_ANIMATED = glGetAttribLocation(shader_program_handle(program_animated), "tangent");
  BINORMAL_ANIMATED = glGetAttribLocation(shader_program_handle(program_animated), "binormal");  
  BONE_INDICIES = glGetAttribLocation(shader_program_handle(program_animated), "bone_indicies");
  BONE_WEIGHTS = glGetAttribLocation(shader_program_handle(program_animated), "bone_weights"); 
  
  int viewport_width = graphics_viewport_width();
  int viewport_height = graphics_viewport_height();
  
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  
  glGenRenderbuffers(1, &diffuse_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, diffuse_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, viewport_width, viewport_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, diffuse_buffer);   
  
  glGenRenderbuffers(1, &positions_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, positions_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F, viewport_width, viewport_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, positions_buffer);  
  
  glGenRenderbuffers(1, &normals_buffer);  
  glBindRenderbuffer(GL_RENDERBUFFER, normals_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F, viewport_width, viewport_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, normals_buffer);  
  
  glGenRenderbuffers(1, &depth_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, viewport_width, viewport_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);  
  
  glGenTextures(1, &diffuse_texture);
  glBindTexture(GL_TEXTURE_2D, diffuse_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport_width, viewport_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, diffuse_texture, 0);
  
  glGenTextures(1, &positions_texture);
  glBindTexture(GL_TEXTURE_2D, positions_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, viewport_width, viewport_height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, positions_texture, 0);
  
  glGenTextures(1, &normals_texture);
  glBindTexture(GL_TEXTURE_2D, normals_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, viewport_width, viewport_height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, normals_texture, 0);
  
  glGenTextures(1, &depth_texture);
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, viewport_width, viewport_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
  
  
  glGenFramebuffers(1, &ssao_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
  
  glGenRenderbuffers(1, &ssao_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, ssao_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, viewport_width / 2, viewport_height / 2);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ssao_buffer);   
  
  glGenTextures(1, &ssao_texture);
  glBindTexture(GL_TEXTURE_2D, ssao_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport_width / 2, viewport_height / 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_texture, 0);
  
  glGenFramebuffers(1, &hdr_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo);
  
  glGenRenderbuffers(1, &hdr_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, hdr_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F, viewport_width, viewport_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ldr_buffer);   
  
  glGenTextures(1, &hdr_texture);
  glBindTexture(GL_TEXTURE_2D, hdr_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, viewport_width, viewport_height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdr_texture, 0);
  
  glGenFramebuffers(1, &ldr_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, ldr_fbo);
  
  glGenRenderbuffers(1, &ldr_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, ldr_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, viewport_width, viewport_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ldr_buffer);   
  
  glGenTextures(1, &ldr_texture);
  glBindTexture(GL_TEXTURE_2D, ldr_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport_width, viewport_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ldr_texture, 0);
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
}

void deferred_renderer_finish() {
  
  glDeleteFramebuffers(1, &fbo);
  
  glDeleteRenderbuffers(1, &diffuse_buffer);
  glDeleteRenderbuffers(1, &positions_buffer);
  glDeleteRenderbuffers(1, &normals_buffer);
  glDeleteRenderbuffers(1, &depth_buffer);
  
  glDeleteTextures(1,&diffuse_texture);
  glDeleteTextures(1,&positions_texture);
  glDeleteTextures(1,&normals_texture);
  glDeleteTextures(1,&depth_texture);
  
  glDeleteFramebuffers(1, &ssao_fbo);
  glDeleteRenderbuffers(1, &ssao_buffer);
  glDeleteTextures(1,&ssao_texture);
  
  glDeleteFramebuffers(1, &hdr_fbo);
  glDeleteRenderbuffers(1, &hdr_buffer);
  glDeleteTextures(1,&hdr_texture);
  
  glDeleteFramebuffers(1, &ldr_fbo);
  glDeleteRenderbuffers(1, &ldr_buffer);
  glDeleteTextures(1,&ldr_texture);
  
  folder_unload(P("$CORANGE/shaders/deferred/"));
  
}

void deferred_renderer_add_light(light* l) {
  
  if (num_lights == DEFERRED_MAX_LIGHTS) {
    warning("Cannot add extra light. Maxiumum lights reached!");
    return;
  }
  
  lights[num_lights] = l;
  num_lights++;
}

void deferred_renderer_remove_light(light* l) {
  
  bool found = false;
  for(int i = 0; i < num_lights; i++) {
    if ((lights[i] == l) && !found) {
      found = true;
    }
    
    if (found) {
      lights[i] = lights[i+1];
    }
  }
  
  if (found) {
    num_lights--;
  } else {
    warning("Could not find light %p to remove!", l);
  }
  
}

void deferred_renderer_set_camera(camera* cam) {
  CAMERA = cam;
}

void deferred_renderer_set_shadow_texture(texture* t) {
  SHADOW_TEX = t;
}

void deferred_renderer_set_color_correction(asset_hndl ah) {
  COLOR_CORRECTION = ah;
}

void deferred_renderer_set_shadow_light(light* l) {
  SHADOW_LIGHT = l;
}

static void deferred_renderer_use_material_entry(material_entry* me, shader_program* PROG) {
  
  /* Set material parameters */
  
  GLint world_matrix_u = glGetUniformLocation(shader_program_handle(PROG), "world_matrix");
  glUniformMatrix4fv(world_matrix_u, 1, 0, WORLD_MATRIX);

  GLint proj_matrix_u = glGetUniformLocation(shader_program_handle(PROG), "proj_matrix");
  glUniformMatrix4fv(proj_matrix_u, 1, 0, PROJ_MATRIX);
  
  GLint view_matrix_u = glGetUniformLocation(shader_program_handle(PROG), "view_matrix");
  glUniformMatrix4fv(view_matrix_u, 1, 0, VIEW_MATRIX);
  
  int tex_counter = 0;
  
  for(int i = 0; i < me->num_items; i++) {
    char* key = me->names[i];
    int type = me->types[i];
    material_item val = me->items[i];
    
    GLint loc = glGetUniformLocation(shader_program_handle(PROG), key);
    
    if (type == mat_item_texture) {
      glUniform1i(loc, tex_counter);
      glActiveTexture(GL_TEXTURE0 + tex_counter);
      glBindTexture(GL_TEXTURE_2D, texture_handle(asset_hndl_ptr(val.as_asset)));
      tex_counter++;
    }
    if (type == mat_item_int)   { glUniform1i(loc, val.as_int); }
    if (type == mat_item_float) { glUniform1f(loc, val.as_float); }
    if (type == mat_item_vec2)  { glUniform2f(loc, val.as_vec2.x, val.as_vec2.y); }
    if (type == mat_item_vec3)  { glUniform3f(loc, val.as_vec3.x, val.as_vec3.y, val.as_vec3.z); }
    if (type == mat_item_vec4)  { glUniform4f(loc, val.as_vec4.x, val.as_vec4.y, val.as_vec4.z, val.as_vec4.w); }
  }  

}

static void deferred_renderer_setup_camera() {

  mat4 viewm = camera_view_matrix(CAMERA);
  mat4 projm = camera_proj_matrix(CAMERA, graphics_viewport_ratio() );
  
  mat4_to_array(viewm, VIEW_MATRIX);
  mat4_to_array(projm, PROJ_MATRIX);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(VIEW_MATRIX);
  
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(PROJ_MATRIX);
  
  mat4 lviewm = light_view_matrix(SHADOW_LIGHT);
  mat4 lprojm = light_proj_matrix(SHADOW_LIGHT);
  
  mat4_to_array(lviewm, LIGHT_VIEW_MATRIX);
  mat4_to_array(lprojm, LIGHT_PROJ_MATRIX);

}

void deferred_renderer_begin() {

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  
  deferred_renderer_setup_camera();
  
  GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
  glDrawBuffers(3, buffers);
  
  glClearColor(0.2, 0.2, 0.2, 1.0f);
  glClearDepth(1.0f);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  shader_program* program_clear = material_get_entry(asset_hndl_ptr(MAT_CLEAR), 0)->program;
  
  glUseProgram(shader_program_handle(program_clear));
  
  GLint start = glGetUniformLocation(shader_program_handle(program_clear), "start");
  GLint end = glGetUniformLocation(shader_program_handle(program_clear), "end");
  glUniform4f(start, 0.5, 0.5, 0.5, 1.0);
  glUniform4f(end, 0.0, 0.0, 0.0, 1.0);
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
	glBegin(GL_QUADS);
		glVertex3f(-1.0, -1.0,  0.0f);
		glVertex3f(1.0, -1.0,  0.0f);
		glVertex3f(1.0,  1.0,  0.0f);
		glVertex3f(-1.0,  1.0,  0.0f);
	glEnd();
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
  glUseProgram(0);
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  
}

void deferred_renderer_end() {
  
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  
  /* Render out ssao */
  
  glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
  
  glViewport(0, 0, graphics_viewport_width() / 2, graphics_viewport_height() / 2);
  
  shader_program* program_ssao = material_get_entry(asset_hndl_ptr(MAT_SSAO), 0)->program;
  
  GLuint ssao_handle = shader_program_handle(program_ssao);
  glUseProgram(ssao_handle);
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(ssao_handle, "depth_texture"), 0);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glBindTexture(GL_TEXTURE_2D, texture_handle(asset_hndl_ptr(RANDOM)));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(ssao_handle, "random_texture"), 1);
  
  float seed = CAMERA->position.x + CAMERA->position.y + CAMERA->position.z;
  glUniform1f(glGetUniformLocation(ssao_handle, "seed"), seed);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0,  1.0,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0,  1.0,  0.0f);
	glEnd();
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
  /* End */
  
  /* Render full screen quad to hdr fbo */
  
  glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo);
  
  glViewport(0, 0, graphics_viewport_width(), graphics_viewport_height());
  
  shader_program* program_compose = material_get_entry(asset_hndl_ptr(MAT_COMPOSE), 0)->program;
  
  GLuint screen_handle = shader_program_handle(program_compose);
  glUseProgram(screen_handle);
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, diffuse_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(screen_handle, "diffuse_texture"), 0);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glBindTexture(GL_TEXTURE_2D, positions_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(screen_handle, "positions_texture"), 1);
  
  glActiveTexture(GL_TEXTURE0 + 2 );
  glBindTexture(GL_TEXTURE_2D, normals_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(screen_handle, "normals_texture"), 2);
  
  glActiveTexture(GL_TEXTURE0 + 3 );
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(screen_handle, "depth_texture"), 3);
  
  glActiveTexture(GL_TEXTURE0 + 4 );
  glBindTexture(GL_TEXTURE_2D, texture_handle(SHADOW_TEX));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(screen_handle, "shadows_texture"), 4);
  
  glActiveTexture(GL_TEXTURE0 + 5 );
  glBindTexture(GL_TEXTURE_2D, ssao_texture);
  glEnable(GL_TEXTURE_2D);
  glGenerateMipmap(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(screen_handle, "ssao_texture"), 5);
  
  glActiveTexture(GL_TEXTURE0 + 6 );
  glBindTexture(GL_TEXTURE_2D, texture_handle(asset_hndl_ptr(ENVIRONMENT)));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(screen_handle, "env_texture"), 6);
  
  GLint cam_position = glGetUniformLocation(screen_handle, "camera_position");
  glUniform3f(cam_position, CAMERA->position.x, CAMERA->position.y, CAMERA->position.z);
  
  GLint lproj_matrix_u = glGetUniformLocation(screen_handle, "light_proj");
  glUniformMatrix4fv(lproj_matrix_u, 1, 0, LIGHT_PROJ_MATRIX);
  
  GLint lview_matrix_u = glGetUniformLocation(screen_handle, "light_view");
  glUniformMatrix4fv(lview_matrix_u, 1, 0, LIGHT_VIEW_MATRIX);
  
  for(int i = 0; i < num_lights; i++) {
    light_power[i] = lights[i]->power;
    light_falloff[i] = lights[i]->falloff;
    light_position[i] = lights[i]->position;
    light_target[i] = lights[i]->target;
    light_diffuse[i] = lights[i]->diffuse_color;
    light_ambient[i] = lights[i]->ambient_color;
    light_specular[i] = lights[i]->specular_color;
  }
  
  glUniform1i(glGetUniformLocation(screen_handle, "num_lights"), num_lights);
  
  GLint light_power_u = glGetUniformLocation(screen_handle, "light_power");
  GLint light_falloff_u = glGetUniformLocation(screen_handle, "light_falloff");
  GLint light_position_u = glGetUniformLocation(screen_handle, "light_position");
  GLint light_target_u = glGetUniformLocation(screen_handle, "light_target");
  GLint light_diffuse_u = glGetUniformLocation(screen_handle, "light_diffuse");
  GLint light_ambient_u = glGetUniformLocation(screen_handle, "light_ambient");
  GLint light_specular_u = glGetUniformLocation(screen_handle, "light_specular");
  
  glUniform1fv(light_power_u, num_lights, (const GLfloat*)light_power);
  glUniform1fv(light_falloff_u, num_lights, (const GLfloat*)light_falloff);
  glUniform3fv(light_position_u, num_lights, (const GLfloat*)light_position);
  glUniform3fv(light_target_u, num_lights, (const GLfloat*)light_target);
  glUniform3fv(light_diffuse_u, num_lights, (const GLfloat*)light_diffuse);
  glUniform3fv(light_ambient_u, num_lights, (const GLfloat*)light_ambient);
  glUniform3fv(light_specular_u, num_lights, (const GLfloat*)light_specular);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0,  1.0,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0,  1.0,  0.0f);
	glEnd();
  
  glActiveTexture(GL_TEXTURE0 + 6 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 5 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 4 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 3 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 2 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
  glUseProgram(0);

  /* Render HDR to LDR buffer */
  
  glBindFramebuffer(GL_FRAMEBUFFER, ldr_fbo);
  
  shader_program* program_tonemap = material_get_entry(asset_hndl_ptr(MAT_TONEMAP), 0)->program;
  
  GLuint screen_tonemap_handle = shader_program_handle(program_tonemap);
  glUseProgram(screen_tonemap_handle);
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, hdr_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(screen_tonemap_handle, "hdr_texture"), 0);

  glUniform1f(glGetUniformLocation(screen_tonemap_handle, "exposure"), EXPOSURE);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0,  1.0,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0,  1.0,  0.0f);
	glEnd();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
  glUseProgram(0);
  
  /* Generate Mipmaps, adjust exposure */
  
  unsigned char color[4] = {0,0,0,0};
  int level = -1;
  int width = 0;
  int height = 0;
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, ldr_texture);
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
  
  EXPOSURE += (EXPOSURE_TARGET - average) * EXPOSURE_SPEED;
  
  /* Render final frame */
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  shader_program* program_post = material_get_entry(asset_hndl_ptr(MAT_POST), 0)->program;
  
  GLuint screen_post_handle = shader_program_handle(program_post);
  glUseProgram(screen_post_handle);
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, ldr_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(screen_post_handle, "diffuse_texture"), 0);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glBindTexture(GL_TEXTURE_2D, texture_handle(asset_hndl_ptr(VIGNETTING)));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(screen_post_handle, "vignetting_texture"), 1);
  
  glActiveTexture(GL_TEXTURE0 + 2 );
  glBindTexture(GL_TEXTURE_3D, texture_handle(asset_hndl_ptr(COLOR_CORRECTION)));
  glEnable(GL_TEXTURE_3D);
  glUniform1i(glGetUniformLocation(screen_post_handle, "lut"), 2);
  
  glUniform1i(glGetUniformLocation(screen_post_handle, "width"), graphics_viewport_width());
  glUniform1i(glGetUniformLocation(screen_post_handle, "height"), graphics_viewport_height());
  glUniform1i(glGetUniformLocation(screen_post_handle, "aa_type"), 1);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0,  1.0,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0,  1.0,  0.0f);
	glEnd();
  
  glActiveTexture(GL_TEXTURE0 + 2 );
  glDisable(GL_TEXTURE_3D);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
  glUseProgram(0);
  
}

float deferred_renderer_get_exposure() {
  return EXPOSURE;
}

void deferred_renderer_set_exposure(float exposure) {
  EXPOSURE = exposure;
}

void deferred_renderer_render_static(static_object* so) {

  mat4 r_world_matrix = mat4_world( so->position, so->scale, so->rotation );
  mat4_to_array(r_world_matrix, WORLD_MATRIX);
  
  renderable* r = asset_hndl_ptr(so->renderable);
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    if(s->is_rigged) {
      error("Renderable for static object is rigged!");
    }
    
    shader_program* program_static = material_get_entry(asset_hndl_ptr(MAT_STATIC), 0)->program;
    
    GLuint program_handle = shader_program_handle(program_static);
    glUseProgram(program_handle);
    
    deferred_renderer_use_material_entry(material_get_entry(asset_hndl_ptr(s->material), 0), program_static);
    
    GLsizei stride = sizeof(float) * 18;
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
        
    glVertexPointer(3, GL_FLOAT, stride, (void*)0);
    glEnableClientState(GL_VERTEX_ARRAY);
    
    glVertexAttribPointer(NORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(NORMAL);
    
    glVertexAttribPointer(TANGENT, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
    glEnableVertexAttribArray(TANGENT);
    
    glVertexAttribPointer(BINORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 9));
    glEnableVertexAttribArray(BINORMAL);
    
    glTexCoordPointer(2, GL_FLOAT, stride, (void*)(sizeof(float) * 12));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);  
    
    glDisableVertexAttribArray(NORMAL);
    glDisableVertexAttribArray(TANGENT);
    glDisableVertexAttribArray(BINORMAL);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glUseProgram(0);

  }
  
}

#define MAX_BONES 32
static mat4 bone_matrices[MAX_BONES];
static float bone_matrix_data[4 * 4 * MAX_BONES];

void deferred_renderer_render_animated(animated_object* ao) {

  skeleton* skel = asset_hndl_ptr(ao->skeleton);
  skeleton* pose = ao->pose;

  if (skel->num_bones > MAX_BONES) {
    error("animated object skeleton has too many bones (over %i)", MAX_BONES);
  }

  mat4 r_world_matrix = mat4_world( ao->position, ao->scale, ao->rotation );
  mat4_to_array(r_world_matrix, WORLD_MATRIX);
  
  skeleton_gen_transforms(pose);
  
  for(int i = 0; i < skel->num_bones; i++) {
    mat4 base, ani;
    base = skel->inv_transforms[i];
    ani = pose->transforms[i];
    
    bone_matrices[i] = mat4_mul_mat4(ani, base);
    mat4_to_array(bone_matrices[i], bone_matrix_data + (i * 4 * 4));
  }
  
  renderable* r = asset_hndl_ptr(ao->renderable);
  
  for(int i = 0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    if(s->is_rigged) {
      
      shader_program* program_animated = material_get_entry(asset_hndl_ptr(MAT_ANIMATED), 0)->program;
      
      GLuint program_animated_handle = shader_program_handle(program_animated);
      glUseProgram(program_animated_handle);
      
      deferred_renderer_use_material_entry(material_get_entry(asset_hndl_ptr(s->material),0), program_animated);
      
      GLint bone_world_matrices_u = glGetUniformLocation(program_animated_handle, "bone_world_matrices");
      glUniformMatrix4fv(bone_world_matrices_u, skel->num_bones, GL_FALSE, bone_matrix_data);
      
      GLint bone_count_u = glGetUniformLocation(program_animated_handle, "bone_count");
      glUniform1i(bone_count_u, skel->num_bones);
      
      GLsizei stride = sizeof(float) * 24;
      
      glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
          
      glVertexPointer(3, GL_FLOAT, stride, (void*)0);
      glEnableClientState(GL_VERTEX_ARRAY);
      
      glVertexAttribPointer(NORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
      glEnableVertexAttribArray(NORMAL);
      
      glVertexAttribPointer(TANGENT, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
      glEnableVertexAttribArray(TANGENT);
      
      glVertexAttribPointer(BINORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 9));
      glEnableVertexAttribArray(BINORMAL);
      
      glTexCoordPointer(2, GL_FLOAT, stride, (void*)(sizeof(float) * 12));
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      
      glVertexAttribPointer(BONE_INDICIES, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 18));
      glEnableVertexAttribArray(BONE_INDICIES);
      
      glVertexAttribPointer(BONE_WEIGHTS, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 21));
      glEnableVertexAttribArray(BONE_WEIGHTS);
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
      
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);  
      
      glDisableVertexAttribArray(NORMAL);
      glDisableVertexAttribArray(TANGENT);
      glDisableVertexAttribArray(BINORMAL);
      glDisableVertexAttribArray(BONE_INDICIES);  
      glDisableVertexAttribArray(BONE_WEIGHTS);  
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      
      glUseProgram(0);
      
    } else {
    
      error("Animated object is not rigged!");
    
    }

  }

}

void deferred_renderer_render_light(light* l) {
  
  mat4 viewm = camera_view_matrix(CAMERA);
  mat4 projm = camera_proj_matrix(CAMERA, graphics_viewport_ratio() );
  
  vec4 light_pos = vec4_new(l->position.x, l->position.y, l->position.z, 1);
  light_pos = mat4_mul_vec4(viewm, light_pos);
  light_pos = mat4_mul_vec4(projm, light_pos);
  
  light_pos = vec4_div(light_pos, light_pos.w);
  
  shader_program* program_ui = material_get_entry(asset_hndl_ptr(MAT_UI), 0)->program;
  
  glUseProgram(shader_program_handle(program_ui));
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(0, graphics_viewport_width(), graphics_viewport_height(), 0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  float top = ((-light_pos.y + 1) / 2) * graphics_viewport_height() - 8;
  float bot = ((-light_pos.y + 1) / 2) * graphics_viewport_height() + 8;
  float left = ((light_pos.x + 1) / 2) * graphics_viewport_width() - 8;
  float right = ((light_pos.x + 1) / 2) * graphics_viewport_width() + 8;
  
  texture* lightbulb = asset_hndl_ptr(asset_hndl_new(P("$CORANGE/ui/lightbulb.dds")));
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, texture_handle(lightbulb));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(shader_program_handle(program_ui), "diffuse"), 0);
  
  glUniform1f(glGetUniformLocation(shader_program_handle(program_ui), "alpha_test"), 0.5);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(left, top, -light_pos.z);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(left,  bot, -light_pos.z);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(right,  bot, -light_pos.z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(right, top, -light_pos.z);
	glEnd();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glUseProgram(0);

}

void deferred_renderer_render_axis(mat4 world) {

  vec4 x_pos = mat4_mul_vec4(world, vec4_new(2,0,0,1));
  vec4 y_pos = mat4_mul_vec4(world, vec4_new(0,2,0,1));
  vec4 z_pos = mat4_mul_vec4(world, vec4_new(0,0,2,1));
  vec4 base_pos = mat4_mul_vec4(world, vec4_new(0,0,0,1));
  
  x_pos = vec4_div(x_pos, x_pos.w);
  y_pos = vec4_div(y_pos, y_pos.w);
  z_pos = vec4_div(z_pos, z_pos.w);
  base_pos = vec4_div(base_pos, base_pos.w);
  
  glDisable(GL_DEPTH_TEST);
  
  glLineWidth(2.0);
  glBegin(GL_LINES);
    glColor3f(1.0,0.0,0.0);
    glVertex3f(x_pos.x, x_pos.y, x_pos.z);
    glVertex3f(base_pos.x, base_pos.y, base_pos.z);
    glColor3f(0.0,1.0,0.0);
    glVertex3f(y_pos.x, y_pos.y, y_pos.z);
    glVertex3f(base_pos.x, base_pos.y, base_pos.z);
    glColor3f(0.0,0.0,1.0);
    glVertex3f(z_pos.x, z_pos.y, z_pos.z);
    glVertex3f(base_pos.x, base_pos.y, base_pos.z);
  glEnd();
  glLineWidth(1.0);
  
  glPointSize(5.0);
  glBegin(GL_POINTS);
    glColor3f(1.0,0.0,0.0);
    glVertex3f(x_pos.x, x_pos.y, x_pos.z);
    glColor3f(0.0,1.0,0.0);
    glVertex3f(y_pos.x, y_pos.y, y_pos.z);
    glColor3f(0.0,0.0,1.0);
    glVertex3f(z_pos.x, z_pos.y, z_pos.z);
    glColor3f(1.0,1.0,1.0);
    glVertex3f(base_pos.x, base_pos.y, base_pos.z);
  glEnd();
  glPointSize(1.0);
  
  glEnable(GL_DEPTH_TEST);

}


