
#include "glsl.h"

#include "deferred_renderer.h"

static glsl_program* PROGRAM;

void deferred_renderer_init() {

  /*

  PROGRAM = (glsl_program*)glsl_load_shaders("./Engine/Assets/Shaders/deferred.vs","./Engine/Assets/Shaders/deferred.fs");
  
  
  
  glGenFramebuffersEXT(1, &fbo);
  glGenRenderbuffersEXT(1, &depthBuffer);
  glGenFramebuffersEXT(1, &diffuseBuffer);
  glGenRenderbuffersEXT(1, &positionsBuffer);
  glGenRenderbuffersEXT(1, &normalsBuffer);

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
  
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, diffuseBuffer);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGB, dWidth,
                          dHeight);
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                          GL_COLOR_ATTACHMENT0_EXT,
                          GL_RENDERBUFFER_EXT, diffuseBuffer);

  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, positionsBuffer);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGB32F_ARB, dWidth,
                          dHeight);
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                          GL_COLOR_ATTACHMENT1_EXT,
                          GL_RENDERBUFFER_EXT, positionsBuffer);

  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, normalsBuffer);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGB16F_ARB, dWidth,
                          dHeight);
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                          GL_COLOR_ATTACHMENT2_EXT,
                          GL_RENDERBUFFER_EXT, normalsBuffer);

  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthBuffer);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24,
                          dWidth, dHeight);
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                              GL_DEPTH_ATTACHMENT_EXT,
                              GL_RENDERBUFFER_EXT, depthBuffer);
  */
};

void deferred_renderer_finish() {

};

void deferred_renderer_set_camera(camera* cam) {


};

void deferred_renderer_set_viewport(int width, int height) {


};

void deferred_renderer_setup_camera() {


};

void deferred_renderer_begin() {

};

void deferred_renderer_end() {


};

void deferred_renderer_render_model(model* m) {


};


