
#define GLEW_STATIC
#include "GL/glew.h"

#define NO_SDL_GLEXT
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "shader.h"
#include "camera.h"
#include "texture.h"
#include "material.h"

#include "deferred_renderer.h"

static camera* CAMERA = NULL;

static float proj_matrix[16];
static float view_matrix[16];

static shader_program* PROGRAM;
static shader_program* SCREEN_PROGRAM;

static int WIDTH;
static int HEIGHT;

static int NORMAL;
static int TANGENT;
static int BINORMAL;

static texture* PIANO_DIFFUSE;
static texture* PIANO_NORMAL;
static texture* PIANO_SPECULAR;

static float CAM_POSITION[3];

static GLuint fbo = 0;
static GLuint depth_buffer;
static GLuint diffuse_buffer;
static GLuint positions_buffer;
static GLuint normals_buffer;

static GLuint diffuse_texture;
static GLuint positions_texture;
static GLuint normals_texture;
static GLuint depth_texture;

static float ASPECT_RATIO(){
  return (float)HEIGHT / (float)WIDTH;
}

void deferred_renderer_init(int width, int height) {

  WIDTH = width;
  HEIGHT = height;

  shader* deferred_vs = vs_load_file("./Engine/Assets/Shaders/deferred.vs");
  shader* deferred_fs = fs_load_file("./Engine/Assets/Shaders/deferred.fs");
  PROGRAM = shader_program_new();
  shader_program_attach_shader(PROGRAM, deferred_vs);
  shader_program_attach_shader(PROGRAM, deferred_fs);
  shader_program_link(PROGRAM);
  
  shader_delete(deferred_vs);
  shader_delete(deferred_fs);
  
  shader* deferred_screen_vs = vs_load_file("./Engine/Assets/Shaders/deferred_screen.vs");
  shader* deferred_screen_fs = fs_load_file("./Engine/Assets/Shaders/deferred_screen.fs");
  SCREEN_PROGRAM = shader_program_new();
  shader_program_attach_shader(SCREEN_PROGRAM, deferred_screen_vs);
  shader_program_attach_shader(SCREEN_PROGRAM, deferred_screen_fs);
  shader_program_link(SCREEN_PROGRAM);
  
  shader_delete(deferred_screen_vs);
  shader_delete(deferred_screen_fs);
  
  PIANO_DIFFUSE = (texture*)dds_load_file("./Engine/Assets/Textures/piano.dds");
  PIANO_NORMAL = (texture*)dds_load_file("./Engine/Assets/Textures/piano_nm.dds");
  PIANO_SPECULAR = (texture*)dds_load_file("./Engine/Assets/Textures/piano_s.dds");  
  
  NORMAL = glGetAttribLocation(*PROGRAM, "vNormal");
  TANGENT = glGetAttribLocation(*PROGRAM, "vTangent");
  BINORMAL = glGetAttribLocation(*PROGRAM, "vBiNormal");  
  
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  
  glGenRenderbuffers(1, &depth_buffer);
  glGenRenderbuffers(1, &diffuse_buffer);
  glGenRenderbuffers(1, &positions_buffer);
  glGenRenderbuffers(1, &normals_buffer);  
  
  glBindRenderbuffer(GL_RENDERBUFFER, diffuse_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, WIDTH, HEIGHT);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, diffuse_buffer);   
  
  glBindRenderbuffer(GL_RENDERBUFFER, positions_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F, WIDTH, HEIGHT);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, positions_buffer);  
  
  glBindRenderbuffer(GL_RENDERBUFFER, normals_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F, WIDTH, HEIGHT);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, normals_buffer);  
  
  glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, WIDTH, HEIGHT);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);  
  
  glGenTextures(1, &diffuse_texture);
  glBindTexture(GL_TEXTURE_2D, diffuse_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, diffuse_texture, 0);
  
  glGenTextures(1, &positions_texture);
  glBindTexture(GL_TEXTURE_2D, positions_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, positions_texture, 0);
  
  glGenTextures(1, &normals_texture);
  glBindTexture(GL_TEXTURE_2D, normals_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, normals_texture, 0);
  
  glGenTextures(1, &depth_texture);
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, WIDTH, HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
 
};

void deferred_renderer_finish() {
  
  glDeleteFramebuffers(1, &fbo);
  
  glDeleteRenderbuffers(1, &diffuse_buffer);
  glDeleteRenderbuffers(1, &positions_buffer);
  glDeleteRenderbuffers(1, &normals_buffer);
  glDeleteRenderbuffers(1, &depth_buffer);
  
  glDeleteTextures(1,&diffuse_texture);
  glDeleteTextures(1,&positions_texture);
  glDeleteTextures(1,&normals_texture);
  glDeleteTextures(1,&depth_texture);
  
  texture_delete(PIANO_DIFFUSE);
  texture_delete(PIANO_NORMAL);
  texture_delete(PIANO_SPECULAR);  
  
  shader_program_delete(PROGRAM);
  shader_program_delete(SCREEN_PROGRAM);
  
};

void deferred_renderer_set_camera(camera* cam) {
  CAMERA = cam;
};

void deferred_renderer_set_viewport(int width, int height) {
  WIDTH = width;
  HEIGHT = height;
};

void deferred_renderer_setup_camera() {

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

};

void deferred_renderer_begin() {

  deferred_renderer_setup_camera();

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glPushAttrib(GL_VIEWPORT_BIT);
  glViewport(0,0,WIDTH, HEIGHT);  
  
  glClearColor(1.0f, 0.769f, 0.0f, 0.0f);
  glClearDepth(1.0f);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
  glDrawBuffers(3, buffers);
  
  glDisable(GL_LIGHTING);
  
};

void deferred_renderer_end() {
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  glUseProgramObjectARB(*SCREEN_PROGRAM);
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glDepthMask(GL_FALSE);
  glDisable(GL_DEPTH_TEST);
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glBindTexture(GL_TEXTURE_2D, diffuse_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1iARB(glGetUniformLocation(*SCREEN_PROGRAM, "diffuse_texture"), 0);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glBindTexture(GL_TEXTURE_2D, positions_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1iARB(glGetUniformLocation(*SCREEN_PROGRAM, "positions_texture"), 1);
  
  glActiveTexture(GL_TEXTURE0 + 2 );
  glBindTexture(GL_TEXTURE_2D, normals_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1iARB(glGetUniformLocation(*SCREEN_PROGRAM, "normals_texture"), 2);
  
  glActiveTexture(GL_TEXTURE0 + 3 );
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1iARB(glGetUniformLocation(*SCREEN_PROGRAM, "depth_texture"), 3);
  
  GLint cam_position = glGetUniformLocation(*SCREEN_PROGRAM, "cameraPosition");
  v3_to_array(CAMERA->position, CAM_POSITION);
  glUniform3fv(cam_position, 1, CAM_POSITION);
  
	glBegin(GL_QUADS);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f); glVertex3f(-1.0, -1.0,  0.0f);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f); glVertex3f(1.0, -1.0,  0.0f);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 1.0f); glVertex3f(1.0,  1.0,  0.0f);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f); glVertex3f(-1.0,  1.0,  0.0f);
	glEnd();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 2 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 3 );
  glDisable(GL_TEXTURE_2D);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
  glUseProgramObjectARB(0);
  
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);

};

void deferred_renderer_render_model(render_model* m, material* mat) {

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  
  glEnableVertexAttribArray(NORMAL);
  glEnableVertexAttribArray(TANGENT);
  glEnableVertexAttribArray(BINORMAL);
  
    int i;
    for(i=0; i < m->num_meshes; i++) {
      
      render_mesh* me = m->meshes[i];
            
      glUseProgramObjectARB(*PROGRAM);
      
      GLint diffuse_loc = glGetUniformLocation(*PROGRAM, "tDiffuse");
      GLint bump_loc = glGetUniformLocation(*PROGRAM, "tBumpMap");
      GLint spec_loc = glGetUniformLocation(*PROGRAM, "tSpecMap");
      
      glUniform1i(diffuse_loc, 0);
      glUniform1i(bump_loc, 1);
      glUniform1i(spec_loc, 2);
      
      glActiveTexture(GL_TEXTURE0 + 0);
      glBindTexture(GL_TEXTURE_2D, *PIANO_DIFFUSE);
      glActiveTexture(GL_TEXTURE0 + 1);
      glBindTexture(GL_TEXTURE_2D, *PIANO_NORMAL);
      glActiveTexture(GL_TEXTURE0 + 2);
      glBindTexture(GL_TEXTURE_2D, *PIANO_SPECULAR);
      
      GLint specular_level = glGetUniformLocation(*PROGRAM, "specular_level");
      glUniform1f(specular_level, 2.0);
      
      GLint glossiness = glGetUniformLocation(*PROGRAM, "glossiness");
      glUniform1f(glossiness, 20.0);
      
      GLint bumpiness = glGetUniformLocation(*PROGRAM, "bumpiness");
      glUniform1f(bumpiness, 2.0);
      
      glVertexPointer(3, GL_FLOAT, 0, me->vertex_positions);
      glTexCoordPointer(2, GL_FLOAT, 0, me->vertex_uvs);
      
      glVertexAttribPointer(NORMAL, 3, GL_FLOAT, GL_TRUE, 0, me->vertex_normals);
      glVertexAttribPointer(TANGENT, 3, GL_FLOAT, GL_TRUE, 0, me->vertex_tangents);
      glVertexAttribPointer(BINORMAL, 3, GL_FLOAT, GL_TRUE, 0, me->vertex_binormals);
      
      glDrawElements(GL_TRIANGLES, me->num_triangles_3, GL_UNSIGNED_INT, me->triangles);
  
      glUseProgramObjectARB(0);
  
    }
    
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  
  glDisableVertexAttribArray(NORMAL);
  glDisableVertexAttribArray(TANGENT);
  glDisableVertexAttribArray(BINORMAL);

};


