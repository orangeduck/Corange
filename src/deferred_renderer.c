
#define GLEW_STATIC
#include "GL/glew.h"

#define NO_SDL_GLEXT
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "shader.h"
#include "camera.h"
#include "texture.h"
#include "material.h"
#include "asset_manager.h"

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
  
  PROGRAM = asset_get("./engine/shaders/deferred.prog");
  SCREEN_PROGRAM = asset_get("./engine/shaders/deferred_screen.prog");
    
  PIANO_DIFFUSE = asset_get("./engine/textures/piano.dds");
  PIANO_NORMAL = asset_get("./engine/Assets/Textures/piano_nm.dds");
  PIANO_SPECULAR = asset_get("./Engine/Assets/Textures/piano_s.dds");  
  
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
      
      deferred_renderer_use_material(mat);
      
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

void deferred_renderer_use_material(material* mat) {
  
  /* Set material parameters */
  
  int tex_counter = 0;
  
  int i;
  for(i = 0; i < mat->keys->num_items; i++) {
    char* key = list_get(mat->keys, i);
    
    int* type = dictionary_get(mat->types, key);
    void* property = dictionary_get(mat->properties, key);
    
    GLint loc = glGetUniformLocation(*PROGRAM, key);
    
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


