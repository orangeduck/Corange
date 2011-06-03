#include <string.h>

#define GLEW_STATIC
#include "GL/glew.h"

#define NO_SDL_GLEXT
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "camera.h"
#include "matrix.h"
#include "geometry.h"
#include "glsl.h"
#include "font.h"
#include "texture.h"

#include "renderer.h"

static camera* CAMERA = NULL;

static float proj_matrix[16];
static float view_matrix[16];

static int WIDTH;
static int HEIGHT;

static glsl_program* PROGRAM;

static texture* PIANO_DIFFUSE;
static texture* PIANO_NORMAL;
static texture* PIANO_SPECULAR;

static float* EYE_POSITION;
static float* LIGHT_POSITION;

static float* DIFFUSE_LIGHT;
static float* SPECULAR_LIGHT;
static float* AMBIENT_LIGHT;

static float GLOSSINESS = 7.0f;
static float BUMPINESS = 1.0f;
static float SPECULAR_LEVEL = 2.0f;

static int TANGENT;
static int BINORMAL;
static int COLOR;

float ASPECT_RATIO(){
  return (float)HEIGHT / (float)WIDTH;
}

void forward_renderer_init() {
  
  /* Clear Colors */
  glClearColor(1.0f, 0.769f, 0.0f, 0.0f);
  glClearDepth(1.0f);
  
  /* Enables */
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_MULTISAMPLE_ARB);
  glEnable(GL_DEPTH_TEST);
  
  /* Loading testing stuff */
  PROGRAM = (glsl_program*)glsl_load_shaders("./Engine/Assets/Shaders/normal_spec.vs","./Engine/Assets/Shaders/normal_spec.fs");
  
  PIANO_DIFFUSE = (texture*)dds_load_file("./Engine/Assets/Textures/piano.dds");
  PIANO_NORMAL = (texture*)dds_load_file("./Engine/Assets/Textures/piano_nm.dds");
  PIANO_SPECULAR = (texture*)dds_load_file("./Engine/Assets/Textures/piano_s.dds");
  
  TANGENT = glGetAttribLocationARB(*PROGRAM, "tangent");
  BINORMAL = glGetAttribLocationARB(*PROGRAM, "binormal");
  COLOR = glGetAttribLocationARB(*PROGRAM, "color");
  
  EYE_POSITION = malloc(sizeof(float) * 3);
  LIGHT_POSITION = malloc(sizeof(float) * 3);
  
  LIGHT_POSITION[0] = 100.0f; LIGHT_POSITION[1] = 100.0f; LIGHT_POSITION[2] = 100.0f;
  
  DIFFUSE_LIGHT = malloc(sizeof(float) * 3);
  SPECULAR_LIGHT = malloc(sizeof(float) * 3);
  AMBIENT_LIGHT = malloc(sizeof(float) * 3);
  
  DIFFUSE_LIGHT[0] = 1.0f; DIFFUSE_LIGHT[1] = 1.0f; DIFFUSE_LIGHT[2] = 1.0f;
  SPECULAR_LIGHT[0] = 1.0f; SPECULAR_LIGHT[1] = 1.0f; SPECULAR_LIGHT[2] = 1.0f;
  AMBIENT_LIGHT[0] = 0.25f; AMBIENT_LIGHT[1] = 0.25f; AMBIENT_LIGHT[2] = 0.25f;
  
}

void forward_renderer_finish() {  
  
  texture_delete(PIANO_DIFFUSE);
  texture_delete(PIANO_NORMAL);
  texture_delete(PIANO_SPECULAR);
  
  glsl_program_delete(PROGRAM);
  
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

  glFlush();
  
  SDL_GL_SwapBuffers();
  
}

void forward_renderer_render_model(render_model* m) {
  
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  
  glEnableVertexAttribArrayARB(TANGENT);
  glEnableVertexAttribArrayARB(BINORMAL);
  glEnableVertexAttribArrayARB(COLOR);
  
    int i;
    for(i=0; i < m->num_meshes; i++) {
      
      render_mesh* me = m->meshes[i];
      
      /* BEGIN HARD CODED MATERIAL SETTINGS */
      
      glUseProgramObjectARB(*PROGRAM);
      
      GLint diffuse_loc = glGetUniformLocation(*PROGRAM, "diffuse_map");
      GLint bump_loc = glGetUniformLocation(*PROGRAM, "bump_map");
      GLint spec_loc = glGetUniformLocation(*PROGRAM, "spec_map");
      
      GLint light_position = glGetUniformLocation(*PROGRAM, "light_position");
      GLint eye_position = glGetUniformLocation(*PROGRAM, "eye_position");
      
      GLint diffuse_light = glGetUniformLocation(*PROGRAM, "diffuse_light");
      GLint ambient_light = glGetUniformLocation(*PROGRAM, "ambient_light");
      GLint specular_light = glGetUniformLocation(*PROGRAM, "specular_light");
      
      GLint glossiness = glGetUniformLocation(*PROGRAM, "glossiness");
      GLint bumpiness = glGetUniformLocation(*PROGRAM, "bumpiness");
      GLint specular_level = glGetUniformLocation(*PROGRAM, "specular_level");
      
      glUniform1iARB(diffuse_loc, 0);
      glUniform1iARB(bump_loc, 1);
      glUniform1iARB(spec_loc, 2);
      
      glActiveTexture(GL_TEXTURE0 + 0);
      glBindTexture(GL_TEXTURE_2D, *PIANO_DIFFUSE);
      glActiveTexture(GL_TEXTURE0 + 1);
      glBindTexture(GL_TEXTURE_2D, *PIANO_NORMAL);
      glActiveTexture(GL_TEXTURE0 + 2);
      glBindTexture(GL_TEXTURE_2D, *PIANO_SPECULAR);
      
      v3_to_array(CAMERA->position, EYE_POSITION);
      
      glUniform3fvARB(light_position, 1, LIGHT_POSITION);
      glUniform3fvARB(eye_position, 1, EYE_POSITION);
      
      glUniform3fvARB(diffuse_light, 1, DIFFUSE_LIGHT);
      glUniform3fvARB(specular_light, 1, SPECULAR_LIGHT);
      glUniform3fvARB(ambient_light, 1, AMBIENT_LIGHT);
      
      glUniform1fARB(glossiness, GLOSSINESS);
      glUniform1fARB(bumpiness, BUMPINESS);
      glUniform1fARB(specular_level, SPECULAR_LEVEL);
      
      /* END HARD CODED MATERIAL SETTINGS */
      
      glVertexPointer(3, GL_FLOAT, 0, me->vertex_positions);
      glNormalPointer(GL_FLOAT, 0, me->vertex_normals);
      glTexCoordPointer(2, GL_FLOAT, 0, me->vertex_uvs);
      
      glVertexAttribPointer(TANGENT, 3, GL_FLOAT, GL_TRUE, 0, me->vertex_tangents);
      glVertexAttribPointer(BINORMAL, 3, GL_FLOAT, GL_TRUE, 0, me->vertex_binormals);
      glVertexAttribPointer(COLOR, 4, GL_FLOAT, GL_TRUE, 0, me->vertex_colors);
      
      glDrawElements(GL_TRIANGLES, me->num_triangles_3, GL_UNSIGNED_INT, me->triangles);
  
      /* DISABLE PROGRAM */
      glUseProgramObjectARB(0);
  
    }
    
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  
  glDisableVertexAttribArrayARB(TANGENT);
  glDisableVertexAttribArrayARB(BINORMAL);
  glDisableVertexAttribArrayARB(COLOR);

}

void forward_renderer_render_quad(texture* quad_texture, vector2 top_left, vector2 bottom_right) {
  
  glUseProgramObjectARB(0);
  
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
  
  glDisable(GL_DEPTH_TEST);
  
  glActiveTexture(GL_TEXTURE0 + 0);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, *quad_texture);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(top_left.x, top_left.y,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(bottom_right.x, top_left.y,  0.0f);
		glTexCoord2f(1.0f, -1.0f); glVertex3f(bottom_right.x,  bottom_right.y,  0.0f);
		glTexCoord2f(0.0f, -1.0f); glVertex3f(top_left.x,  bottom_right.y,  0.0f);
	glEnd();

  glEnable(GL_DEPTH_TEST);
  
  forward_renderer_setup_camera();
}

void forward_renderer_render_screen_quad(texture* quad_texture) {
  forward_renderer_render_quad(quad_texture, v2(-1,-1), v2(1,1) );
}

