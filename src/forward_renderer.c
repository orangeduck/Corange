#include <string.h>

#define GLEW_STATIC
#include "GL/glew.h"

#define NO_SDL_GLEXT
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "camera.h"
#include "matrix.h"
#include "geometry.h"
#include "shader.h"
#include "font.h"
#include "texture.h"
#include "logger.h"
#include "dictionary.h"

#include "forward_renderer.h"

static camera* CAMERA = NULL;

static float proj_matrix[16];
static float view_matrix[16];

static int WIDTH;
static int HEIGHT;

static float* EYE_POSITION;
static float* LIGHT_POSITION;

static float* DIFFUSE_LIGHT;
static float* SPECULAR_LIGHT;
static float* AMBIENT_LIGHT;

static int TANGENT;
static int BINORMAL;
static int COLOR;

static float ASPECT_RATIO(){
  return (float)HEIGHT / (float)WIDTH;
}

void forward_renderer_init(int width, int height) {
  
  WIDTH = width;
  HEIGHT = height;
  
  /* Clear Colors */
  glClearColor(1.0f, 0.769f, 0.0f, 0.0f);
  glClearDepth(1.0f);
  
  /* Enables */
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  
  EYE_POSITION = malloc(sizeof(float) * 3);
  LIGHT_POSITION = malloc(sizeof(float) * 3);
  
  LIGHT_POSITION[0] = 150.0f; LIGHT_POSITION[1] = 250.0f; LIGHT_POSITION[2] = 0.0f;
  
  DIFFUSE_LIGHT = malloc(sizeof(float) * 3);
  SPECULAR_LIGHT = malloc(sizeof(float) * 3);
  AMBIENT_LIGHT = malloc(sizeof(float) * 3);
  
  DIFFUSE_LIGHT[0] = 1.0f; DIFFUSE_LIGHT[1] = 1.0f; DIFFUSE_LIGHT[2] = 1.0f;
  SPECULAR_LIGHT[0] = 1.0f; SPECULAR_LIGHT[1] = 1.0f; SPECULAR_LIGHT[2] = 1.0f;
  AMBIENT_LIGHT[0] = 0.5f; AMBIENT_LIGHT[1] = 0.5f; AMBIENT_LIGHT[2] = 0.5f;
  
}

void forward_renderer_finish() {  
  
}

void forward_renderer_set_camera(camera* c) {
  CAMERA = c;
}

void forward_renderer_set_dimensions(int width, int height) {
  WIDTH = width;
  HEIGHT = height;
  glViewport(0, 0, width, height);
}

void forward_renderer_begin() {
  
  forward_renderer_setup_camera();
  
  /* Clear Backbuffer */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

void forward_renderer_setup_camera() {

  /* Load camera data */
  if (CAMERA != NULL) {
    
    matrix_4x4 viewm = camera_view_matrix(CAMERA);
    matrix_4x4 projm = camera_proj_matrix(CAMERA, ASPECT_RATIO() );
    
    m44_to_array(viewm, view_matrix);
    m44_to_array(projm, proj_matrix);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(view_matrix);
    
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(proj_matrix);
  
  }
  
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
      glUseProgramObjectARB(0);
  
    }
    
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

}

void forward_renderer_use_material(material* mat) {

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

