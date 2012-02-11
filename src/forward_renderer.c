#include <string.h>
#include <math.h>

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_local.h"

#include "error.h"

#include "viewport.h"
#include "asset_manager.h"

#include "renderable.h"
#include "camera.h"
#include "matrix.h"
#include "shader.h"
#include "texture.h"
#include "dictionary.h"
#include "timing.h"

#include "forward_renderer.h"

static int use_shadows = 0;

static camera* CAMERA = NULL;
static light* LIGHT = NULL;

static texture* SHADOW_TEX = NULL;
static texture* COLOR_CORRECTION = NULL;
static texture* VIGNETTING = NULL;

static shader_program* GRADIENT = NULL;
static shader_program* SCREEN_POST = NULL;

static float proj_matrix[16];
static float view_matrix[16];
static float world_matrix[16];
static float lview_matrix[16];
static float lproj_matrix[16];

static float timer = 0.0;

static int TANGENT;
static int BINORMAL;
static int BONE_INDICIES;
static int BONE_WEIGHTS;

static GLuint ldr_fbo;
static GLuint ldr_buffer;
static GLuint ldr_dpeth_buffer;
static GLuint ldr_texture;
static GLuint ldr_depth_texture;

void forward_renderer_init() {
  
  COLOR_CORRECTION = asset_load_get("$CORANGE/resources/identity.lut");
  VIGNETTING = asset_load_get("$CORANGE/resources/vignetting.dds");
  GRADIENT = asset_load_get("$SHADERS/gradient.prog");
  SCREEN_POST = asset_load_get("$SHADERS/deferred_post.prog");
  
  glClearColor(0.2, 0.2, 0.2, 1.0f);
  glClearDepth(1.0f);
  
  glGenFramebuffers(1, &ldr_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, ldr_fbo);
  
  glGenRenderbuffers(1, &ldr_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, ldr_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, viewport_width(), viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ldr_buffer);   
  
  glGenRenderbuffers(1, &ldr_dpeth_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, ldr_dpeth_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, viewport_width(), viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ldr_dpeth_buffer);  
  
  glGenTextures(1, &ldr_texture);
  glBindTexture(GL_TEXTURE_2D, ldr_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport_width(), viewport_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ldr_texture, 0);
  
  glGenTextures(1, &ldr_depth_texture);
  glBindTexture(GL_TEXTURE_2D, ldr_depth_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, viewport_width(), viewport_height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ldr_depth_texture, 0);
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
}

void forward_renderer_finish() {  
  
  glDeleteFramebuffers(1, &ldr_fbo);
  glDeleteRenderbuffers(1, &ldr_buffer);
  glDeleteTextures(1,&ldr_texture);
  
}

void forward_renderer_set_camera(camera* c) {
  CAMERA = c;
}

void forward_renderer_set_light(light* l) {
  LIGHT = l;
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

  glUseProgram(*GRADIENT);
  
  GLint start = glGetUniformLocation(*GRADIENT, "start");
  GLint end = glGetUniformLocation(*GRADIENT, "end");
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
  
  glBindFramebuffer(GL_FRAMEBUFFER, ldr_fbo);
  
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
  render_gradient();
  
  forward_renderer_setup_camera();
  
  glEnable(GL_DEPTH_TEST);
  
}

void forward_renderer_setup_camera() {

  if (CAMERA == NULL) {
    error("Camera not set yet!");
  }
  if (LIGHT == NULL) {
    error("Light not set yet!");
  }

  matrix_4x4 viewm = camera_view_matrix(CAMERA);
  matrix_4x4 projm = camera_proj_matrix(CAMERA, viewport_ratio() );
  
  m44_to_array(viewm, view_matrix);
  m44_to_array(projm, proj_matrix);

  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(view_matrix);
  
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(proj_matrix);
  
  /* Setup light stuff */
  
  matrix_4x4 lviewm = light_view_matrix(LIGHT);
  matrix_4x4 lprojm = light_proj_matrix(LIGHT);
  
  m44_to_array(lviewm, lview_matrix);
  m44_to_array(lprojm, lproj_matrix);
  
}

void forward_renderer_end() {
  
  glDisable(GL_DEPTH_TEST);
  
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

static int tex_counter = 0;
static void forward_renderer_use_material(material* mat) {
  
  shader_program* prog = dictionary_get(mat->properties, "program");
  
  glUseProgram(*prog);
  
  /* Set global parameters */
  
  TANGENT = glGetAttribLocation(*prog, "tangent");
  BINORMAL = glGetAttribLocation(*prog, "binormal");
  BONE_INDICIES = glGetAttribLocation(*prog, "bone_indicies");
  BONE_WEIGHTS = glGetAttribLocation(*prog, "bone_weights");
  
  GLint light_position = glGetUniformLocation(*prog, "light_position");
  GLint eye_position = glGetUniformLocation(*prog, "eye_position");
  
  GLint diffuse_light = glGetUniformLocation(*prog, "diffuse_light");
  GLint ambient_light = glGetUniformLocation(*prog, "ambient_light");
  GLint specular_light = glGetUniformLocation(*prog, "specular_light");
 
  GLint time = glGetUniformLocation(*prog, "time");
  
  glUniform3f(light_position, LIGHT->position.x, LIGHT->position.y, LIGHT->position.z);
  glUniform3f(eye_position, CAMERA->position.x, CAMERA->position.y, CAMERA->position.z);
  
  glUniform3f(diffuse_light, LIGHT->diffuse_color.r, LIGHT->diffuse_color.g, LIGHT->diffuse_color.b);
  glUniform3f(specular_light, LIGHT->specular_color.r, LIGHT->specular_color.g, LIGHT->specular_color.b);
  glUniform3f(ambient_light, LIGHT->ambient_color.r, LIGHT->ambient_color.g, LIGHT->ambient_color.b);

  glUniform1f(time,timer);
  
  GLint world_matrix_u = glGetUniformLocation(*prog, "world_matrix");
  glUniformMatrix4fv(world_matrix_u, 1, 0, world_matrix);
  
  GLint proj_matrix_u = glGetUniformLocation(*prog, "proj_matrix");
  glUniformMatrix4fv(proj_matrix_u, 1, 0, proj_matrix);
  
  GLint view_matrix_u = glGetUniformLocation(*prog, "view_matrix");
  glUniformMatrix4fv(view_matrix_u, 1, 0, view_matrix);
  
  GLint lproj_matrix_u = glGetUniformLocation(*prog, "light_proj");
  glUniformMatrix4fv(lproj_matrix_u, 1, 0, lproj_matrix);
  
  GLint lview_matrix_u = glGetUniformLocation(*prog, "light_view");
  glUniformMatrix4fv(lview_matrix_u, 1, 0, lview_matrix);
  
  /* Set material parameters */
  
  tex_counter = 0;
  
  for(int i = 0; i < mat->keys->num_items; i++) {
    char* key = list_get(mat->keys, i);
    
    int* type = dictionary_get(mat->types, key);
    void* property = dictionary_get(mat->properties, key);
    
    GLint loc = glGetUniformLocation(*prog, key);
    
    if (*type == mat_type_texture) {
    
      glUniform1i(loc, tex_counter);
      glActiveTexture(GL_TEXTURE0 + tex_counter);
      glBindTexture(GL_TEXTURE_2D, *((texture*)property));
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
  
  if ( use_shadows ) {
  
    GLint shadow_map = glGetUniformLocation(*prog, "shadow_map");
    glUniform1i(shadow_map, tex_counter);
    glActiveTexture(GL_TEXTURE0 + tex_counter);
    glBindTexture(GL_TEXTURE_2D, *SHADOW_TEX);
    glEnable(GL_TEXTURE_2D);
    tex_counter++;
  }

  
  GLuint color_correction = glGetUniformLocation(*prog, "lut");
  glUniform1i(color_correction, tex_counter);
  glActiveTexture(GL_TEXTURE0 + tex_counter );
  glBindTexture(GL_TEXTURE_3D, *COLOR_CORRECTION);
  glEnable(GL_TEXTURE_3D);
  tex_counter++;
  
}

static void forward_renderer_disuse_material() {
  
  tex_counter--;
  glActiveTexture(GL_TEXTURE0 + tex_counter );
  glDisable(GL_TEXTURE_3D);
  
  while(tex_counter > 0) {
    tex_counter--;
    glActiveTexture(GL_TEXTURE0 + tex_counter);
    glDisable(GL_TEXTURE_2D);
  }
  
  glUseProgram(0);

}

static void render_collision_mesh(collision_mesh* cm) {
  
  if (cm->is_leaf) {
    
    shader_program* collision_prog = asset_load_get("$SHADERS/collision_mesh.prog");
    glUseProgram(*collision_prog);
    
    GLint color = glGetUniformLocation(*collision_prog, "color");
    glUniform3f(color, 1, 1, 1);
    
    GLint world_matrix_u = glGetUniformLocation(*collision_prog, "world_matrix");
    glUniformMatrix4fv(world_matrix_u, 1, 0, world_matrix);
    
    GLint proj_matrix_u = glGetUniformLocation(*collision_prog, "proj_matrix");
    glUniformMatrix4fv(proj_matrix_u, 1, 0, proj_matrix);
    
    GLint view_matrix_u = glGetUniformLocation(*collision_prog, "view_matrix");
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
      
      forward_renderer_use_material(s->base);
      
      shader_program* prog = dictionary_get(s->base->properties, "program");
      GLint recieve_shadows = glGetUniformLocation(*prog, "recieve_shadows");
      glUniform1i(recieve_shadows, so->recieve_shadows);
      
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
    
    } else {
    
      forward_renderer_use_material(s->base);
      
      shader_program* prog = dictionary_get(s->base->properties, "program");
      GLint recieve_shadows = glGetUniformLocation(*prog, "recieve_shadows");
      glUniform1i(recieve_shadows, so->recieve_shadows);
      
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

  }
  
  /*
  if (so->collision_body != NULL) {
    bsp_counter = 0;
    render_bsp_mesh(so->collision_body->collision_mesh);
  }
  */
  
}

void forward_renderer_render_physics(physics_object* po) {

  matrix_4x4 r_world_matrix = m44_world( po->position, po->scale, po->rotation );
  m44_to_array(r_world_matrix, world_matrix);
  
  renderable* r = po->renderable;
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    if(s->is_rigged) {

      forward_renderer_use_material(s->base);
      
      shader_program* prog = dictionary_get(s->base->properties, "program");
      GLint recieve_shadows = glGetUniformLocation(*prog, "recieve_shadows");
      glUniform1i(recieve_shadows, po->recieve_shadows);
      
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
      
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
        glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
      
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_NORMAL_ARRAY);
      glDisableClientState(GL_COLOR_ARRAY);  
      
      glDisableVertexAttribArray(TANGENT);
      glDisableVertexAttribArray(BINORMAL);
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      
      forward_renderer_disuse_material();
    
    } else {
    
      forward_renderer_use_material(s->base);
      
      shader_program* prog = dictionary_get(s->base->properties, "program");
      GLint recieve_shadows = glGetUniformLocation(*prog, "recieve_shadows");
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
      
      GLint bone_world_matrices_u = glGetUniformLocation(*prog, "bone_world_matrices");
      glUniformMatrix4fv(bone_world_matrices_u, ao->skeleton->num_bones, GL_FALSE, bone_matrix_data);
      
      GLint bone_count_u = glGetUniformLocation(*prog, "bone_count");
      glUniform1i(bone_count_u, ao->skeleton->num_bones);
      
      GLint recieve_shadows = glGetUniformLocation(*prog, "recieve_shadows");
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
  
  vector4 x_pos = m44_mul_v4(world, v4(1,0,0,1));
  vector4 y_pos = m44_mul_v4(world, v4(0,1,0,1));
  vector4 z_pos = m44_mul_v4(world, v4(0,0,1,1));
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


