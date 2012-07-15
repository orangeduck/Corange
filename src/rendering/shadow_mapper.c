#include <math.h>

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_local.h"

#include "error.h"

#include "assets/shader.h"

#include "graphics_manager.h"
#include "asset_manager.h"

#include "rendering/shadow_mapper.h"

static material* depth_mat;
static material* depth_mat_animated;
static texture* texture_ptr;

static GLuint fbo;
static GLuint depth_buffer;
static GLuint depth_texture;

static light* LIGHT;

static float proj_matrix[16];
static float view_matrix[16];
static float world_matrix[16];

static int BONE_INDICIES;
static int BONE_WEIGHTS;

void shadow_mapper_init(light* l) {

  LIGHT = l;
  
  depth_mat = asset_load_get("$CORANGE/shaders/depth.mat");
  depth_mat_animated = asset_load_get("$CORANGE/shaders/depth_animated.mat");
  
  shader_program* depth_shader_animated = dictionary_get(depth_mat_animated->properties, "program");
  
  BONE_INDICIES = glGetAttribLocation(*depth_shader_animated, "bone_indicies");
  BONE_WEIGHTS = glGetAttribLocation(*depth_shader_animated, "bone_weights");
  
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  
  int width = l->shadow_map_width;
  int height = l->shadow_map_height;
  
  glGenRenderbuffers(1, &depth_buffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);  
  
  glGenTextures(1, &depth_texture);
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
  
  texture_ptr = malloc(sizeof(texture));
  *texture_ptr = depth_texture;
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
}

void shadow_mapper_finish() {
  
  texture_delete(texture_ptr);
  
  glDeleteFramebuffers(1, &fbo);
  
  glDeleteRenderbuffers(1, &depth_buffer);
  glDeleteTextures(1,&depth_texture);
  
}

static void shadow_mapper_setup_camera() {
  
  matrix_4x4 viewm = light_view_matrix(LIGHT);
  matrix_4x4 projm = light_proj_matrix(LIGHT);
  
  m44_to_array(viewm, view_matrix);
  m44_to_array(projm, proj_matrix);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(view_matrix);
  
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(proj_matrix);
  
}

void shadow_mapper_begin() {
  
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  
  glClearDepth(1.0f);
  glClear(GL_DEPTH_BUFFER_BIT);
  
  glViewport( 0, 0, LIGHT->shadow_map_width, LIGHT->shadow_map_height);
  
  shadow_mapper_setup_camera();
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
  
}

void shadow_mapper_end() {
  
  glCullFace(GL_BACK);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  
  glViewport( 0, 0, graphics_viewport_width(), graphics_viewport_height());
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
}

void shadow_mapper_render_static(static_object* s) {
  
  if (!s->cast_shadows) {
    return;
  }
  
  matrix_4x4 r_world_matrix = m44_world( s->position, s->scale, s->rotation );
  m44_to_array(r_world_matrix, world_matrix);
  
  shader_program* depth_shader = dictionary_get(depth_mat->properties, "program");
  
  glUseProgram(*depth_shader);
  
  GLint world_matrix_u = glGetUniformLocation(*depth_shader, "world_matrix");
  glUniformMatrix4fv(world_matrix_u, 1, 0, world_matrix);
  
  GLint proj_matrix_u = glGetUniformLocation(*depth_shader, "proj_matrix");
  glUniformMatrix4fv(proj_matrix_u, 1, 0, proj_matrix);
  
  GLint view_matrix_u = glGetUniformLocation(*depth_shader, "view_matrix");
  glUniformMatrix4fv(view_matrix_u, 1, 0, view_matrix);
  
  GLint alpha_test_u = glGetUniformLocation(*depth_shader, "alpha_test");
  GLint diffuse_u = glGetUniformLocation(*depth_shader, "diffuse");
  
  renderable* r = s->renderable;
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    
    if(s->is_rigged) {
      error("Static Object is rigged!");
    }
    
    float* alpha_test = dictionary_get(s->base->properties, "alpha_test");
    if (alpha_test != NULL) {
      glUniform1f(alpha_test_u, *alpha_test);
    } else {
      glUniform1f(alpha_test_u, 0.0);
    }
    
    texture* diffuse_texture = dictionary_get(s->base->properties, "diffuse_texture");
    if (diffuse_texture != NULL) {
      glUniform1i(diffuse_u, 0);
      glActiveTexture(GL_TEXTURE0 + 0);
      glBindTexture(GL_TEXTURE_2D, *diffuse_texture);
      glEnable(GL_TEXTURE_2D);
    }
  
    GLsizei stride = sizeof(float) * 18;
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
        
    glVertexPointer(3, GL_FLOAT, stride, (void*)0);
    glEnableClientState(GL_VERTEX_ARRAY);
    
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    if (diffuse_texture != NULL) {
      glActiveTexture(GL_TEXTURE0 + 0);
      glDisable(GL_TEXTURE_2D);
    }

  }
  
  glUseProgram(0);
  
}

#define MAX_BONES 32
static matrix_4x4 bone_matrices[MAX_BONES];
static float bone_matrix_data[4 * 4 * MAX_BONES];

void shadow_mapper_render_animated(animated_object* ao) {
  
  if (ao->skeleton->num_bones > MAX_BONES) {
    error("animated object skeleton has too many bones (over %i)", MAX_BONES);
  }
  
  matrix_4x4 r_world_matrix = m44_world( ao->position, ao->scale, ao->rotation );
  m44_to_array(r_world_matrix, world_matrix);
  
  for(int i = 0; i < ao->skeleton->num_bones; i++) {
    matrix_4x4 base, ani;
    base = bone_transform(ao->skeleton->bones[i]);
    ani = bone_transform(ao->pose->bones[i]);
    
    bone_matrices[i] = m44_mul_m44(ani, m44_inverse(base));
    m44_to_array(bone_matrices[i], bone_matrix_data + (i * 4 * 4));
  }
  
  shader_program* depth_shader_animated = dictionary_get(depth_mat_animated->properties, "program");
  
  glUseProgram(*depth_shader_animated);
  
  GLint bone_world_matrices_u = glGetUniformLocation(*depth_shader_animated, "bone_world_matrices");
  glUniformMatrix4fv(bone_world_matrices_u, ao->skeleton->num_bones, GL_FALSE, bone_matrix_data);
  
  GLint bone_count_u = glGetUniformLocation(*depth_shader_animated, "bone_count");
  glUniform1i(bone_count_u, ao->skeleton->num_bones);
  
  GLint world_matrix_u = glGetUniformLocation(*depth_shader_animated, "world_matrix");
  glUniformMatrix4fv(world_matrix_u, 1, 0, world_matrix);
  
  GLint proj_matrix_u = glGetUniformLocation(*depth_shader_animated, "proj_matrix");
  glUniformMatrix4fv(proj_matrix_u, 1, 0, proj_matrix);
  
  GLint view_matrix_u = glGetUniformLocation(*depth_shader_animated, "view_matrix");
  glUniformMatrix4fv(view_matrix_u, 1, 0, view_matrix);
  
  renderable* r = ao->renderable;
  
  for(int i = 0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    if(s->is_rigged) {
    
      GLsizei stride = sizeof(float) * 24;
      
      glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
          
      glVertexPointer(3, GL_FLOAT, stride, (void*)0);
      glEnableClientState(GL_VERTEX_ARRAY);
      
      glVertexAttribPointer(BONE_INDICIES, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 18));
      glEnableVertexAttribArray(BONE_INDICIES);
      
      glVertexAttribPointer(BONE_WEIGHTS, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 21));
      glEnableVertexAttribArray(BONE_WEIGHTS);
      
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
        glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
      
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableVertexAttribArray(BONE_INDICIES);  
      glDisableVertexAttribArray(BONE_WEIGHTS);  
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      
    } else {
      error("animated object is not rigged");
    }

  }
  
  glUseProgram(0);
  
}

void shadow_mapper_render_landscape(landscape* ls) {
  
  matrix_4x4 r_world_matrix = m44_world( ls->position, ls->scale, ls->rotation );
  m44_to_array(r_world_matrix, world_matrix);
  
  shader_program* depth_shader = dictionary_get(depth_mat->properties, "program");
  
  glUseProgram(*depth_shader);
  
  GLint world_matrix_u = glGetUniformLocation(*depth_shader, "world_matrix");
  glUniformMatrix4fv(world_matrix_u, 1, 0, world_matrix);
  
  GLint proj_matrix_u = glGetUniformLocation(*depth_shader, "proj_matrix");
  glUniformMatrix4fv(proj_matrix_u, 1, 0, proj_matrix);
  
  GLint view_matrix_u = glGetUniformLocation(*depth_shader, "view_matrix");
  glUniformMatrix4fv(view_matrix_u, 1, 0, view_matrix);
  
  GLint alpha_test_u = glGetUniformLocation(*depth_shader, "alpha_test");
  glUniform1f(alpha_test_u, 0.0);
  
  for(int i = 0; i < ls->terrain->num_chunks; i++) {
    
    terrain_chunk* tc = ls->terrain->chunks[i];
    
    glBindBuffer(GL_ARRAY_BUFFER, tc->vertex_buffer);
  
    glVertexPointer(3, GL_FLOAT, 0, (void*)0);
    glEnableClientState(GL_VERTEX_ARRAY);
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tc->index_buffers[NUM_TERRAIN_BUFFERS-2]);
      glDrawElements(GL_TRIANGLES, tc->num_indicies[NUM_TERRAIN_BUFFERS-2], GL_UNSIGNED_INT, (void*)0);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    
  }
  
  glUseProgram(0);

}

texture* shadow_mapper_depth_texture() {
  return texture_ptr;
}
