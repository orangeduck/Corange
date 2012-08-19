#include "rendering/forward_renderer.h"

#include "graphics_manager.h"
#include "asset_manager.h"

#include "assets/material.h"
#include "assets/shader.h"
#include "assets/renderable.h"
#include "assets/terrain.h"

static bool use_shadows;

static camera* CAMERA = NULL;
static light* SHADOW_LIGHT = NULL;

static texture* SHADOW_TEX = NULL;
static texture* DEPTH_TEX = NULL;

static asset_hndl COLOR_CORRECTION;
static asset_hndl VIGNETTING;

static asset_hndl GRADIENT;
static asset_hndl SCREEN_TONEMAP;
static asset_hndl SCREEN_POST;

static float proj_matrix[16];
static float view_matrix[16];
static float world_matrix[16];
static float lview_matrix[16];
static float lproj_matrix[16];

static float world_matricies[16*MAX_INSTANCES];

static float shader_timer = 0.0;

static int ATTR_POSITION;
static int ATTR_NORMAL;
static int ATTR_TEXCOORD;
static int ATTR_COLOR;

static int ATTR_TANGENT;
static int ATTR_BINORMAL;

static int ATTR_BONE_INDICIES;
static int ATTR_BONE_WEIGHTS;

static GLuint hdr_fbo;
static GLuint hdr_buffer;
static GLuint hdr_depth_buffer;

static GLuint hdr_res_fbo;
static GLuint hdr_res_texture;

static GLuint ldr_fbo;
static GLuint ldr_buffer;
static GLuint ldr_texture;

#define FORWARD_MAX_LIGHTS 32

static int num_lights;

static light* lights[FORWARD_MAX_LIGHTS];
static float light_power[FORWARD_MAX_LIGHTS];
static float light_falloff[FORWARD_MAX_LIGHTS];
static vec3 light_position[FORWARD_MAX_LIGHTS];
static vec3 light_target[FORWARD_MAX_LIGHTS];
static vec3 light_diffuse[FORWARD_MAX_LIGHTS];
static vec3 light_ambient[FORWARD_MAX_LIGHTS];
static vec3 light_specular[FORWARD_MAX_LIGHTS];

static float EXPOSURE;
static float EXPOSURE_SPEED;
static float EXPOSURE_TARGET;

static int MULTISAMPLES;

void forward_renderer_init() {
  
  MULTISAMPLES = graphics_get_multisamples();
  
  use_shadows = false;
  num_lights = 0;
  
  EXPOSURE = 0.0;
  EXPOSURE_SPEED = 1.0;
  EXPOSURE_TARGET = 0.4;
  
  COLOR_CORRECTION = asset_hndl_new(P("$CORANGE/resources/identity.lut"));
  VIGNETTING = asset_hndl_new(P("$CORANGE/resources/vignetting.dds"));
  
  GRADIENT = asset_hndl_new(P("$CORANGE/shaders/gradient.mat"));
  
  folder_load(P("$CORANGE/shaders/forward/"));
  
  SCREEN_TONEMAP = asset_hndl_new(P("$CORANGE/shaders/forward/tonemap.mat"));
  SCREEN_POST = asset_hndl_new(P("$CORANGE/shaders/forward/post.mat"));
  
  glClearColor(0.2, 0.2, 0.2, 1.0f);
  glClearDepth(1.0f);
  
  glGenFramebuffers(1, &hdr_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo);
  
  glGenRenderbuffers(1, &hdr_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, hdr_buffer);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, MULTISAMPLES, GL_RGBA16F, graphics_viewport_width(), graphics_viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, hdr_buffer);   
  
  glGenRenderbuffers(1, &hdr_depth_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, hdr_depth_buffer);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, MULTISAMPLES, GL_DEPTH_COMPONENT24, graphics_viewport_width(), graphics_viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, hdr_depth_buffer);  

  glGenFramebuffers(1, &hdr_res_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, hdr_res_fbo);
  
  glGenTextures(1, &hdr_res_texture);
  glBindTexture(GL_TEXTURE_2D, hdr_res_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, graphics_viewport_width(), graphics_viewport_height(), 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdr_res_texture, 0);
  
  glGenFramebuffers(1, &ldr_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, ldr_fbo);
  
  glGenRenderbuffers(1, &ldr_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, ldr_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, graphics_viewport_width(), graphics_viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ldr_buffer);   
  
  glGenTextures(1, &ldr_texture);
  glBindTexture(GL_TEXTURE_2D, ldr_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, graphics_viewport_width(), graphics_viewport_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ldr_texture, 0);
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
}

void forward_renderer_finish() {  
  
  glDeleteFramebuffers(1, &hdr_fbo);
  glDeleteRenderbuffers(1, &hdr_buffer);
  glDeleteRenderbuffers(1, &hdr_depth_buffer);
  
  glDeleteFramebuffers(1, &hdr_res_fbo);
  glDeleteTextures(1,&hdr_res_texture);
  
  glDeleteFramebuffers(1, &ldr_fbo);
  glDeleteRenderbuffers(1, &ldr_buffer);
  glDeleteTextures(1,&ldr_texture);
  
  folder_unload(P("$CORANGE/shaders/forward/"));
  
}

void forward_renderer_add_light(light* l) {
  
  if (num_lights == FORWARD_MAX_LIGHTS) {
    warning("Cannot add extra light. Maxiumum lights reached!");
    return;
  }
  
  lights[num_lights] = l;
  num_lights++;
}

void forward_renderer_remove_light(light* l) {
  
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

void forward_renderer_set_camera(camera* c) {
  CAMERA = c;
}

void forward_renderer_set_shadow_light(light* l) {
  SHADOW_LIGHT = l;
}

void forward_renderer_set_shadow_texture(texture* t) {
  
  if ( t == NULL) {
    use_shadows = false;
  } else {
    use_shadows = true;
    SHADOW_TEX = t;
  }
  
}

void forward_renderer_set_depth_texture(texture* t) {
  DEPTH_TEX = t;
}


void forward_renderer_set_color_correction(asset_hndl ah) {
  COLOR_CORRECTION = ah;
}

static void render_gradient() {

  shader_program* gradient_prog = material_get_entry(asset_hndl_ptr(GRADIENT), 0)->program;

  GLuint gradient_handle = shader_program_handle(gradient_prog);
  glUseProgram(gradient_handle);
  
  GLint start = glGetUniformLocation(gradient_handle, "start");
  GLint end = glGetUniformLocation(gradient_handle, "end");
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

}

void forward_renderer_begin() {
  
  SDL_GL_CheckError();
  
  shader_timer += frame_time();
  
  glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo);
  
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
  render_gradient();
  
  if (CAMERA == NULL) {
    error("Camera not set yet!");
  }
  if (SHADOW_LIGHT == NULL) {
    error("Light not set yet!");
  }

  mat4 viewm = camera_view_matrix(CAMERA);
  mat4 projm = camera_proj_matrix(CAMERA, graphics_viewport_ratio() );
  
  mat4_to_array(viewm, view_matrix);
  mat4_to_array(projm, proj_matrix);

  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(view_matrix);
  
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(proj_matrix);
  
  /* Setup light stuff */
  
  mat4 lviewm = light_view_matrix(SHADOW_LIGHT);
  mat4 lprojm = light_proj_matrix(SHADOW_LIGHT);
  
  mat4_to_array(lviewm, lview_matrix);
  mat4_to_array(lprojm, lproj_matrix);
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  
  SDL_GL_CheckError();
  
}

void forward_renderer_end() {
  
  SDL_GL_CheckError();
  
  /* Resolve multisamples */
  
  glBindFramebuffer(GL_READ_FRAMEBUFFER, hdr_fbo);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdr_res_fbo);
  glBlitFramebuffer(0, 0, graphics_viewport_width(), graphics_viewport_height(), 0, 0, graphics_viewport_width(), graphics_viewport_height(), GL_COLOR_BUFFER_BIT, GL_LINEAR);
  
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  
  /* Render HDR to LDR buffer */
  
  glBindFramebuffer(GL_FRAMEBUFFER, ldr_fbo);
  
  shader_program* tonemap_prog = material_get_entry(asset_hndl_ptr(SCREEN_TONEMAP), 0)->program;
  
  GLuint tonemap_handle = shader_program_handle(tonemap_prog);
  glUseProgram(tonemap_handle);
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, hdr_res_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(tonemap_handle, "hdr_texture"), 0);

  glUniform1f(glGetUniformLocation(tonemap_handle, "exposure"), EXPOSURE);
  
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
  
  //EXPOSURE += (EXPOSURE_TARGET - average) * EXPOSURE_SPEED;
  EXPOSURE = 5.0;
  
  /* Render final frame */
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  shader_program* post_prog = material_get_entry(asset_hndl_ptr(SCREEN_POST), 0)->program;
  
  GLuint post_handle = shader_program_handle(post_prog);
  glUseProgram(post_handle);
  
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
  glUniform1i(glGetUniformLocation(post_handle, "diffuse_texture"), 0);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glBindTexture(GL_TEXTURE_2D, texture_handle(asset_hndl_ptr(VIGNETTING)));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(post_handle, "vignetting_texture"), 1);
  
  glActiveTexture(GL_TEXTURE0 + 2 );
  glBindTexture(GL_TEXTURE_3D, texture_handle(asset_hndl_ptr(COLOR_CORRECTION)));
  glEnable(GL_TEXTURE_3D);
  glUniform1i(glGetUniformLocation(post_handle, "lut"), 2);
  
  glUniform1i(glGetUniformLocation(post_handle, "width"), graphics_viewport_width());
  glUniform1i(glGetUniformLocation(post_handle, "height"), graphics_viewport_height());
  
  if (MULTISAMPLES == 0) {
    glUniform1i(glGetUniformLocation(post_handle, "aa_type"), 1);
  } else {
    glUniform1i(glGetUniformLocation(post_handle, "aa_type"), 0);
  }
  
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
  
  SDL_GL_CheckError();
  
}

float forward_renderer_get_exposure() {
  return EXPOSURE;
}

void forward_renderer_set_exposure(float exposure) {
  EXPOSURE = exposure;
}

static int tex_counter = 0;
static void forward_renderer_use_material(material* mat) {
  
  shader_program* prog = material_get_entry(mat, 0)->program;
  GLuint prog_handle = shader_program_handle(prog);
  
  glUseProgram(prog_handle);
  SDL_GL_CheckError();
  
  /* Set global parameters */
  
  ATTR_POSITION = glGetAttribLocation(prog_handle, "position");
  ATTR_NORMAL = glGetAttribLocation(prog_handle, "normal");
  ATTR_TEXCOORD = glGetAttribLocation(prog_handle, "texcoord");
  ATTR_COLOR = glGetAttribLocation(prog_handle, "color");
  
  ATTR_TANGENT = glGetAttribLocation(prog_handle, "tangent");
  ATTR_BINORMAL = glGetAttribLocation(prog_handle, "binormal");
  
  ATTR_BONE_INDICIES = glGetAttribLocation(prog_handle, "bone_indicies");
  ATTR_BONE_WEIGHTS = glGetAttribLocation(prog_handle, "bone_weights");

  GLint camera_position = glGetUniformLocation(prog_handle, "camera_position");
  if (camera_position != -1) {
    glUniform3f(camera_position, CAMERA->position.x, CAMERA->position.y, CAMERA->position.z);
  }
  
  GLint camera_direction = glGetUniformLocation(prog_handle, "camera_direction");
  if (camera_direction != -1) {
    vec3 direction = vec3_normalize(vec3_sub(CAMERA->target, CAMERA->position));
    glUniform3f(camera_direction, direction.x, direction.y, direction.z);
  }
  
  SDL_GL_CheckError();
  
  for(int i = 0; i < num_lights; i++) {
    light_power[i] = lights[i]->power;
    light_falloff[i] = lights[i]->falloff;
    light_position[i] = lights[i]->position;
    light_target[i] = lights[i]->target;
    light_diffuse[i] = lights[i]->diffuse_color;
    light_ambient[i] = lights[i]->ambient_color;
    light_specular[i] = lights[i]->specular_color;
  }
  
  SDL_GL_CheckError();
  
  glUniform1i(glGetUniformLocation(prog_handle, "num_lights"), num_lights);
  
  GLint light_power_u = glGetUniformLocation(prog_handle, "light_power");
  GLint light_falloff_u = glGetUniformLocation(prog_handle, "light_falloff");
  GLint light_position_u = glGetUniformLocation(prog_handle, "light_position");
  GLint light_target_u = glGetUniformLocation(prog_handle, "light_target");
  GLint light_diffuse_u = glGetUniformLocation(prog_handle, "light_diffuse");
  GLint light_ambient_u = glGetUniformLocation(prog_handle, "light_ambient");
  GLint light_specular_u = glGetUniformLocation(prog_handle, "light_specular");
  
  glUniform1fv(light_power_u, num_lights, (const GLfloat*)light_power);
  glUniform1fv(light_falloff_u, num_lights, (const GLfloat*)light_falloff);
  glUniform3fv(light_position_u, num_lights, (const GLfloat*)light_position);
  glUniform3fv(light_target_u, num_lights, (const GLfloat*)light_target);
  glUniform3fv(light_diffuse_u, num_lights, (const GLfloat*)light_diffuse);
  glUniform3fv(light_ambient_u, num_lights, (const GLfloat*)light_ambient);
  glUniform3fv(light_specular_u, num_lights, (const GLfloat*)light_specular);

  SDL_GL_CheckError();
  
  GLint time = glGetUniformLocation(prog_handle, "time");
  glUniform1f(time, shader_timer);
  
  GLint world_matrix_u = glGetUniformLocation(prog_handle, "world_matrix");
  glUniformMatrix4fv(world_matrix_u, 1, 0, world_matrix);
  
  GLint proj_matrix_u = glGetUniformLocation(prog_handle, "proj_matrix");
  glUniformMatrix4fv(proj_matrix_u, 1, 0, proj_matrix);
  
  GLint view_matrix_u = glGetUniformLocation(prog_handle, "view_matrix");
  glUniformMatrix4fv(view_matrix_u, 1, 0, view_matrix);
  
  GLint lproj_matrix_u = glGetUniformLocation(prog_handle, "light_proj");
  if (lproj_matrix_u != -1) {
    glUniformMatrix4fv(lproj_matrix_u, 1, 0, lproj_matrix);
  }
  
  GLint lview_matrix_u = glGetUniformLocation(prog_handle, "light_view");
  if (lproj_matrix_u != -1) {
    glUniformMatrix4fv(lview_matrix_u, 1, 0, lview_matrix);
  }
  
  GLint world_matricies_u = glGetUniformLocation(prog_handle, "world_matricies");
  if (world_matricies_u != -1) {
    glUniformMatrix4fv(world_matricies_u, MAX_INSTANCES, 0, world_matricies);
  }
  
  SDL_GL_CheckError();
  
  /* Set material parameters */
  
  tex_counter = 0;
  
  material_entry* me = material_get_entry(mat, 0);
  
  for(int i = 0; i < me->num_items; i++) {
    char* key = me->names[i];
    int type = me->types[i];
    material_item val = me->items[i];
    
    GLint loc = glGetUniformLocation(prog_handle, key);
    
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
     
    SDL_GL_CheckError();
     
  }
  
  GLint shadow_map = glGetUniformLocation(prog_handle, "shadow_map");
  if ( use_shadows && (shadow_map != -1) ) {
    glUniform1i(shadow_map, tex_counter);
    glActiveTexture(GL_TEXTURE0 + tex_counter);
    glBindTexture(GL_TEXTURE_2D, texture_handle(SHADOW_TEX));
    glEnable(GL_TEXTURE_2D);
    tex_counter++;
  }
  
  SDL_GL_CheckError();
  
}

static void forward_renderer_disuse_material() {
  
  while(tex_counter > 0) {
    tex_counter--;
    glActiveTexture(GL_TEXTURE0 + tex_counter);
    glDisable(GL_TEXTURE_2D);
  }
  
  glUseProgram(0);

}

static void bind_attributes_static() {

  GLsizei stride = sizeof(float) * 18;
  
  glVertexPointer(3, GL_FLOAT, stride, (void*)0);
  glEnableClientState(GL_VERTEX_ARRAY);
  
  glNormalPointer(GL_FLOAT, stride, (void*)(sizeof(float) * 3));
  glEnableClientState(GL_NORMAL_ARRAY);
  
  glTexCoordPointer(2, GL_FLOAT, stride, (void*)(sizeof(float) * 12));
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  
  glColorPointer(4, GL_FLOAT, stride, (void*)(sizeof(float) * 14));
  glEnableClientState(GL_COLOR_ARRAY);
  
  if (ATTR_POSITION != -1) {
    glVertexAttribPointer(ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(ATTR_POSITION);
  }
  
  if (ATTR_NORMAL != -1) {
    glVertexAttribPointer(ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(ATTR_NORMAL);
  }
  
  if (ATTR_TEXCOORD != -1) {
    glVertexAttribPointer(ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 12));
    glEnableVertexAttribArray(ATTR_TEXCOORD);
  }
  
  if (ATTR_COLOR != -1) {
    glVertexAttribPointer(ATTR_COLOR, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 14));
    glEnableVertexAttribArray(ATTR_COLOR);
  }
  
  if (ATTR_TANGENT != -1) {
    glVertexAttribPointer(ATTR_TANGENT, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
    glEnableVertexAttribArray(ATTR_TANGENT);
  }
  
  if (ATTR_BINORMAL != -1) {
    glVertexAttribPointer(ATTR_BINORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 9));
    glEnableVertexAttribArray(ATTR_BINORMAL);
  }

}

static void unbind_attributes_static() {

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);  
  glDisableClientState(GL_COLOR_ARRAY);  
  
  if (ATTR_POSITION != -1) { glDisableVertexAttribArray(ATTR_POSITION); }
  if (ATTR_NORMAL != -1) { glDisableVertexAttribArray(ATTR_NORMAL); }
  if (ATTR_TEXCOORD != -1) { glDisableVertexAttribArray(ATTR_TEXCOORD); }
  if (ATTR_COLOR != -1) { glDisableVertexAttribArray(ATTR_COLOR); }
  if (ATTR_TANGENT != -1) { glDisableVertexAttribArray(ATTR_TANGENT); }
  if (ATTR_BINORMAL != -1) { glDisableVertexAttribArray(ATTR_BINORMAL);  }

}

void forward_renderer_render_static(static_object* so) {
  
  mat4 r_world_matrix = mat4_world( so->position, so->scale, so->rotation );
  mat4_to_array(r_world_matrix, world_matrix);
  
  renderable* r = asset_hndl_ptr(so->renderable);
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    if(s->is_rigged) {
      error("Renderable for static object is rigged!");
    }
    
    forward_renderer_use_material(asset_hndl_ptr(s->material));
    SDL_GL_CheckError();
    
    shader_program* prog = material_get_entry(asset_hndl_ptr(s->material), 0)->program;
    GLint recieve_shadows = glGetUniformLocation(shader_program_handle(prog), "recieve_shadows");
    if (recieve_shadows != -1) {
      glUniform1i(recieve_shadows, so->recieve_shadows);
    }
    SDL_GL_CheckError();
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
    SDL_GL_CheckError();
    
    bind_attributes_static();
    
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
    
    unbind_attributes_static();
    
    SDL_GL_CheckError();
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    SDL_GL_CheckError();
    
    forward_renderer_disuse_material();

    SDL_GL_CheckError();
    
  }
  
}

void forward_renderer_render_static_tess(static_object* so) {

  mat4 r_world_matrix = mat4_world( so->position, so->scale, so->rotation );
  mat4_to_array(r_world_matrix, world_matrix);
  
  renderable* r = asset_hndl_ptr(so->renderable);
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    if(s->is_rigged) {
      error("Renderable for static object is rigged!");
    }
    
    forward_renderer_use_material(asset_hndl_ptr(s->material));
    
    shader_program* prog = material_get_entry(asset_hndl_ptr(s->material), 0)->program;
    GLint recieve_shadows = glGetUniformLocation(shader_program_handle(prog), "recieve_shadows");
    if (recieve_shadows != -1) {
      glUniform1i(recieve_shadows, so->recieve_shadows);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
    
    bind_attributes_static();
      
      glPatchParameteri(GL_PATCH_VERTICES, 3);
      glDrawElements(GL_PATCHES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
    
    unbind_attributes_static();
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    forward_renderer_disuse_material();

  }

}

void forward_renderer_render_instance(instance_object* io) {
  
  mat4 r_world_matrix = mat4_world(io->instances[0].position, io->instances[0].scale, io->instances[0].rotation);
  mat4_to_array(r_world_matrix, world_matrix);
  
  for(int i = 0; i < io->num_instances; i++) {
    mat4 r_world_matrix = mat4_world(io->instances[i].position, io->instances[i].scale, io->instances[i].rotation);
    mat4_to_array(r_world_matrix, world_matricies+(i*16));
  }
  
  renderable* r = asset_hndl_ptr(io->renderable);
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    if(s->is_rigged) {
      error("Renderable for static object is rigged!");
    }
    
    forward_renderer_use_material(asset_hndl_ptr(s->material));
    
    shader_program* prog = material_get_entry(asset_hndl_ptr(s->material), 0)->program;
    GLint recieve_shadows = glGetUniformLocation(shader_program_handle(prog), "recieve_shadows");
    if (recieve_shadows != -1) {
      glUniform1i(recieve_shadows, io->recieve_shadows);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
    
    bind_attributes_static();
    
      glDrawElementsInstanced(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0, io->num_instances);
    
    unbind_attributes_static();
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    forward_renderer_disuse_material();

  }

}

void forward_renderer_render_physics(physics_object* po) {

  mat4 r_world_matrix = mat4_world( po->position, po->scale, po->rotation );
  mat4_to_array(r_world_matrix, world_matrix);
  
  renderable* r = asset_hndl_ptr(po->renderable);
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    if(s->is_rigged) {
      error("Physics object is rigged!");
    } 
    
    forward_renderer_use_material(asset_hndl_ptr(s->material));
    
    shader_program* prog = material_get_entry(asset_hndl_ptr(s->material), 0)->program;
    GLint recieve_shadows = glGetUniformLocation(shader_program_handle(prog), "recieve_shadows");
    glUniform1i(recieve_shadows, po->recieve_shadows);
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
        
    bind_attributes_static();
    
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
    
    unbind_attributes_static();
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    forward_renderer_disuse_material();

  }

}

#define MAX_BONES 32
static mat4 bone_matrices[MAX_BONES];
static float bone_matrix_data[4 * 4 * MAX_BONES];

void forward_renderer_render_animated(animated_object* ao) {

  skeleton* skel = asset_hndl_ptr(ao->skeleton);

  if (skel->num_bones > MAX_BONES) {
    error("animated object skeleton has too many bones (over %i)", MAX_BONES);
  }
  
  mat4 r_world_matrix = mat4_world( ao->position, ao->scale, ao->rotation );
  mat4_to_array(r_world_matrix, world_matrix);
  
  skeleton_gen_transforms(ao->pose);
  
  for(int i = 0; i < skel->num_bones; i++) {
    mat4 base, ani;
    base = skel->inv_transforms[i];
    ani = ao->pose->transforms[i];
    
    bone_matrices[i] = mat4_mul_mat4(ani, base);
    mat4_to_array(bone_matrices[i], bone_matrix_data + (i * 4 * 4));
  }
  
  renderable* r = asset_hndl_ptr(ao->renderable);
  
  for(int i = 0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    if(s->is_rigged) {

      forward_renderer_use_material(asset_hndl_ptr(s->material));
      
      shader_program* prog = material_get_entry(asset_hndl_ptr(s->material), 0)->program;
      
      GLint bone_world_matrices_u = glGetUniformLocation(shader_program_handle(prog), "bone_world_matrices");
      glUniformMatrix4fv(bone_world_matrices_u, skel->num_bones, GL_FALSE, bone_matrix_data);
      
      GLint bone_count_u = glGetUniformLocation(shader_program_handle(prog), "bone_count");
      glUniform1i(bone_count_u, skel->num_bones);
      
      GLint recieve_shadows = glGetUniformLocation(shader_program_handle(prog), "recieve_shadows");
      glUniform1i(recieve_shadows, ao->recieve_shadows);
      
      GLsizei stride = sizeof(float) * 24;
      
      glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
          
      glVertexPointer(3, GL_FLOAT, stride, (void*)0);
      glEnableClientState(GL_VERTEX_ARRAY);
      
      glNormalPointer(GL_FLOAT, stride, (void*)(sizeof(float) * 3));
      glEnableClientState(GL_NORMAL_ARRAY);
      
      glVertexAttribPointer(ATTR_TANGENT, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
      glEnableVertexAttribArray(ATTR_TANGENT);
      
      glVertexAttribPointer(ATTR_BINORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 9));
      glEnableVertexAttribArray(ATTR_BINORMAL);
      
      glTexCoordPointer(2, GL_FLOAT, stride, (void*)(sizeof(float) * 12));
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      
      glColorPointer(4, GL_FLOAT, stride, (void*)(sizeof(float) * 14));
      glEnableClientState(GL_COLOR_ARRAY);
      
      glVertexAttribPointer(ATTR_BONE_INDICIES, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 18));
      glEnableVertexAttribArray(ATTR_BONE_INDICIES);
      
      glVertexAttribPointer(ATTR_BONE_WEIGHTS, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 21));
      glEnableVertexAttribArray(ATTR_BONE_WEIGHTS);
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
      
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_NORMAL_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);  
      glDisableClientState(GL_COLOR_ARRAY);  
      
      glDisableVertexAttribArray(ATTR_TANGENT);
      glDisableVertexAttribArray(ATTR_BINORMAL);
      glDisableVertexAttribArray(ATTR_BONE_INDICIES);  
      glDisableVertexAttribArray(ATTR_BONE_WEIGHTS);  
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      forward_renderer_disuse_material();
    
    } else {
      error("animated object is not rigged");
    }
    
  }
  
}

void forward_renderer_render_skeleton(skeleton* s) {
  
  skeleton_gen_transforms(s);
  
  for(int i = 0; i < s->num_bones; i++) {
    bone* main_bone = s->bones[i];
    vec4 pos = mat4_mul_vec4(s->transforms[i], vec4_new(0,0,0,1));
    forward_renderer_render_axis(s->transforms[i]);
    
    if (main_bone->parent != NULL) {
      vec4 par_pos = mat4_mul_vec4(s->transforms[main_bone->parent->id], vec4_new(0,0,0,1));
      glDisable(GL_DEPTH_TEST);
      glColor3f(0.0,0.0,0.0);
      glBegin(GL_LINES);
        glVertex3f(pos.x, pos.y, pos.z);
        glVertex3f(par_pos.x, par_pos.y, par_pos.z);
      glEnd();
      glColor3f(1.0,1.0,1.0);
      glEnable(GL_DEPTH_TEST);
    }
    
  }
  
}

void forward_renderer_render_axis(mat4 world) {
  
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

void forward_renderer_render_light(light* l) {
  
  mat4 viewm = camera_view_matrix(CAMERA);
  mat4 projm = camera_proj_matrix(CAMERA, graphics_viewport_ratio() );
  
  vec4 light_pos = vec4_new(l->position.x, l->position.y, l->position.z, 1);
  light_pos = mat4_mul_vec4(viewm, light_pos);
  light_pos = mat4_mul_vec4(projm, light_pos);
  
  light_pos = vec4_div(light_pos, light_pos.w);
  
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
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0.25);
  
  texture* lightbulb = asset_hndl_ptr(asset_hndl_new(P("$CORANGE/ui/lightbulb.dds")));
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, texture_handle(lightbulb));
  glEnable(GL_TEXTURE_2D);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(left, top, -light_pos.z);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(left,  bot, -light_pos.z);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(right,  bot, -light_pos.z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(right, top, -light_pos.z);
	glEnd();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
  
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_BLEND);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
}

void forward_renderer_render_landscape(landscape* ls) {
  
  mat4 r_world_matrix = mat4_world(ls->position, ls->scale, ls->rotation);
  mat4_to_array(r_world_matrix, world_matrix);
  
  material* terrain_mat = asset_hndl_ptr(asset_hndl_new(P("$CORANGE/shaders/forward/terrain.mat")));
  
  shader_program* terrain_prog = material_get_entry(terrain_mat, 0)->program;
  GLuint terrain_handle = shader_program_handle(terrain_prog);
  glUseProgram(terrain_handle);
  
  GLint world_matrix_u = glGetUniformLocation(terrain_handle, "world_matrix");
  glUniformMatrix4fv(world_matrix_u, 1, 0, world_matrix);
  
  GLint proj_matrix_u = glGetUniformLocation(terrain_handle, "proj_matrix");
  glUniformMatrix4fv(proj_matrix_u, 1, 0, proj_matrix);
  
  GLint view_matrix_u = glGetUniformLocation(terrain_handle, "view_matrix");
  glUniformMatrix4fv(view_matrix_u, 1, 0, view_matrix);
  
  GLint camera_position = glGetUniformLocation(terrain_handle, "camera_position");
  glUniform3f(camera_position, CAMERA->position.x, CAMERA->position.y, CAMERA->position.z);
  
  GLint camera_direction = glGetUniformLocation(terrain_handle, "camera_direction");
  vec3 direction = vec3_normalize(vec3_sub(CAMERA->target, CAMERA->position));
  glUniform3f(camera_direction, direction.x, direction.y, direction.z);
  
  for(int i = 0; i < num_lights; i++) {
    light_power[i] = lights[i]->power;
    light_falloff[i] = lights[i]->falloff;
    light_position[i] = lights[i]->position;
    light_target[i] = lights[i]->target;
    light_diffuse[i] = lights[i]->diffuse_color;
    light_ambient[i] = lights[i]->ambient_color;
    light_specular[i] = lights[i]->specular_color;
  }
  
  glUniform1i(glGetUniformLocation(terrain_handle, "num_lights"), num_lights);
  
  GLint light_power_u = glGetUniformLocation(terrain_handle, "light_power");
  GLint light_falloff_u = glGetUniformLocation(terrain_handle, "light_falloff");
  GLint light_position_u = glGetUniformLocation(terrain_handle, "light_position");
  GLint light_target_u = glGetUniformLocation(terrain_handle, "light_target");
  GLint light_diffuse_u = glGetUniformLocation(terrain_handle, "light_diffuse");
  GLint light_ambient_u = glGetUniformLocation(terrain_handle, "light_ambient");
  GLint light_specular_u = glGetUniformLocation(terrain_handle, "light_specular");
  
  glUniform1fv(light_power_u, num_lights, (const GLfloat*)light_power);
  glUniform1fv(light_falloff_u, num_lights, (const GLfloat*)light_falloff);
  glUniform3fv(light_position_u, num_lights, (const GLfloat*)light_position);
  glUniform3fv(light_target_u, num_lights, (const GLfloat*)light_target);
  glUniform3fv(light_diffuse_u, num_lights, (const GLfloat*)light_diffuse);
  glUniform3fv(light_ambient_u, num_lights, (const GLfloat*)light_ambient);
  glUniform3fv(light_specular_u, num_lights, (const GLfloat*)light_specular);
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, texture_handle(asset_hndl_ptr(ls->normalmap)));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(terrain_handle, "normals"), 0);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glBindTexture(GL_TEXTURE_2D, texture_handle(asset_hndl_ptr(ls->colormap)));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(terrain_handle, "color"), 1);
  
  glActiveTexture(GL_TEXTURE0 + 2 );
  glBindTexture(GL_TEXTURE_2D, texture_handle(asset_hndl_ptr(ls->attributemap)));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(terrain_handle, "attribs"), 2);
  
  texture* random = asset_hndl_ptr(asset_hndl_new(P("$CORANGE/resources/random.dds")));
  
  glActiveTexture(GL_TEXTURE0 + 3 );
  glBindTexture(GL_TEXTURE_2D, texture_handle(random));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(terrain_handle, "random"), 3);
  
  glActiveTexture(GL_TEXTURE0 + 4 );
  glBindTexture(GL_TEXTURE_2D, texture_handle(asset_hndl_ptr(ls->near_texture)));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(terrain_handle, "surface_diffuse"), 4);
  
  glActiveTexture(GL_TEXTURE0 + 5 );
  glBindTexture(GL_TEXTURE_2D, texture_handle(asset_hndl_ptr(ls->near_texture_bump)));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(terrain_handle, "surface_bump"), 5);
  
  glActiveTexture(GL_TEXTURE0 + 6 );
  glBindTexture(GL_TEXTURE_2D, texture_handle(asset_hndl_ptr(ls->far_texture)));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(terrain_handle, "surface_diffuse_far"), 6);
  
  glActiveTexture(GL_TEXTURE0 + 7 );
  glBindTexture(GL_TEXTURE_2D, texture_handle(asset_hndl_ptr(ls->far_texture_bump)));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(terrain_handle, "surface_bump_far"), 7);
  
  terrain* terr = asset_hndl_ptr(ls->terrain);
  
  for(int i = 0; i < terr->num_chunks; i++) {
    
    terrain_chunk* tc = terr->chunks[i];
    
    vec3 position = vec3_add(vec3_new((tc->x+0.5) * terr->chunk_width, 0, (tc->y+0.5) * terr->chunk_height), ls->position);
    
    vec3 camera_pos = vec3_normalize(vec3_sub(position, CAMERA->position));
    vec3 camera_dir = vec3_normalize(vec3_sub(CAMERA->target, CAMERA->position));
    float angle = vec3_dot(camera_pos, camera_dir);
    
    if (angle < -CAMERA->fov) continue;
    
    int index_id = min(0.01 * vec3_dist_manhattan(position, CAMERA->position), NUM_TERRAIN_BUFFERS-1);
    
    glBindBuffer(GL_ARRAY_BUFFER, tc->vertex_buffer);
  
    glVertexPointer(3, GL_FLOAT, 0, (void*)0);
    glEnableClientState(GL_VERTEX_ARRAY);
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tc->index_buffers[index_id]);
      glDrawElements(GL_TRIANGLES, tc->num_indicies[index_id], GL_UNSIGNED_INT, (void*)0);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    
  }
  
  tex_counter = 7;
  while(tex_counter >= 0) {
    glActiveTexture(GL_TEXTURE0 + tex_counter);
    glDisable(GL_TEXTURE_2D);
    tex_counter--;
  }
  
  glUseProgram(0);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
}


