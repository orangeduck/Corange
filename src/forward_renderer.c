#include <string.h>
#include <math.h>

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_local.h"

#include "error.h"

#include "graphics_manager.h"
#include "asset_manager.h"

#include "renderable.h"
#include "camera.h"
#include "matrix.h"
#include "shader.h"
#include "texture.h"
#include "dictionary.h"
#include "timing.h"

#include "forward_renderer.h"

static int use_shadows;

static camera* CAMERA = NULL;
static light* SHADOW_LIGHT = NULL;

static texture* SHADOW_TEX = NULL;
static texture* COLOR_CORRECTION = NULL;
static texture* VIGNETTING = NULL;

static shader_program* GRADIENT = NULL;
static shader_program* SCREEN_TONEMAP = NULL;
static shader_program* SCREEN_POST = NULL;

static float proj_matrix[16];
static float view_matrix[16];
static float world_matrix[16];
static float lview_matrix[16];
static float lproj_matrix[16];

static float world_matricies[16*MAX_INSTANCES];

static float timer = 0.0;

static int TANGENT;
static int BINORMAL;
static int BONE_INDICIES;
static int BONE_WEIGHTS;

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
static vector3 light_position[FORWARD_MAX_LIGHTS];
static vector3 light_target[FORWARD_MAX_LIGHTS];
static vector3 light_diffuse[FORWARD_MAX_LIGHTS];
static vector3 light_ambient[FORWARD_MAX_LIGHTS];
static vector3 light_specular[FORWARD_MAX_LIGHTS];

static float EXPOSURE;
static float EXPOSURE_SPEED;
static float EXPOSURE_TARGET;

static int MULTISAMPLES;

void forward_renderer_init() {
  
  MULTISAMPLES = 0;
  
  use_shadows = false;
  num_lights = 0;
  
  EXPOSURE = 0.0;
  EXPOSURE_SPEED = 1.0;
  EXPOSURE_TARGET = 0.4;
  
  COLOR_CORRECTION = asset_load_get("$CORANGE/resources/identity.lut");
  VIGNETTING = asset_load_get("$CORANGE/resources/vignetting.dds");
  GRADIENT = asset_load_get("$CORANGE/shaders/gradient.prog");
  SCREEN_TONEMAP = asset_load_get("$CORANGE/shaders/deferred_tonemap.prog");
  SCREEN_POST = asset_load_get("$CORANGE/shaders/deferred_post.prog");
  
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
    use_shadows = 0;
  } else {
    use_shadows = 1;
    SHADOW_TEX = t;
  }
  
}

void forward_renderer_set_color_correction(texture* t) {
  COLOR_CORRECTION = t;
}

static void render_gradient() {

  GLuint gradient_handle = shader_program_handle(GRADIENT);
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
  
  timer += frame_time();
  
  glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo);
  
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
  render_gradient();
  
  forward_renderer_setup_camera();
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  
}

void forward_renderer_setup_camera() {

  if (CAMERA == NULL) {
    error("Camera not set yet!");
  }
  if (SHADOW_LIGHT == NULL) {
    error("Light not set yet!");
  }

  matrix_4x4 viewm = camera_view_matrix(CAMERA);
  matrix_4x4 projm = camera_proj_matrix(CAMERA, graphics_viewport_ratio() );
  
  m44_to_array(viewm, view_matrix);
  m44_to_array(projm, proj_matrix);

  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(view_matrix);
  
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(proj_matrix);
  
  /* Setup light stuff */
  
  matrix_4x4 lviewm = light_view_matrix(SHADOW_LIGHT);
  matrix_4x4 lprojm = light_proj_matrix(SHADOW_LIGHT);
  
  m44_to_array(lviewm, lview_matrix);
  m44_to_array(lprojm, lproj_matrix);
  
}

void forward_renderer_end() {
  
  /* Resolve multisamples */
  
  glBindFramebuffer(GL_READ_FRAMEBUFFER, hdr_fbo);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdr_res_fbo);
  glBlitFramebuffer(0, 0, graphics_viewport_width(), graphics_viewport_height(), 0, 0, graphics_viewport_width(), graphics_viewport_height(), GL_COLOR_BUFFER_BIT, GL_LINEAR);
  
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  
  /* Render HDR to LDR buffer */
  
  glBindFramebuffer(GL_FRAMEBUFFER, ldr_fbo);
  
  GLuint tonemap_handle = shader_program_handle(SCREEN_TONEMAP);
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
  
  GLuint post_handle = shader_program_handle(SCREEN_POST);
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
  glBindTexture(GL_TEXTURE_2D, texture_handle(VIGNETTING));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(post_handle, "vignetting_texture"), 1);
  
  glActiveTexture(GL_TEXTURE0 + 2 );
  glBindTexture(GL_TEXTURE_3D, texture_handle(COLOR_CORRECTION));
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
  
}

float forward_renderer_get_exposure() {
  return EXPOSURE;
}

void forward_renderer_set_exposure(float exposure) {
  EXPOSURE = exposure;
}

static int tex_counter = 0;
static void forward_renderer_use_material(material* mat) {
  
  shader_program* prog = dictionary_get(mat->properties, "program");
  GLuint prog_handle = shader_program_handle(prog);
  
  glUseProgram(prog_handle);
  
  /* Set global parameters */
  
  TANGENT = glGetAttribLocation(prog_handle, "tangent");
  BINORMAL = glGetAttribLocation(prog_handle, "binormal");
  BONE_INDICIES = glGetAttribLocation(prog_handle, "bone_indicies");
  BONE_WEIGHTS = glGetAttribLocation(prog_handle, "bone_weights");

  GLint camera_position = glGetUniformLocation(prog_handle, "camera_position");
  if (camera_position != -1) {
    glUniform3f(camera_position, CAMERA->position.x, CAMERA->position.y, CAMERA->position.z);
  }
  
  GLint camera_direction = glGetUniformLocation(prog_handle, "camera_direction");
  if (camera_direction != -1) {
    vector3 direction = v3_normalize(v3_sub(CAMERA->target, CAMERA->position));
    glUniform3f(camera_direction, direction.x, direction.y, direction.z);
  }
  
  for(int i = 0; i < num_lights; i++) {
    light_power[i] = lights[i]->power;
    light_falloff[i] = lights[i]->falloff;
    light_position[i] = lights[i]->position;
    light_target[i] = lights[i]->target;
    light_diffuse[i] = lights[i]->diffuse_color;
    light_ambient[i] = lights[i]->ambient_color;
    light_specular[i] = lights[i]->specular_color;
  }
  
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

  GLint time = glGetUniformLocation(prog_handle, "time");
  glUniform1f(time,timer);
  
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
  
  /* Set material parameters */
  
  tex_counter = 0;
  
  for(int i = 0; i < mat->keys->num_items; i++) {
    char* key = list_get(mat->keys, i);
    
    int* type = dictionary_get(mat->types, key);
    void* property = dictionary_get(mat->properties, key);
    
    GLint loc = glGetUniformLocation(prog_handle, key);
    
    if (*type == mat_type_texture) {
    
      glUniform1i(loc, tex_counter);
      glActiveTexture(GL_TEXTURE0 + tex_counter);
      glBindTexture(GL_TEXTURE_2D, texture_handle(property));
      glEnable(GL_TEXTURE_2D);
      tex_counter++;
    
    } else if (*type == mat_type_int) {
    
      glUniform1i(loc, *((float*)property));
    
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
  
  GLint shadow_map = glGetUniformLocation(prog_handle, "shadow_map");
  if ( use_shadows && (shadow_map != -1) ) {
    glUniform1i(shadow_map, tex_counter);
    glActiveTexture(GL_TEXTURE0 + tex_counter);
    glBindTexture(GL_TEXTURE_2D, texture_handle(SHADOW_TEX));
    glEnable(GL_TEXTURE_2D);
    tex_counter++;
  }
  
}

static void forward_renderer_disuse_material() {
  
  while(tex_counter > 0) {
    tex_counter--;
    glActiveTexture(GL_TEXTURE0 + tex_counter);
    glDisable(GL_TEXTURE_2D);
  }
  
  glUseProgram(0);

}

static void render_collision_mesh(collision_mesh* cm) {
  
  if (cm->is_leaf) {
    
    shader_program* collision_prog = asset_load_get("$CORANGE/shaders/collision_mesh.prog");
    GLuint collision_handle = shader_program_handle(collision_prog);
    glUseProgram(collision_handle);
    
    GLint color = glGetUniformLocation(collision_handle, "color");
    glUniform3f(color, 1, 1, 1);
    
    GLint world_matrix_u = glGetUniformLocation(collision_handle, "world_matrix");
    glUniformMatrix4fv(world_matrix_u, 1, 0, world_matrix);
    
    GLint proj_matrix_u = glGetUniformLocation(collision_handle, "proj_matrix");
    glUniformMatrix4fv(proj_matrix_u, 1, 0, proj_matrix);
    
    GLint view_matrix_u = glGetUniformLocation(collision_handle, "view_matrix");
    glUniformMatrix4fv(view_matrix_u, 1, 0, view_matrix);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1, 1.0);
    
    glVertexPointer(3, GL_FLOAT, 0, cm->verticies);
    glEnableClientState(GL_VERTEX_ARRAY);
    
      glDrawArrays(GL_TRIANGLES, 0, cm->num_verticies);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    
    glDisable(GL_BLEND);
    
    glDisable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(0, 0);
    
    glUseProgram(0);
    
  } else {
    render_collision_mesh(cm->front);
    render_collision_mesh(cm->back);
  }

}

void forward_renderer_render_collision_body(collision_body* cb) {
  
  if (cb->collision_type == collision_type_mesh) {
    render_collision_mesh(cb->collision_mesh);
  }
  
}

void forward_renderer_render_static(static_object* so) {
  
  matrix_4x4 r_world_matrix = m44_world( so->position, so->scale, so->rotation );
  m44_to_array(r_world_matrix, world_matrix);
  
  renderable* r = so->renderable;
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    if(s->is_rigged) {
      error("Renderable for static object is rigged!");
    }
    
    forward_renderer_use_material(s->base);
    
    shader_program* prog = dictionary_get(s->base->properties, "program");
    GLint recieve_shadows = glGetUniformLocation(shader_program_handle(prog), "recieve_shadows");
    if (recieve_shadows != -1) {
      glUniform1i(recieve_shadows, so->recieve_shadows);
    }
    
    GLsizei stride = sizeof(float) * 18;
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    
    glVertexPointer(3, GL_FLOAT, stride, (void*)0);
    glEnableClientState(GL_VERTEX_ARRAY);
    
    glNormalPointer(GL_FLOAT, stride, (void*)(sizeof(float) * 3));
    glEnableClientState(GL_NORMAL_ARRAY);
    
    if (TANGENT != -1) {
      glVertexAttribPointer(TANGENT, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
      glEnableVertexAttribArray(TANGENT);
    }
    
    if (BINORMAL != -1) {
      glVertexAttribPointer(BINORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 9));
      glEnableVertexAttribArray(BINORMAL);
    }
    
    glTexCoordPointer(2, GL_FLOAT, stride, (void*)(sizeof(float) * 12));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glColorPointer(4, GL_FLOAT, stride, (void*)(sizeof(float) * 14));
    glEnableClientState(GL_COLOR_ARRAY);
    
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);  
    glDisableClientState(GL_COLOR_ARRAY);  
    
    if (TANGENT != -1) {
      glDisableVertexAttribArray(TANGENT);
    }
    if (BINORMAL != -1) {
      glDisableVertexAttribArray(BINORMAL); 
    }
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    forward_renderer_disuse_material();

  }
  
  /*
  if (so->collision_body != NULL) {
    bsp_counter = 0;
    render_bsp_mesh(so->collision_body->collision_mesh);
  }
  */
  
}

void forward_renderer_render_instance(instance_object* io) {
  
  matrix_4x4 r_world_matrix = m44_world(io->instances[0].position, io->instances[0].scale, io->instances[0].rotation);
  m44_to_array(r_world_matrix, world_matrix);
  
  for(int i = 0; i < io->num_instances; i++) {
    matrix_4x4 r_world_matrix = m44_world(io->instances[i].position, io->instances[i].scale, io->instances[i].rotation);
    m44_to_array(r_world_matrix, world_matricies+(i*16));
  }
  
  renderable* r = io->renderable;
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    if(s->is_rigged) {
      error("Renderable for static object is rigged!");
    }
    
    forward_renderer_use_material(s->base);
    
    shader_program* prog = dictionary_get(s->base->properties, "program");
    GLint recieve_shadows = glGetUniformLocation(shader_program_handle(prog), "recieve_shadows");
    if (recieve_shadows != -1) {
      glUniform1i(recieve_shadows, io->recieve_shadows);
    }
    
    GLsizei stride = sizeof(float) * 18;
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    
    glVertexPointer(3, GL_FLOAT, stride, (void*)0);
    glEnableClientState(GL_VERTEX_ARRAY);
    
    glNormalPointer(GL_FLOAT, stride, (void*)(sizeof(float) * 3));
    glEnableClientState(GL_NORMAL_ARRAY);
    
    if (TANGENT != -1) {
      glVertexAttribPointer(TANGENT, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
      glEnableVertexAttribArray(TANGENT);
    }
    
    if (BINORMAL != -1) {
      glVertexAttribPointer(BINORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 9));
      glEnableVertexAttribArray(BINORMAL);
    }
    
    glTexCoordPointer(2, GL_FLOAT, stride, (void*)(sizeof(float) * 12));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glColorPointer(4, GL_FLOAT, stride, (void*)(sizeof(float) * 14));
    glEnableClientState(GL_COLOR_ARRAY);
    
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      glDrawElementsInstanced(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0, io->num_instances);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);  
    glDisableClientState(GL_COLOR_ARRAY);  
    
    if (TANGENT != -1) {
      glDisableVertexAttribArray(TANGENT);
    }
    if (BINORMAL != -1) {
      glDisableVertexAttribArray(BINORMAL); 
    }
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    forward_renderer_disuse_material();

  }

}

void forward_renderer_render_physics(physics_object* po) {

  matrix_4x4 r_world_matrix = m44_world( po->position, po->scale, po->rotation );
  m44_to_array(r_world_matrix, world_matrix);
  
  renderable* r = po->renderable;
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    if(s->is_rigged) {
      error("Physics object is rigged!")
    } 
    
    forward_renderer_use_material(s->base);
    
    shader_program* prog = dictionary_get(s->base->properties, "program");
    GLint recieve_shadows = glGetUniformLocation(shader_program_handle(prog), "recieve_shadows");
    glUniform1i(recieve_shadows, po->recieve_shadows);
    
    GLsizei stride = sizeof(float) * 18;
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
        
    glVertexPointer(3, GL_FLOAT, stride, (void*)0);
    glEnableClientState(GL_VERTEX_ARRAY);
    
    glNormalPointer(GL_FLOAT, stride, (void*)(sizeof(float) * 3));
    glEnableClientState(GL_NORMAL_ARRAY);
    
    glVertexAttribPointer(TANGENT, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
    glEnableVertexAttribArray(TANGENT);
    
    glVertexAttribPointer(BINORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 9));
    glEnableVertexAttribArray(BINORMAL);
    
    glTexCoordPointer(2, GL_FLOAT, stride, (void*)(sizeof(float) * 12));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glColorPointer(4, GL_FLOAT, stride, (void*)(sizeof(float) * 14));
    glEnableClientState(GL_COLOR_ARRAY);
    
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);  
    glDisableClientState(GL_COLOR_ARRAY);  
    
    glDisableVertexAttribArray(TANGENT);
    glDisableVertexAttribArray(BINORMAL);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    forward_renderer_disuse_material();

  }
  
  //if (po->collision_body != NULL) {
  //  bsp_counter = 0;
  //  render_bsp_mesh(po->collision_body->collision_mesh);
  //}

}

#define MAX_BONES 32
static matrix_4x4 bone_matrices[MAX_BONES];
static float bone_matrix_data[4 * 4 * MAX_BONES];

void forward_renderer_render_animated(animated_object* ao) {

  if (ao->skeleton->num_bones > MAX_BONES) {
    error("animated object skeleton has too many bones (over %i)", MAX_BONES);
  }
  
  matrix_4x4 r_world_matrix = m44_world( ao->position, ao->scale, ao->rotation );
  m44_to_array(r_world_matrix, world_matrix);
  
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

      forward_renderer_use_material(s->base);    
      
      shader_program* prog = dictionary_get(s->base->properties, "program");
      
      GLint bone_world_matrices_u = glGetUniformLocation(shader_program_handle(prog), "bone_world_matrices");
      glUniformMatrix4fv(bone_world_matrices_u, ao->skeleton->num_bones, GL_FALSE, bone_matrix_data);
      
      GLint bone_count_u = glGetUniformLocation(shader_program_handle(prog), "bone_count");
      glUniform1i(bone_count_u, ao->skeleton->num_bones);
      
      GLint recieve_shadows = glGetUniformLocation(shader_program_handle(prog), "recieve_shadows");
      glUniform1i(recieve_shadows, ao->recieve_shadows);
      
      GLsizei stride = sizeof(float) * 24;
      
      glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
          
      glVertexPointer(3, GL_FLOAT, stride, (void*)0);
      glEnableClientState(GL_VERTEX_ARRAY);
      
      glNormalPointer(GL_FLOAT, stride, (void*)(sizeof(float) * 3));
      glEnableClientState(GL_NORMAL_ARRAY);
      
      glVertexAttribPointer(TANGENT, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
      glEnableVertexAttribArray(TANGENT);
      
      glVertexAttribPointer(BINORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 9));
      glEnableVertexAttribArray(BINORMAL);
      
      glTexCoordPointer(2, GL_FLOAT, stride, (void*)(sizeof(float) * 12));
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      
      glColorPointer(4, GL_FLOAT, stride, (void*)(sizeof(float) * 14));
      glEnableClientState(GL_COLOR_ARRAY);
      
      glVertexAttribPointer(BONE_INDICIES, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 18));
      glEnableVertexAttribArray(BONE_INDICIES);
      
      glVertexAttribPointer(BONE_WEIGHTS, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 21));
      glEnableVertexAttribArray(BONE_WEIGHTS);
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
      
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_NORMAL_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);  
      glDisableClientState(GL_COLOR_ARRAY);  
      
      glDisableVertexAttribArray(TANGENT);
      glDisableVertexAttribArray(BINORMAL);
      glDisableVertexAttribArray(BONE_INDICIES);  
      glDisableVertexAttribArray(BONE_WEIGHTS);  
      
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
    vector4 pos = m44_mul_v4(s->transforms[i], v4(0,0,0,1));
    forward_renderer_render_axis(s->transforms[i]);
    
    if (main_bone->parent != NULL) {
      vector4 par_pos = m44_mul_v4(s->transforms[main_bone->parent->id], v4(0,0,0,1));
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

void forward_renderer_render_axis(matrix_4x4 world) {
  
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

void forward_renderer_render_light(light* l) {
  
  matrix_4x4 viewm = camera_view_matrix(CAMERA);
  matrix_4x4 projm = camera_proj_matrix(CAMERA, graphics_viewport_ratio() );
  
  vector4 light_pos = v4(l->position.x, l->position.y, l->position.z, 1);
  light_pos = m44_mul_v4(viewm, light_pos);
  light_pos = m44_mul_v4(projm, light_pos);
  
  light_pos = v4_div(light_pos, light_pos.w);
  
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
  
  texture* lightbulb = asset_load_get("$CORANGE/ui/lightbulb.dds");
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
  
  matrix_4x4 r_world_matrix = m44_world(ls->position, ls->scale, ls->rotation);
  m44_to_array(r_world_matrix, world_matrix);
  
  shader_program* terrain = asset_load_get("$CORANGE/shaders/terrain.prog");
  GLuint terrain_handle = shader_program_handle(terrain);
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
  vector3 direction = v3_normalize(v3_sub(CAMERA->target, CAMERA->position));
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
  glBindTexture(GL_TEXTURE_2D, texture_handle(ls->normalmap));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(terrain_handle, "normals"), 0);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glBindTexture(GL_TEXTURE_2D, texture_handle(ls->colormap));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(terrain_handle, "color"), 1);
  
  glActiveTexture(GL_TEXTURE0 + 2 );
  glBindTexture(GL_TEXTURE_2D, texture_handle(ls->attributemap));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(terrain_handle, "attribs"), 2);
  
  texture* random = asset_load_get("$CORANGE/resources/random.dds");
  
  glActiveTexture(GL_TEXTURE0 + 3 );
  glBindTexture(GL_TEXTURE_2D, texture_handle(random));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(terrain_handle, "random"), 3);
  
  char diffuse_name[512];
  char normals_name[512];
  char diffuse_far_name[512];
  char normals_far_name[512];
  
  int tex_counter = 4;
  for(int i = 0; i < 4; i++) {
    
    texture* diffuse = ls->surface_types[i].near_texture;
    texture* diffuse_nm = ls->surface_types[i].near_texture_nm;
    texture* diffuse_far = ls->surface_types[i].far_texture;
    texture* diffuse_far_nm = ls->surface_types[i].far_texture_nm;
    
    if (diffuse == NULL) continue;
    
    sprintf(diffuse_name, "surface_diffuse%i", i);
    sprintf(normals_name, "surface_normals%i", i);
    sprintf(diffuse_far_name, "surface_diffuse_far%i", i);
    sprintf(normals_far_name, "surface_normals_far%i", i);
    
    glActiveTexture(GL_TEXTURE0 + tex_counter );
    glBindTexture(GL_TEXTURE_2D, texture_handle(diffuse));
    glEnable(GL_TEXTURE_2D);
    glUniform1i(glGetUniformLocation(terrain_handle, diffuse_name), tex_counter);
    tex_counter++;
    
    glActiveTexture(GL_TEXTURE0 + tex_counter );
    glBindTexture(GL_TEXTURE_2D, texture_handle(diffuse_nm));
    glEnable(GL_TEXTURE_2D);
    glUniform1i(glGetUniformLocation(terrain_handle, normals_name), tex_counter);
    tex_counter++;
    
    glActiveTexture(GL_TEXTURE0 + tex_counter );
    glBindTexture(GL_TEXTURE_2D, texture_handle(diffuse_far));
    glEnable(GL_TEXTURE_2D);
    glUniform1i(glGetUniformLocation(terrain_handle, diffuse_far_name), tex_counter);
    tex_counter++;
    
    glActiveTexture(GL_TEXTURE0 + tex_counter );
    glBindTexture(GL_TEXTURE_2D, texture_handle(diffuse_far_nm));
    glEnable(GL_TEXTURE_2D);
    glUniform1i(glGetUniformLocation(terrain_handle, normals_far_name), tex_counter);
    tex_counter++;
  }
  
  for(int i = 0; i < ls->terrain->num_chunks; i++) {
    
    terrain_chunk* tc = ls->terrain->chunks[i];
    
    vector3 position = v3_add(v3((tc->x+0.5) * ls->terrain->chunk_width, 0, (tc->y+0.5) * ls->terrain->chunk_height), ls->position);
    
    vector3 camera_pos = v3_normalize(v3_sub(position, CAMERA->position));
    vector3 camera_dir = v3_normalize(v3_sub(CAMERA->target, CAMERA->position));
    float angle = v3_dot(camera_pos, camera_dir);
    
    if (angle < -CAMERA->fov) continue;
    
    int index_id = min(0.01 * v3_dist_manhattan(position, CAMERA->position), NUM_TERRAIN_BUFFERS-1);
    
    glBindBuffer(GL_ARRAY_BUFFER, tc->vertex_buffer);
  
    glVertexPointer(3, GL_FLOAT, 0, (void*)0);
    glEnableClientState(GL_VERTEX_ARRAY);
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tc->index_buffers[index_id]);
      glDrawElements(GL_TRIANGLES, tc->num_indicies[index_id], GL_UNSIGNED_INT, (void*)0);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    
  }
  
  tex_counter--;
  while(tex_counter >= 0) {
    glActiveTexture(GL_TEXTURE0 + tex_counter);
    glDisable(GL_TEXTURE_2D);
    tex_counter--;
  }
  
  glUseProgram(0);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
}


