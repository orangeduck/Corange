#include <string.h>

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_local.h"

#include "viewport.h"

#include "camera.h"
#include "matrix.h"
#include "geometry.h"
#include "shader.h"
#include "texture.h"
#include "dictionary.h"
#include "timing.h"

#include "forward_renderer.h"

static int use_shadows = 0;

static camera* CAMERA = NULL;
static light* LIGHT = NULL;
static texture* SHADOW_TEX = NULL;

static float proj_matrix[16];
static float view_matrix[16];
static float world_matrix[16];
static float lview_matrix[16];
static float lproj_matrix[16];

static float timer = 0.0;

static int TANGENT;
static int BINORMAL;
static int COLOR;

void forward_renderer_init() {
  
  /* Enables */
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  
  glClearDepth(1.0f);
  
}

void forward_renderer_finish() {  
  
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

void forward_renderer_begin() {
  
  glClear(GL_DEPTH_BUFFER_BIT);
  
  forward_renderer_setup_camera();
  
  timer += frame_time();
}

void forward_renderer_setup_camera() {

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
  
}

void forward_renderer_render_model(render_model* m, material* mat) {
  
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  
    int i;
    for(i=0; i < m->num_meshes; i++) {
      
      render_mesh* me = m->meshes[i];
      
      forward_renderer_use_material(mat);
      
      glEnableVertexAttribArray(TANGENT);
      glEnableVertexAttribArray(BINORMAL);
      glEnableVertexAttribArray(COLOR);
      
      glVertexPointer(3, GL_FLOAT, 0, me->vertex_positions);
      glNormalPointer(GL_FLOAT, 0, me->vertex_normals);
      glTexCoordPointer(2, GL_FLOAT, 0, me->vertex_uvs);
      
      glVertexAttribPointer(TANGENT, 3, GL_FLOAT, GL_TRUE, 0, me->vertex_tangents);
      glVertexAttribPointer(BINORMAL, 3, GL_FLOAT, GL_TRUE, 0, me->vertex_binormals);
      glVertexAttribPointer(COLOR, 4, GL_FLOAT, GL_TRUE, 0, me->vertex_colors);
      
      glDrawElements(GL_TRIANGLES, me->num_triangles_3, GL_UNSIGNED_INT, me->triangles);
  
      glDisableVertexAttribArray(TANGENT);
      glDisableVertexAttribArray(BINORMAL);
      glDisableVertexAttribArray(COLOR);  
  
      /* DISABLE PROGRAM */
      glUseProgram(0);
  
    }
    
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

}

void forward_renderer_render_renderable(renderable* r) {
  
  matrix_4x4 r_world_matrix = m44_world( r->position, r->scale, r->rotation );
  m44_to_array(r_world_matrix, world_matrix);
  
  int i;
  for(i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
        
    forward_renderer_use_material(s->base);    
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
    glUseProgram(0);

  }
  
}

void forward_renderer_use_material(material* mat) {

  shader_program* prog = dictionary_get(mat->properties, "program");
  
  glUseProgram(*prog);
  
  /* Set global parameters */
  
  TANGENT = glGetAttribLocation(*prog, "tangent");
  BINORMAL = glGetAttribLocation(*prog, "binormal");
  COLOR = glGetAttribLocation(*prog, "color");
    
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
  
  if ( use_shadows ) {
  
    GLint shadow_map = glGetUniformLocation(*prog, "shadow_map");
    glUniform1i(shadow_map, tex_counter);
    glActiveTexture(GL_TEXTURE0 + tex_counter);
    glBindTexture(GL_TEXTURE_2D, *SHADOW_TEX);
    tex_counter++;
  
  }

}

