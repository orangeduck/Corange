#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_local.h"

#include "shader.h"
#include "camera.h"
#include "texture.h"
#include "renderable.h"
#include "material.h"
#include "asset_manager.h"
#include "shadow_mapper.h"

#include "viewport.h"

#include "deferred_renderer.h"

static camera* CAMERA = NULL;

static float PROJ_MATRIX[16];
static float VIEW_MATRIX[16];
static float WORLD_MATRIX[16];

static float LIGHT_VIEW_MATRIX[16];
static float LIGHT_PROJ_MATRIX[16];

static shader_program* PROGRAM;
static shader_program* SCREEN_PROGRAM;

static int NORMAL;
static int TANGENT;
static int BINORMAL;

static GLuint fbo = 0;
static GLuint depth_buffer;
static GLuint diffuse_buffer;
static GLuint positions_buffer;
static GLuint normals_buffer;

static GLuint diffuse_texture;
static GLuint positions_texture;
static GLuint normals_texture;
static GLuint depth_texture;

static texture* SHADOW_TEX;
static light* LIGHT;

void deferred_renderer_init() {
  
  PROGRAM = asset_get("./engine/shaders/deferred.prog");
  SCREEN_PROGRAM = asset_get("./engine/shaders/deferred_screen.prog");
  
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
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, viewport_width(), viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, diffuse_buffer);   
  
  glBindRenderbuffer(GL_RENDERBUFFER, positions_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F, viewport_width(), viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, positions_buffer);  
  
  glBindRenderbuffer(GL_RENDERBUFFER, normals_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F, viewport_width(), viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, normals_buffer);  
  
  glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, viewport_width(), viewport_height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);  
  
  glGenTextures(1, &diffuse_texture);
  glBindTexture(GL_TEXTURE_2D, diffuse_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport_width(), viewport_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, diffuse_texture, 0);
  
  glGenTextures(1, &positions_texture);
  glBindTexture(GL_TEXTURE_2D, positions_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, viewport_width(), viewport_height(), 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, positions_texture, 0);
  
  glGenTextures(1, &normals_texture);
  glBindTexture(GL_TEXTURE_2D, normals_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, viewport_width(), viewport_height(), 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, normals_texture, 0);
  
  glGenTextures(1, &depth_texture);
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, viewport_width(), viewport_height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
 
}

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
  
}

void deferred_renderer_set_camera(camera* cam) {
  CAMERA = cam;
}

void deferred_renderer_set_shadow_texture(texture* t) {
  SHADOW_TEX = t;
}

void deferred_renderer_set_light(light* l) {
  LIGHT = l;
}

static void deferred_renderer_use_material(material* mat) {
  
  /* Set material parameters */
  
  int tex_counter = 0;
  
  int i;
  for(i = 0; i < mat->keys->num_items; i++) {
    char* key = list_get(mat->keys, i);
    
    int* type = dictionary_get(mat->types, key);
    void* property = dictionary_get(mat->properties, key);
    
    GLint loc = glGetUniformLocation(*PROGRAM, key);
    
    GLint world_matrix_u = glGetUniformLocation(*PROGRAM, "world_matrix");
    glUniformMatrix4fv(world_matrix_u, 1, 0, WORLD_MATRIX);
    
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

static void deferred_renderer_setup_camera() {

  matrix_4x4 viewm = camera_view_matrix(CAMERA);
  matrix_4x4 projm = camera_proj_matrix(CAMERA, viewport_ratio() );
  
  m44_to_array(viewm, VIEW_MATRIX);
  m44_to_array(projm, PROJ_MATRIX);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(VIEW_MATRIX);
  
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(PROJ_MATRIX);
  
  matrix_4x4 lviewm = light_view_matrix(LIGHT);
  matrix_4x4 lprojm = light_proj_matrix(LIGHT);
  
  m44_to_array(lviewm, LIGHT_VIEW_MATRIX);
  m44_to_array(lprojm, LIGHT_PROJ_MATRIX);

}

void deferred_renderer_begin() {

  deferred_renderer_setup_camera();

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
  glDrawBuffers(3, buffers);
  
  glDisable(GL_LIGHTING);
  
  glClearColor(1.0f, 0.769f, 0.0f, 0.0f);
  glClearDepth(1.0f);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  
}

void deferred_renderer_end() {
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  glUseProgram(*SCREEN_PROGRAM);
  
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
  glUniform1i(glGetUniformLocation(*SCREEN_PROGRAM, "diffuse_texture"), 0);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glBindTexture(GL_TEXTURE_2D, positions_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*SCREEN_PROGRAM, "positions_texture"), 1);
  
  glActiveTexture(GL_TEXTURE0 + 2 );
  glBindTexture(GL_TEXTURE_2D, normals_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*SCREEN_PROGRAM, "normals_texture"), 2);
  
  glActiveTexture(GL_TEXTURE0 + 3 );
  glBindTexture(GL_TEXTURE_2D, depth_texture);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*SCREEN_PROGRAM, "depth_texture"), 3);
  
  glActiveTexture(GL_TEXTURE0 + 4 );
  glBindTexture(GL_TEXTURE_2D, *SHADOW_TEX);
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*SCREEN_PROGRAM, "shadows_texture"), 4);
  
  glActiveTexture(GL_TEXTURE0 + 5 );
  glBindTexture(GL_TEXTURE_2D, *(texture*)asset_get("./engine/resources/random.dds"));
  glEnable(GL_TEXTURE_2D);
  glUniform1i(glGetUniformLocation(*SCREEN_PROGRAM, "random_texture"), 5);
  
  glActiveTexture(GL_TEXTURE0 + 6 );
  glBindTexture(GL_TEXTURE_3D, *(texture*)asset_get("./engine/resources/bluey.lut"));
  glEnable(GL_TEXTURE_3D);
  glUniform1i(glGetUniformLocation(*SCREEN_PROGRAM, "lut"), 6);
  
  GLint cam_position = glGetUniformLocation(*SCREEN_PROGRAM, "camera_position");
  glUniform3f(cam_position, CAMERA->position.x, CAMERA->position.y, CAMERA->position.z);
  
  GLint light_position = glGetUniformLocation(*SCREEN_PROGRAM, "light_position");
  glUniform3f(light_position, LIGHT->position.x, LIGHT->position.y, LIGHT->position.z);
  
  GLint lproj_matrix_u = glGetUniformLocation(*SCREEN_PROGRAM, "light_proj");
  glUniformMatrix4fv(lproj_matrix_u, 1, 0, LIGHT_PROJ_MATRIX);
  
  GLint lview_matrix_u = glGetUniformLocation(*SCREEN_PROGRAM, "light_view");
  glUniformMatrix4fv(lview_matrix_u, 1, 0, LIGHT_VIEW_MATRIX);
  
  GLint diffuse_light = glGetUniformLocation(*SCREEN_PROGRAM, "diffuse_light");
  GLint ambient_light = glGetUniformLocation(*SCREEN_PROGRAM, "ambient_light");
  GLint specular_light = glGetUniformLocation(*SCREEN_PROGRAM, "specular_light");
  
  glUniform3f(diffuse_light, LIGHT->diffuse_color.r, LIGHT->diffuse_color.g, LIGHT->diffuse_color.b);
  glUniform3f(specular_light, LIGHT->specular_color.r, LIGHT->specular_color.g, LIGHT->specular_color.b);
  glUniform3f(ambient_light, LIGHT->ambient_color.r, LIGHT->ambient_color.g, LIGHT->ambient_color.b);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0,  1.0,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0,  1.0,  0.0f);
	glEnd();
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 1 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 2 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 3 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 4 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 5 );
  glDisable(GL_TEXTURE_2D);
  
  glActiveTexture(GL_TEXTURE0 + 6 );
  glDisable(GL_TEXTURE_3D);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
  glUseProgram(0);
  
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
            
      glUseProgram(*PROGRAM);
      
      deferred_renderer_use_material(mat);
      
      glVertexPointer(3, GL_FLOAT, 0, me->vertex_positions);
      glTexCoordPointer(2, GL_FLOAT, 0, me->vertex_uvs);
      
      glVertexAttribPointer(NORMAL, 3, GL_FLOAT, GL_TRUE, 0, me->vertex_normals);
      glVertexAttribPointer(TANGENT, 3, GL_FLOAT, GL_TRUE, 0, me->vertex_tangents);
      glVertexAttribPointer(BINORMAL, 3, GL_FLOAT, GL_TRUE, 0, me->vertex_binormals);
      
      glDrawElements(GL_TRIANGLES, me->num_triangles_3, GL_UNSIGNED_INT, me->triangles);
  
      glUseProgram(0);
  
    }
    
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  
  glDisableVertexAttribArray(NORMAL);
  glDisableVertexAttribArray(TANGENT);
  glDisableVertexAttribArray(BINORMAL);

};

void deferred_renderer_render_static(static_object* s) {

  matrix_4x4 r_world_matrix = m44_world( s->position, s->scale, s->rotation );
  m44_to_array(r_world_matrix, WORLD_MATRIX);
  
  renderable* r = s->renderable;
  
  int i;
  for(i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    if(s->is_rigged) {
      
      glUseProgram(*PROGRAM);
      
      deferred_renderer_use_material(s->base);
      
      GLsizei stride = sizeof(float) * 24;
      
      glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
          
      glVertexPointer(3, GL_FLOAT, stride, (void*)0);
      glEnableClientState(GL_VERTEX_ARRAY);
      
      glVertexAttribPointer(NORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
      glEnableVertexAttribArray(NORMAL);
      
      glVertexAttribPointer(TANGENT, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
      glEnableVertexAttribArray(TANGENT);
      
      glVertexAttribPointer(BINORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 9));
      glEnableVertexAttribArray(BINORMAL);
      
      glTexCoordPointer(2, GL_FLOAT, stride, (void*)(sizeof(float) * 12));
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
      
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);  
      
      glDisableVertexAttribArray(NORMAL);
      glDisableVertexAttribArray(TANGENT);
      glDisableVertexAttribArray(BINORMAL);
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      
      glUseProgram(0);
      
    } else {
    
      glUseProgram(*PROGRAM);
      
      deferred_renderer_use_material(s->base);
      
      GLsizei stride = sizeof(float) * 18;
      
      glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
          
      glVertexPointer(3, GL_FLOAT, stride, (void*)0);
      glEnableClientState(GL_VERTEX_ARRAY);
      
      glVertexAttribPointer(NORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
      glEnableVertexAttribArray(NORMAL);
      
      glVertexAttribPointer(TANGENT, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
      glEnableVertexAttribArray(TANGENT);
      
      glVertexAttribPointer(BINORMAL, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 9));
      glEnableVertexAttribArray(BINORMAL);
      
      glTexCoordPointer(2, GL_FLOAT, stride, (void*)(sizeof(float) * 12));
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
      
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);  
      
      glDisableVertexAttribArray(NORMAL);
      glDisableVertexAttribArray(TANGENT);
      glDisableVertexAttribArray(BINORMAL);
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      
      glUseProgram(0);
    
    }

  }

}


