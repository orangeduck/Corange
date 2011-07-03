#include <string.h>
#include <time.h>

#define GLEW_STATIC
#include "GL/glew.h"

#define NO_SDL_GLEXT
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "viewport.h"

#include "camera.h"
#include "matrix.h"
#include "geometry.h"
#include "shader.h"
#include "font.h"
#include "texture.h"
#include "dictionary.h"
#include "asset_manager.h"

#include "painting_renderer.h"

static shader_program* PAINTING_PROG;

static camera* CAMERA = NULL;

static texture* BACKGROUND;

static float proj_matrix[16];
static float view_matrix[16];
static float world_matrix[16];

static float* EYE_POSITION;
static float* LIGHT_POSITION;

static float* DIFFUSE_LIGHT;
static float* SPECULAR_LIGHT;
static float* AMBIENT_LIGHT;

static int TANGENT;
static int BINORMAL;
static int COLOR;

static int FACE_POSITION;
static int FACE_NORMAL;
static int FACE_TANGENT;

static GLuint fbo = 0;
static GLuint diffuse_buffer;
static GLuint depth_buffer;

static GLuint diffuse_texture;
static GLuint depth_texture;

void painting_renderer_init() {
  
  /* Enables */
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  
  PAINTING_PROG = asset_get("./engine/shaders/painting.prog");
  BACKGROUND = asset_get("./engine/resources/paperback.dds");
  
  FACE_POSITION = glGetAttribLocation(*PAINTING_PROG, "face_position");
  FACE_NORMAL = glGetAttribLocation(*PAINTING_PROG, "face_normal");  
  FACE_TANGENT = glGetAttribLocation(*PAINTING_PROG, "face_tangent");
  
  EYE_POSITION = malloc(sizeof(float) * 3);
  LIGHT_POSITION = malloc(sizeof(float) * 3);
  
  LIGHT_POSITION[0] = 150.0f; LIGHT_POSITION[1] = 250.0f; LIGHT_POSITION[2] = 0.0f;
  
  DIFFUSE_LIGHT = malloc(sizeof(float) * 3);
  SPECULAR_LIGHT = malloc(sizeof(float) * 3);
  AMBIENT_LIGHT = malloc(sizeof(float) * 3);
  
  DIFFUSE_LIGHT[0] = 1.0f; DIFFUSE_LIGHT[1] = 1.0f; DIFFUSE_LIGHT[2] = 1.0f;
  SPECULAR_LIGHT[0] = 1.0f; SPECULAR_LIGHT[1] = 1.0f; SPECULAR_LIGHT[2] = 1.0f;
  AMBIENT_LIGHT[0] = 0.5f; AMBIENT_LIGHT[1] = 0.5f; AMBIENT_LIGHT[2] = 0.5f;
  
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  
  glGenRenderbuffers(1, &depth_buffer);
  glGenRenderbuffers(1, &diffuse_buffer);
  
  glBindRenderbuffer(GL_RENDERBUFFER, diffuse_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, viewport_width(), viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, diffuse_buffer);   
  
  glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, viewport_width(), viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);  
  
  glGenTextures(1, &diffuse_texture);
  glBindTexture(GL_TEXTURE_2D, diffuse_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport_width(), viewport_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, diffuse_texture, 0);
  
  glGenTextures(1, &depth_texture);
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, viewport_width(), viewport_height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
  
}

void painting_renderer_finish() {  
  
}

void painting_renderer_set_camera(camera* c) {
  CAMERA = c;
}

void painting_renderer_begin_render() {
  
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  
  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
  glClearDepth(1.0f);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers(1, buffers);
  
  painting_renderer_setup_camera();
  
}

void painting_renderer_setup_camera() {

  /* Load camera data */
  if (CAMERA != NULL) {
    
    matrix_4x4 viewm = camera_view_matrix(CAMERA);
    matrix_4x4 projm = camera_proj_matrix(CAMERA, viewport_ratio() );
    
    m44_to_array(viewm, view_matrix);
    m44_to_array(projm, proj_matrix);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(view_matrix);
    
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(proj_matrix);    
  }
  
}

void painting_renderer_end_render() {
  
}

void painting_renderer_begin_painting() {

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
  glClearDepth(1.0f);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  
  glUseProgramObjectARB(*PAINTING_PROG);
  
}

void painting_renderer_end_painting() {

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgramObjectARB(0);

  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glDisable(GL_LIGHTING);
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_ZERO, GL_SRC_COLOR);
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, *BACKGROUND);
  glEnable(GL_TEXTURE_2D);
  
	glBegin(GL_QUADS);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f); glVertex3f(-1.0, -1.0,  0.0f);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f); glVertex3f(1.0, -1.0,  0.0f);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 1.0f); glVertex3f(1.0,  1.0,  0.0f);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f); glVertex3f(-1.0,  1.0,  0.0f);
	glEnd();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  
}

void painting_renderer_render_renderable(painting_renderable* pr) {
  
  renderable* r = pr->renderable;
  
  matrix_4x4 r_world_matrix = m44_world( r->position, r->scale, r->rotation );
  m44_to_array(r_world_matrix, world_matrix);
  
  int i;
  for(i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
        
    painting_renderer_use_material(s->base);    
    //forward_renderer_use_material(s->instance);
    
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
    
    glVertexAttribPointer(COLOR, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 14));
    glEnableVertexAttribArray(COLOR);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
    glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);  
    
    glDisableVertexAttribArray(TANGENT);
    glDisableVertexAttribArray(BINORMAL);
    glDisableVertexAttribArray(COLOR);  
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /* DISABLE PROGRAM */
    glUseProgramObjectARB(0);

  }
  
  glPopMatrix();
  
}

void painting_renderer_paint_renderable(painting_renderable* pr) {

  GLint world_matrix_u = glGetUniformLocation(*PAINTING_PROG, "world_matrix");
  glUniformMatrix4fv(world_matrix_u, 1, 0, world_matrix);
  
  GLint proj_matrix_u = glGetUniformLocation(*PAINTING_PROG, "proj_matrix");
  glUniformMatrix4fv(proj_matrix_u, 1, 0, proj_matrix);
  
  GLint view_matrix_u = glGetUniformLocation(*PAINTING_PROG, "view_matrix");
  glUniformMatrix4fv(view_matrix_u, 1, 0, view_matrix);
  
  GLint eye_position = glGetUniformLocation(*PAINTING_PROG, "eye_position");
  v3_to_array(CAMERA->position, EYE_POSITION);
  glUniform3fv(eye_position, 1, EYE_POSITION);
  
  glUniform1i(glGetUniformLocation(*PAINTING_PROG, "background_color"), 0);
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, diffuse_texture);
  
  glUniform1i(glGetUniformLocation(*PAINTING_PROG, "background_depth"), 1);
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  
  glBindBuffer(GL_ARRAY_BUFFER, pr->position_vbo);
  glVertexPointer(3, GL_FLOAT, 0, 0);
  glEnableClientState(GL_VERTEX_ARRAY);
  
  glBindBuffer(GL_ARRAY_BUFFER, pr->uvs_vbo);
  glTexCoordPointer(2, GL_FLOAT, 0, 0);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  
  glBindBuffer(GL_ARRAY_BUFFER, pr->face_position_vbo);
  glVertexAttribPointer(FACE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(FACE_POSITION);
  
  glBindBuffer(GL_ARRAY_BUFFER, pr->face_normal_vbo);
  glVertexAttribPointer(FACE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(FACE_NORMAL);
  
  glBindBuffer(GL_ARRAY_BUFFER, pr->face_tangent_vbo);
  glVertexAttribPointer(FACE_TANGENT, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(FACE_TANGENT);
  
  GLint density = glGetUniformLocation(*PAINTING_PROG, "density");
  glUniform1f(density, pr->density * pr->density);
  
  int skip = v3_length(CAMERA->position) / (pr->density * 600) - 1;
  skip = skip > (pr->num_index_vbos-1) ? (pr->num_index_vbos-1) : skip;
  
  int skip2 =  skip + 2;
  skip2 = skip2 > (pr->num_index_vbos-1) ? (pr->num_index_vbos-1) : skip2;
  
  //printf("Skip: %i, Skip2: %i\n", skip, skip2);
  
  GLint opacity = glGetUniformLocation(*PAINTING_PROG, "opacity");
  GLint skip_u = glGetUniformLocation(*PAINTING_PROG, "skip");
  GLint size = glGetUniformLocation(*PAINTING_PROG, "size");
  
  /* First Pass */
  
  /*
  
  glUniform1f(opacity, 0.5);
  
  glUniform1i(glGetUniformLocation(*PAINTING_PROG, "brush"), 2);
  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_2D, *pr->big_brush);
  
  glUniform1f(skip_u, (float)skip2+1);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pr->index_vbos[skip2]);
  glDrawElements(GL_QUADS, (pr->num_particles / (skip + 1)) * 4, GL_UNSIGNED_INT, 0);
  
  */
  
  /* Detail pass */
  
  glUniform1f(opacity, 1.0);
  glUniform2f(size, pr->brush_size.x, pr->brush_size.y);
  
  glUniform1i(glGetUniformLocation(*PAINTING_PROG, "brush"), 2);
  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_2D, *pr->brush);
  
  glUniform1f(skip_u, (float)skip+1);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pr->index_vbos[skip]);
  glDrawElements(GL_QUADS, (pr->num_particles / (skip + 1)) * 4, GL_UNSIGNED_INT, 0);
  
  
}

void painting_renderer_use_material(material* mat) {

  shader_program* prog = dictionary_get(mat->properties, "program");
  
  glUseProgramObjectARB(*prog);
  
  /* Set global parameters */
  
  TANGENT = glGetAttribLocation(*prog, "tangent");
  BINORMAL = glGetAttribLocation(*prog, "binormal");
  COLOR = glGetAttribLocation(*prog, "color");
    
  GLint light_position = glGetUniformLocation(*prog, "light_position");
  GLint eye_position = glGetUniformLocation(*prog, "eye_position");
  
  GLint diffuse_light = glGetUniformLocation(*prog, "diffuse_light");
  GLint ambient_light = glGetUniformLocation(*prog, "ambient_light");
  GLint specular_light = glGetUniformLocation(*prog, "specular_light");
  
  v3_to_array(CAMERA->position, EYE_POSITION);
  
  glUniform3fv(light_position, 1, LIGHT_POSITION);
  glUniform3fv(eye_position, 1, EYE_POSITION);
  
  glUniform3fv(diffuse_light, 1, DIFFUSE_LIGHT);
  glUniform3fv(specular_light, 1, SPECULAR_LIGHT);
  glUniform3fv(ambient_light, 1, AMBIENT_LIGHT);

  GLint world_matrix_u = glGetUniformLocation(*prog, "world_matrix");
  glUniformMatrix4fv(world_matrix_u, 1, 0, world_matrix);
  
  GLint proj_matrix_u = glGetUniformLocation(*prog, "proj_matrix");
  glUniformMatrix4fv(proj_matrix_u, 1, 0, proj_matrix);
  
  GLint view_matrix_u = glGetUniformLocation(*prog, "view_matrix");
  glUniformMatrix4fv(view_matrix_u, 1, 0, view_matrix);
  
  /* Set material parameters */
  
  int tex_counter = 0;
  
  int i;
  for(i = 0; i < mat->keys->num_items; i++) {
    char* key = list_get(mat->keys, i);
    
    int* type = dictionary_get(mat->types, key);
    void* property = dictionary_get(mat->properties, key);
    
    GLint loc = glGetUniformLocation(*prog, key);
    
    if (*type == mat_type_texture) {
    
      glUniform1i(loc, tex_counter);
      glActiveTexture(GL_TEXTURE0 + tex_counter);
      glBindTexture(GL_TEXTURE_2D, *((texture*)property));
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

}

