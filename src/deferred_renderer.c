#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_local.h"

#include "error.h"

#include "shader.h"
#include "camera.h"
#include "texture.h"
#include "renderable.h"
#include "material.h"
#include "asset_manager.h"
#include "shadow_mapper.h"

#include "viewport.h"

#include "deferred_renderer.h"

static camera* CAMERA = NULL;

static float PROJ_MATRIX[16];
static float VIEW_MATRIX[16];
static float WORLD_MATRIX[16];

static float LIGHT_VIEW_MATRIX[16];
static float LIGHT_PROJ_MATRIX[16];

static shader_program* PROGRAM;
static shader_program* PROGRAM_ANIMATED;
static shader_program* PROGRAM_CLEAR;
static shader_program* PROGRAM_SSAO;

static shader_program* SCREEN_PROGRAM;
static shader_program* SCREEN_TONEMAP;
static shader_program* SCREEN_POST;

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

static texture* SHADOW_TEX = NULL;
static texture* COLOR_CORRECTION = NULL;
static texture* RANDOM = NULL;
static texture* ENVIRONMENT = NULL;
static texture* VIGNETTING = NULL;

static light* SHADOW_LIGHT = NULL;

#define DEFERRED_MAX_LIGHTS 32

static int num_lights;

static light* lights[DEFERRED_MAX_LIGHTS];
static float light_power[DEFERRED_MAX_LIGHTS];
static float light_falloff[DEFERRED_MAX_LIGHTS];
static vector3 light_position[DEFERRED_MAX_LIGHTS];
static vector3 light_target[DEFERRED_MAX_LIGHTS];
static vector3 light_diffuse[DEFERRED_MAX_LIGHTS];
static vector3 light_ambient[DEFERRED_MAX_LIGHTS];
static vector3 light_specular[DEFERRED_MAX_LIGHTS];

static float EXPOSURE;
static float EXPOSURE_SPEED;
static float EXPOSURE_TARGET;

void deferred_renderer_init() {
  
  num_lights = 0;
  
  EXPOSURE = 0.0;
  EXPOSURE_SPEED = 1.0;
  EXPOSURE_TARGET = 0.4;
  
  COLOR_CORRECTION = asset_load_get("$CORANGE/resources/identity.lut");
  RANDOM = asset_load_get("$CORANGE/resources/random.dds");
  ENVIRONMENT = asset_load_get("$CORANGE/resources/envmap.dds");
  VIGNETTING = asset_load_get("$CORANGE/resources/vignetting.dds");
  
  PROGRAM = asset_load_get("$SHADERS/deferred.prog");
  PROGRAM_ANIMATED = asset_load_get("$SHADERS/deferred_animated.prog");
  PROGRAM_CLEAR = asset_load_get("$SHADERS/deferred_clear.prog");
  PROGRAM_SSAO = asset_load_get("$SHADERS/deferred_ssao.prog");
  
  SCREEN_TONEMAP = asset_load_get("$SHADERS/deferred_tonemap.prog");
  SCREEN_PROGRAM = asset_load_get("$SHADERS/deferred_screen.prog");
  SCREEN_POST = asset_load_get("$SHADERS/deferred_post.prog");
  
  NORMAL = glGetAttribLocation(*PROGRAM, "normal");
  TANGENT = glGetAttribLocation(*PROGRAM, "tangent");
  BINORMAL = glGetAttribLocation(*PROGRAM, "binormal");  
  
  NORMAL_ANIMATED = glGetAttribLocation(*PROGRAM_ANIMATED, "normal");
  TANGENT_ANIMATED = glGetAttribLocation(*PROGRAM_ANIMATED, "tangent");
  BINORMAL_ANIMATED = glGetAttribLocation(*PROGRAM_ANIMATED, "binormal");  
  BONE_INDICIES = glGetAttribLocation(*PROGRAM_ANIMATED, "bone_indicies");
  BONE_WEIGHTS = glGetAttribLocation(*PROGRAM_ANIMATED, "bone_weights"); 
  
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  
  glGenRenderbuffers(1, &diffuse_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, diffuse_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, viewport_width(), viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, diffuse_buffer);   
  
  glGenRenderbuffers(1, &positions_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, positions_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F, viewport_width(), viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, positions_buffer);  
  
  glGenRenderbuffers(1, &normals_buffer);  
  glBindRenderbuffer(GL_RENDERBUFFER, normals_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F, viewport_width(), viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, normals_buffer);  
  
  glGenRenderbuffers(1, &depth_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, viewport_width(), viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);  
  
  glGenTextures(1, &diffuse_texture);
  glBindTexture(GL_TEXTURE_2D, diffuse_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport_width(), viewport_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, diffuse_texture, 0);
  
  glGenTextures(1, &positions_texture);
  glBindTexture(GL_TEXTURE_2D, positions_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, viewport_width(), viewport_height(), 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, positions_texture, 0);
  
  glGenTextures(1, &normals_texture);
  glBindTexture(GL_TEXTURE_2D, normals_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, viewport_width(), viewport_height(), 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, normals_texture, 0);
  
  glGenTextures(1, &depth_texture);
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, viewport_width(), viewport_height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
  
  
  glGenFramebuffers(1, &ssao_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
  
  glGenRenderbuffers(1, &ssao_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, ssao_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, viewport_width() / 2, viewport_height() / 2);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ssao_buffer);   
  
  glGenTextures(1, &ssao_texture);
  glBindTexture(GL_TEXTURE_2D, ssao_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport_width() / 2, viewport_height() / 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_texture, 0);
  
  glGenFramebuffers(1, &hdr_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo);
  
  glGenRenderbuffers(1, &hdr_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, hdr_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F, viewport_width(), viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ldr_buffer);   
  
  glGenTextures(1, &hdr_texture);
  glBindTexture(GL_TEXTURE_2D, hdr_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, viewport_width(), viewport_height(), 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdr_texture, 0);
  
  glGenFramebuffers(1, &ldr_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, ldr_fbo);
  
  glGenRenderbuffers(1, &ldr_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, ldr_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, viewport_width(), viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ldr_buffer);   
  
  glGenTextures(1, &ldr_texture);
  glBindTexture(GL_TEXTURE_2D, ldr_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport_width(), viewport_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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

void deferred_renderer_set_color_correction(texture* t) {
  COLOR_CORRECTION = t;
}

void deferred_renderer_set_shadow_light(light* l) {
  SHADOW_LIGHT = l;
}

static void deferred_renderer_use_material(material* mat, shader_program* PROG) {
  
  /* Set material parameters */
  
  int tex_counter = 0;
  
  for(int i = 0; i < mat->keys->num_items; i++) {
    char* key = list_get(mat->keys, i);
    
    int* type = dictionary_get(mat->types, key);
    void* property = dictionary_get(mat->properties, key);
    
    GLint loc = glGetUniformLocation(*PROG, key);
    
    GLint world_matrix_u = glGetUniformLocation(*PROG, "world_matrix");
    glUniformMatrix4fv(world_matrix_u, 1, 0, WORLD_MATRIX);
  
    GLint proj_matrix_u = glGetUniformLocation(*PROG, "proj_matrix");
    glUniformMatrix4fv(proj_matrix_u, 1, 0, PROJ_MATRIX);
    
    GLint view_matrix_u = glGetUniformLocation(*PROG, "view_matrix");
    glUniformMatrix4fv(view_matrix_u, 1, 0, VIEW_MATRIX);
    
    if (*type == mat_type_texture) {
    
      glUniform1i(loc, tex_counter);
      glActiveTexture(GL_TEXTURE0 + tex_counter);
      glBindTexture(GL_TEXTURE_2D, *((texture*)property));
      tex_counter++;
    
    } else if (*type == mat_type_int) {
      
      glUniform1i(loc, *((int*)property));
    
    } else if (*type == mat_type_float) {
    
      glUniform1f(loc, *((float*)property));
      
    } else if (*type == mat_type_vector2) {
    
      vector2 v = *((vector2*)property);
      glUniform2f(loc, v.x, v.y);
    
    } else if (*type == mat_type_vector3) {
    
      vector3 v = *((vector3*)property);
      glUniform3f(loc, v.x, v.y, v.z);
  
    } else if (*type == mat_type_vector4) {
    
      vector4 v = *((vector4*)property);
      glUniform4f(loc, v.w, v.x, v.y, v.z);
    
    } else {
      /* Do nothing */
    }
     
  }  

}

static void deferred_renderer_setup_camera() {

  matrix_4x4 viewm = camera_view_matrix(CAMERA);
  matrix_4x4 projm = camera_proj_matrix(CAMERA, viewport_ratio() );
  
  m44_to_array(viewm, VIEW_MATRIX);
  m44_to_array(projm, PROJ_MATRIX);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(VIEW_MATRIX);
  
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(PROJ_MATRIX);
  
  matrix_4x4 lviewm = light_view_matrix(SHADOW_LIGHT);
  matrix_4x4 lprojm = light_proj_matrix(SHADOW_LIGHT);
  
  m44_to_array(lviewm, LIGHT_VIEW_MATRIX);
  m44_to_array(lprojm, LIGHT_PROJ_MATRIX);

}

void deferred_renderer_begin() {

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  
  deferred_renderer_setup_camera();
  
  GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
  glDrawBuffers(3, buffers);
  
  glClearColor(0.2, 0.2, 0.2, 1.0f);
  glClearDepth(1.0f);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  glUseProgram(*PROGRAM_CLEAR);
  
  GLint start = glGetUniformLocation(*PROGRAM_CLEAR, "start");
  GLint end = glGetUniformLocation(*PROGRAM_CLEAR, "end");
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
  
}

void deferred_renderer_end() {
  
  /* Render out ssao */
  
  glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
  
  glViewport(0, 0, viewport_width() / 2, viewport_height() / 2);
  
  glUseProgram(*PROGRAM_SSAO);
  
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
  glUniform1i(glGetUniformLocation(*PROGRAM_SSAO, "depth_texture"), 0);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glBindTexture(GL_TEXTURE_2D, *RANDOM);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*PROGRAM_SSAO, "random_texture"), 1);
  
  float seed = CAMERA->position.x + CAMERA->position.y + CAMERA->position.z;
  glUniform1f(glGetUniformLocation(*PROGRAM_SSAO, "seed"), seed);
  
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
  
  glViewport(0, 0, viewport_width(), viewport_height());
  glDisable(GL_DEPTH_TEST);
  
  glUseProgram(*SCREEN_PROGRAM);
  
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
  glUniform1i(glGetUniformLocation(*SCREEN_PROGRAM, "diffuse_texture"), 0);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glBindTexture(GL_TEXTURE_2D, positions_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*SCREEN_PROGRAM, "positions_texture"), 1);
  
  glActiveTexture(GL_TEXTURE0 + 2 );
  glBindTexture(GL_TEXTURE_2D, normals_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*SCREEN_PROGRAM, "normals_texture"), 2);
  
  glActiveTexture(GL_TEXTURE0 + 3 );
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*SCREEN_PROGRAM, "depth_texture"), 3);
  
  glActiveTexture(GL_TEXTURE0 + 4 );
  glBindTexture(GL_TEXTURE_2D, *SHADOW_TEX);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*SCREEN_PROGRAM, "shadows_texture"), 4);
  
  glActiveTexture(GL_TEXTURE0 + 5 );
  glBindTexture(GL_TEXTURE_2D, ssao_texture);
  glEnable(GL_TEXTURE_2D);
  glGenerateMipmap(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*SCREEN_PROGRAM, "ssao_texture"), 5);
  
  glActiveTexture(GL_TEXTURE0 + 6 );
  glBindTexture(GL_TEXTURE_2D, *ENVIRONMENT);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*SCREEN_PROGRAM, "env_texture"), 6);
  
  GLint cam_position = glGetUniformLocation(*SCREEN_PROGRAM, "camera_position");
  glUniform3f(cam_position, CAMERA->position.x, CAMERA->position.y, CAMERA->position.z);
  
  GLint lproj_matrix_u = glGetUniformLocation(*SCREEN_PROGRAM, "light_proj");
  glUniformMatrix4fv(lproj_matrix_u, 1, 0, LIGHT_PROJ_MATRIX);
  
  GLint lview_matrix_u = glGetUniformLocation(*SCREEN_PROGRAM, "light_view");
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
  
  glUniform1i(glGetUniformLocation(*SCREEN_PROGRAM, "num_lights"), num_lights);
  
  GLint light_power_u = glGetUniformLocation(*SCREEN_PROGRAM, "light_power");
  GLint light_falloff_u = glGetUniformLocation(*SCREEN_PROGRAM, "light_falloff");
  GLint light_position_u = glGetUniformLocation(*SCREEN_PROGRAM, "light_position");
  GLint light_target_u = glGetUniformLocation(*SCREEN_PROGRAM, "light_target");
  GLint light_diffuse_u = glGetUniformLocation(*SCREEN_PROGRAM, "light_diffuse");
  GLint light_ambient_u = glGetUniformLocation(*SCREEN_PROGRAM, "light_ambient");
  GLint light_specular_u = glGetUniformLocation(*SCREEN_PROGRAM, "light_specular");
  
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
  
  glUseProgram(*SCREEN_TONEMAP);
  
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
  glUniform1i(glGetUniformLocation(*SCREEN_TONEMAP, "hdr_texture"), 0);

  glUniform1f(glGetUniformLocation(*SCREEN_TONEMAP, "exposure"), EXPOSURE);
  
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
  
  glUseProgram(*SCREEN_POST);
  
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
  glUniform1i(glGetUniformLocation(*SCREEN_POST, "diffuse_texture"), 0);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glBindTexture(GL_TEXTURE_2D, *VIGNETTING);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*SCREEN_POST, "vignetting_texture"), 1);
  
  glActiveTexture(GL_TEXTURE0 + 2 );
  glBindTexture(GL_TEXTURE_3D, *COLOR_CORRECTION);
  glEnable(GL_TEXTURE_3D);
  glUniform1i(glGetUniformLocation(*SCREEN_POST, "lut"), 2);
  
  glUniform1i(glGetUniformLocation(*SCREEN_POST, "width"), viewport_width());
  glUniform1i(glGetUniformLocation(*SCREEN_POST, "height"), viewport_height());
  
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

void deferred_renderer_render_static(static_object* s) {

  matrix_4x4 r_world_matrix = m44_world( s->position, s->scale, s->rotation );
  m44_to_array(r_world_matrix, WORLD_MATRIX);
  
  renderable* r = s->renderable;
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    if(s->is_rigged) {
      
      glUseProgram(*PROGRAM);
      
      deferred_renderer_use_material(s->base, PROGRAM);
      
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
      
    } else {
    
      glUseProgram(*PROGRAM);
      
      deferred_renderer_use_material(s->base, PROGRAM);
      
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
  
}

#define MAX_BONES 32
static matrix_4x4 bone_matrices[MAX_BONES];
static float bone_matrix_data[4 * 4 * MAX_BONES];

void deferred_renderer_render_animated(animated_object* ao) {

  if (ao->skeleton->num_bones > MAX_BONES) {
    error("animated object skeleton has too many bones (over %i)", MAX_BONES);
  }

  matrix_4x4 r_world_matrix = m44_world( ao->position, ao->scale, ao->rotation );
  m44_to_array(r_world_matrix, WORLD_MATRIX);
  
  skeleton_gen_transforms(ao->pose);
  
  for(int i = 0; i < ao->skeleton->num_bones; i++) {
    matrix_4x4 base, ani;
    base = ao->skeleton->inv_transforms[i];
    ani = ao->pose->transforms[i];
    
    bone_matrices[i] = m44_mul_m44(ani, base);
    m44_to_array(bone_matrices[i], bone_matrix_data + (i * 4 * 4));
  }
  
  renderable* r = ao->renderable;
  
  for(int i = 0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    if(s->is_rigged) {
      
      glUseProgram(*PROGRAM_ANIMATED);
      
      deferred_renderer_use_material(s->base, PROGRAM_ANIMATED);
      
      GLint bone_world_matrices_u = glGetUniformLocation(*PROGRAM_ANIMATED, "bone_world_matrices");
      glUniformMatrix4fv(bone_world_matrices_u, ao->skeleton->num_bones, GL_FALSE, bone_matrix_data);
      
      GLint bone_count_u = glGetUniformLocation(*PROGRAM_ANIMATED, "bone_count");
      glUniform1i(bone_count_u, ao->skeleton->num_bones);
      
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

/* TODO: This could do with a proper shader attached to it */

void deferred_renderer_render_light(light* l) {
  
  matrix_4x4 viewm = camera_view_matrix(CAMERA);
  matrix_4x4 projm = camera_proj_matrix(CAMERA, viewport_ratio() );
  
  vector4 light_pos = v4(l->position.x, l->position.y, l->position.z, 1);
  light_pos = m44_mul_v4(viewm, light_pos);
  light_pos = m44_mul_v4(projm, light_pos);
  
  light_pos = v4_div(light_pos, light_pos.w);
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(0, viewport_width(), viewport_height(), 0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  float top = ((-light_pos.y + 1) / 2) * viewport_height() - 8;
  float bot = ((-light_pos.y + 1) / 2) * viewport_height() + 8;
  float left = ((light_pos.x + 1) / 2) * viewport_width() - 8;
  float right = ((light_pos.x + 1) / 2) * viewport_width() + 8;
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0.25);
  
  texture* lightbulb = asset_load_get("$CORANGE/ui/lightbulb.dds");
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, *lightbulb);
  glEnable(GL_TEXTURE_2D);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(left, top, -light_pos.z);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(right, top, -light_pos.z);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(right,  bot, -light_pos.z);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(left,  bot, -light_pos.z);
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

void deferred_renderer_render_axis(matrix_4x4 world) {

  vector4 x_pos = m44_mul_v4(world, v4(2,0,0,1));
  vector4 y_pos = m44_mul_v4(world, v4(0,2,0,1));
  vector4 z_pos = m44_mul_v4(world, v4(0,0,2,1));
  vector4 base_pos = m44_mul_v4(world, v4(0,0,0,1));
  
  x_pos = v4_div(x_pos, x_pos.w);
  y_pos = v4_div(y_pos, y_pos.w);
  z_pos = v4_div(z_pos, z_pos.w);
  base_pos = v4_div(base_pos, base_pos.w);
  
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


