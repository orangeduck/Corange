#include "rendering/shadow_mapper.h"

#include "cgraphics.h"
#include "casset.h"

#include "assets/shader.h"
#include "assets/material.h"
#include "assets/renderable.h"
#include "assets/terrain.h"

static asset_hndl depth_mat;
static asset_hndl depth_mat_animated;

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
  
  depth_mat = asset_hndl_new_load(P("$CORANGE/shaders/depth.mat"));
  depth_mat_animated = asset_hndl_new_load(P("$CORANGE/shaders/depth_animated.mat"));
  
  shader_program* depth_shader_animated = material_get_entry(asset_hndl_ptr(depth_mat), 0)->program;
  
  BONE_INDICIES = glGetAttribLocation(shader_program_handle(depth_shader_animated), "bone_indicies");
  BONE_WEIGHTS = glGetAttribLocation(shader_program_handle(depth_shader_animated), "bone_weights");
  
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
  
  mat4 viewm = light_view_matrix(LIGHT);
  mat4 projm = light_proj_matrix(LIGHT);
  
  mat4_to_array(viewm, view_matrix);
  mat4_to_array(projm, proj_matrix);
  
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
  
  mat4 r_world_matrix = mat4_world( s->position, s->scale, s->rotation );
  mat4_to_array(r_world_matrix, world_matrix);
  
  shader_program* depth_shader = material_get_entry(asset_hndl_ptr(depth_mat), 0)->program;
  
  glUseProgram(*depth_shader);
  
  GLint world_matrix_u = glGetUniformLocation(*depth_shader, "world_matrix");
  glUniformMatrix4fv(world_matrix_u, 1, 0, world_matrix);
  
  GLint proj_matrix_u = glGetUniformLocation(*depth_shader, "proj_matrix");
  glUniformMatrix4fv(proj_matrix_u, 1, 0, proj_matrix);
  
  GLint view_matrix_u = glGetUniformLocation(*depth_shader, "view_matrix");
  glUniformMatrix4fv(view_matrix_u, 1, 0, view_matrix);
  
  GLint alpha_test_u = glGetUniformLocation(*depth_shader, "alpha_test");
  GLint diffuse_u = glGetUniformLocation(*depth_shader, "diffuse");
  
  renderable* r = asset_hndl_ptr(s->renderable);
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    
    if(s->is_rigged) {
      error("Static Object is rigged!");
    }
    
    material_entry* me = material_get_entry(asset_hndl_ptr(depth_mat), 0);
    
    if (material_entry_has_item(me, "alpha_test")) {
      float alpha_test = material_entry_item(me, "alpha_test").as_float;
      glUniform1f(alpha_test_u, alpha_test);
    } else {
      glUniform1f(alpha_test_u, 0.0);
    }
    
    if (material_entry_has_item(me, "diffuse_texture")) {
      texture* diffuse_texture = asset_hndl_ptr(material_entry_item(me, "diffuse_texture").as_asset);
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
    
    if (material_entry_has_item(me, "diffuse_texture")) {
      glActiveTexture(GL_TEXTURE0 + 0);
      glDisable(GL_TEXTURE_2D);
    }

  }
  
  glUseProgram(0);
  
}

#define MAX_BONES 32
static mat4 bone_matrices[MAX_BONES];
static float bone_matrix_data[4 * 4 * MAX_BONES];

void shadow_mapper_render_animated(animated_object* ao) {
  
  skeleton* skel = asset_hndl_ptr(ao->skeleton);
  
  if (skel->num_bones > MAX_BONES) {
    error("animated object skeleton has too many bones (over %i)", MAX_BONES);
  }
  
  mat4 r_world_matrix = mat4_world( ao->position, ao->scale, ao->rotation );
  mat4_to_array(r_world_matrix, world_matrix);
  
  for(int i = 0; i < skel->num_bones; i++) {
    mat4 base, ani;
    base = bone_transform(skel->bones[i]);
    ani = bone_transform(ao->pose->bones[i]);
    
    bone_matrices[i] = mat4_mul_mat4(ani, mat4_inverse(base));
    mat4_to_array(bone_matrices[i], bone_matrix_data + (i * 4 * 4));
  }
  
  shader_program* depth_shader_animated = material_get_entry(asset_hndl_ptr(depth_mat_animated), 0)->program;
  
  glUseProgram(*depth_shader_animated);
  
  GLint bone_world_matrices_u = glGetUniformLocation(*depth_shader_animated, "bone_world_matrices");
  glUniformMatrix4fv(bone_world_matrices_u, skel->num_bones, GL_FALSE, bone_matrix_data);
  
  GLint bone_count_u = glGetUniformLocation(*depth_shader_animated, "bone_count");
  glUniform1i(bone_count_u, skel->num_bones);
  
  GLint world_matrix_u = glGetUniformLocation(*depth_shader_animated, "world_matrix");
  glUniformMatrix4fv(world_matrix_u, 1, 0, world_matrix);
  
  GLint proj_matrix_u = glGetUniformLocation(*depth_shader_animated, "proj_matrix");
  glUniformMatrix4fv(proj_matrix_u, 1, 0, proj_matrix);
  
  GLint view_matrix_u = glGetUniformLocation(*depth_shader_animated, "view_matrix");
  glUniformMatrix4fv(view_matrix_u, 1, 0, view_matrix);
  
  renderable* r = asset_hndl_ptr(ao->renderable);
  
  for(int i = 0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    
    if(!s->is_rigged) {
      error("animated object is not rigged");
    }
    
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

  }
  
  glUseProgram(0);
  
}

void shadow_mapper_render_landscape(landscape* ls) {
  
  mat4 r_world_matrix = mat4_world( ls->position, ls->scale, ls->rotation );
  mat4_to_array(r_world_matrix, world_matrix);
  
  shader_program* depth_shader = material_get_entry(asset_hndl_ptr(depth_mat), 0)->program;
  
  glUseProgram(*depth_shader);
  
  GLint world_matrix_u = glGetUniformLocation(*depth_shader, "world_matrix");
  glUniformMatrix4fv(world_matrix_u, 1, 0, world_matrix);
  
  GLint proj_matrix_u = glGetUniformLocation(*depth_shader, "proj_matrix");
  glUniformMatrix4fv(proj_matrix_u, 1, 0, proj_matrix);
  
  GLint view_matrix_u = glGetUniformLocation(*depth_shader, "view_matrix");
  glUniformMatrix4fv(view_matrix_u, 1, 0, view_matrix);
  
  GLint alpha_test_u = glGetUniformLocation(*depth_shader, "alpha_test");
  glUniform1f(alpha_test_u, 0.0);
  
  terrain* terrain = asset_hndl_ptr(ls->terrain);
  
  for(int i = 0; i < terrain->num_chunks; i++) {
    
    terrain_chunk* tc = terrain->chunks[i];
    
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
