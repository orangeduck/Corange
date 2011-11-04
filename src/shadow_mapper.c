#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_local.h"

#include "shader.h"
#include "renderable.h"
#include "viewport.h"
#include "asset_manager.h"

#include "shadow_mapper.h"

static shader_program* depth_shader;
static texture* texture_ptr;

static GLuint fbo;
static GLuint depth_buffer;
static GLuint depth_texture;

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
  
  int width = l->shadow_map_width;
  int height = l->shadow_map_height;
  
  glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);  
  
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
  *texture_ptr = depth_texture;
  
}

void shadow_mapper_finish() {

  glDeleteFramebuffers(1, &fbo);
  
  glDeleteRenderbuffers(1, &depth_buffer);
  glDeleteTextures(1,&depth_texture);
  
}

void shadow_mapper_begin() {

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClearDepth(1.0f);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
  glViewport( 0, 0, LIGHT->shadow_map_width, LIGHT->shadow_map_height);
  glDisable(GL_LIGHTING);
  
  shadow_mapper_setup_camera();
  
  glEnable(GL_CULL_FACE);
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
  
  glCullFace(GL_BACK);
  glDisable(GL_CULL_FACE);

  glViewport( 0, 0, viewport_width(), viewport_height());
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
}

void shadow_mapper_render_static(static_object* s) {
  
  matrix_4x4 r_world_matrix = m44_world( s->position, s->scale, s->rotation );
  m44_to_array(r_world_matrix, world_matrix);
  
  glUseProgram(*depth_shader);
  
  GLint world_matrix_u = glGetUniformLocation(*depth_shader, "world_matrix");
  glUniformMatrix4fv(world_matrix_u, 1, 0, world_matrix);
  
  GLint proj_matrix_u = glGetUniformLocation(*depth_shader, "proj_matrix");
  glUniformMatrix4fv(proj_matrix_u, 1, 0, proj_matrix);
  
  GLint view_matrix_u = glGetUniformLocation(*depth_shader, "view_matrix");
  glUniformMatrix4fv(view_matrix_u, 1, 0, view_matrix);
  
  renderable* r = s->renderable;
  
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
  
  glUseProgram(0);
  
}

texture* shadow_mapper_depth_texture() {
  return texture_ptr;
}