#define GLEW_STATIC
#include "GL/glew.h"

#define NO_SDL_GLEXT
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "shader.h"
#include "viewport.h"
#include "asset_manager.h"

#include "shadow_mapper.h"

static shader_program* depth_shader;
static texture* texture_ptr;

static GLuint fbo;

static GLuint depth_buffer;
static GLuint color_buffer;

static GLuint color_texture;
static GLuint depth_texture;

static light* LIGHT;
static light* LIGHT;

static texture* texture_ptr;

static float proj_matrix[16];
static float view_matrix[16];
static float world_matrix[16];

void shadow_mapper_init(light* l) {

  LIGHT = l;
  
  depth_shader = asset_get("./engine/shaders/depth.prog");
    
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  
  glGenRenderbuffers(1, &depth_buffer);
  glGenRenderbuffers(1, &color_buffer); 
  
  int width = l->shadow_map_width;
  int height = l->shadow_map_height;
  
  glBindRenderbuffer(GL_RENDERBUFFER, color_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_buffer);   
  
  glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);  
  
  glGenTextures(1, &color_texture);
  glBindTexture(GL_TEXTURE_2D, color_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture, 0);
  
  glGenTextures(1, &depth_texture);
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  texture_ptr = malloc(sizeof(texture));
  *texture_ptr = color_texture;
  
}

void shadow_mapper_finish() {

  glDeleteFramebuffers(1, &fbo);
  
  glDeleteRenderbuffers(1, &depth_buffer);
  glDeleteRenderbuffers(1, &color_buffer);
  glDeleteTextures(1,&depth_texture);
  glDeleteTextures(1,&color_texture);
  
}

void shadow_mapper_begin() {

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClearDepth(1.0f);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  glViewport( 0, 0, LIGHT->shadow_map_width, LIGHT->shadow_map_height);
  glDisable(GL_LIGHTING);
  
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  
  shadow_mapper_setup_camera();
  
  glCullFace(GL_FRONT);
  
}

void shadow_mapper_setup_camera() {
  
  //matrix_4x4 viewm = camera_view_matrix(CAMERA);
  //matrix_4x4 projm = camera_proj_matrix(CAMERA, viewport_ratio());
  matrix_4x4 viewm = light_view_matrix(LIGHT);
  matrix_4x4 projm = light_proj_matrix(LIGHT);
  
  m44_to_array(viewm, view_matrix);
  m44_to_array(projm, proj_matrix);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(view_matrix);
  
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(proj_matrix);    
  
}

void shadow_mapper_end() {

  glCullFace(GL_FRONT);

  glViewport( 0, 0, viewport_width(), viewport_height());
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
  glClearDepth(1.0f);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  glDisable(GL_LIGHTING);
  
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
  
	glBegin(GL_QUADS);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f); glVertex3f(-0.9, -0.9,  0.0f);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f); glVertex3f(0.9, -0.9,  0.0f);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 1.0f); glVertex3f(0.9,  0.9,  0.0f);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f); glVertex3f(-0.9,  0.9,  0.0f);
	glEnd();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
}

void shadow_mapper_render_renderable(renderable* r) {
  
  matrix_4x4 r_world_matrix = m44_world( r->position, r->scale, r->rotation );
  m44_to_array(r_world_matrix, world_matrix);
  
  glUseProgramObjectARB(*depth_shader);
  
  int i;
  for(i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    
    GLsizei stride = sizeof(float) * 18;
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
        
    glVertexPointer(3, GL_FLOAT, stride, (void*)0);
    glEnableClientState(GL_VERTEX_ARRAY);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
    glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

  }
  
  glUseProgramObjectARB(0);
  
}

texture* shadow_mapper_depth_texture() {
  return texture_ptr;
}