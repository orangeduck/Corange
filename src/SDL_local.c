#include "error.h"

#include "SDL/SDL_rwops.h"
#include "SDL/SDL_local.h"

GLCREATESHADERFN glCreateShader;
GLCREATEPROGRAMFN glCreateProgram;
GLSHADERSOURCEFN glShaderSource;
GLCOMPILESHADERFN glCompileShader;
GLGETSHADERINFOLOGFN glGetShaderInfoLog;
GLATTACHSHADERFN glAttachShader;
GLLINKPROGRAMFN glLinkProgram;
GLGETPROGRAMINFOLOGFN glGetProgramInfoLog;
GLGETUNIFORMLOCATIONFN glGetUniformLocation;
GLACTIVETEXTUREFN glActiveTexture;
GLUNIFORM1FFN glUniform1f;
GLUNIFORM1IFN glUniform1i;
GLDELETESHADERFN glDeleteShader;
GLDELETEPROGRAMFN glDeleteProgram;
GLUSEPROGRAMFN glUseProgram;
GLVERTEXATTRIBPOINTERFN glVertexAttribPointer;
GLENABLEVERTEXATTRIBARRAYFN glEnableVertexAttribArray;
GLDISABLEVERTEXATTRIBARRAYFN glDisableVertexAttribArray;
GLUNIFORM2FFN glUniform2f;
GLUNIFORM3FFN glUniform3f;
GLUNIFORM4FFN glUniform4f;
GLUNIFORMMATRIX4FVFN glUniformMatrix4fv;
GLUNIFORM1FVFN glUniform1fv;
GLUNIFORM2FVFN glUniform2fv;
GLGETSHADERIVFN glGetShaderiv;
GLGETPROGRAMIVFN glGetProgramiv;
GLBINDATTRIBLOCATIONFN glBindAttribLocation;
GLGENFRAMEBUFFERSFN glGenFramebuffers;
GLBINDFRAMEBUFFERFN glBindFramebuffer;
GLFRAMEBUFFERTEXTURE2DFN glFramebufferTexture2D;
GLDELETEFRAMEBUFFERSFN glDeleteFramebuffers;
GLCHECKFRAMEBUFFERSTATUSFN glCheckFramebufferStatus;
GLGENBUFFERSFN glGenBuffers;
GLGENRENDERBUFFERSFN glGenRenderbuffers;
GLDELETEBUFFERSFN glDeleteBuffers;
GLDELETERENDERBUFFERSFN glDeleteRenderbuffers;
GLBINDBUFFERFN glBindBuffer;
GLBINDRENDERBUFFERFN glBindRenderbuffer;
GLBUFFERDATAFN glBufferData;
GLFRAMEBUFFERRENDERBUFFERFN glFramebufferRenderbuffer;
GLGETATTRIBLOCATIONFN glGetAttribLocation;
GLRENDERBUFFERSTORAGEFN glRenderbufferStorage;
GLDRAWBUFFERSFN glDrawBuffers;
GLGENERATEMIPMAPFN glGenerateMipmap;
GLCOMPRESSEDTEXIMAGE2DFN glCompressedTexImage2D;
GLTEXIMAGE3DFN glTexImage3D;

void SDL_CheckOpenGLError(const char* name) {
  switch (glGetError()) {
    
    case GL_INVALID_ENUM:
      error("OpenGL Error on function %s: Invalid Enum", name);
    break;
    
    case GL_INVALID_VALUE:
      error("OpenGL Error on function %s: Invalid Value", name);
    break;
    
    case GL_INVALID_OPERATION:
      error("OpenGL Error on function %s: Invalid Operation", name);
      exit(EXIT_FAILURE);
    break;
  
    case GL_OUT_OF_MEMORY:
      error("OpenGL Error on function %s: Out of Memory", name);
    break;
  
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      error("OpenGL Error on function %s: Invalid FrameBuffer Operation", name);
    break;
  
    case GL_STACK_OVERFLOW:
      error("OpenGL Error on function %s: Stack Overflow", name);
    break;
    
    case GL_STACK_UNDERFLOW:
      error("OpenGL Error on function %s: Stack Underflow", name);
    break;
    
    case GL_TABLE_TOO_LARGE:
      error("OpenGL Error on function %s: Table Too Large", name);
    break;

  }
}

void SDL_LoadOpenGLExtensions() {

  printf("Loading openGL extensions...\n");

  /* Shaders */
  
	glCreateProgram            = (GLCREATEPROGRAMFN)SDL_GL_GetProcAddress( "glCreateProgram" ); SDL_CheckOpenGLExtension("glCreateProgram", glCreateProgram);
	glLinkProgram              = (GLLINKPROGRAMFN)SDL_GL_GetProcAddress( "glLinkProgram" ); SDL_CheckOpenGLExtension("glLinkProgram", glLinkProgram);
	glDeleteProgram            = (GLDELETEPROGRAMFN)SDL_GL_GetProcAddress( "glDeleteProgram" ); SDL_CheckOpenGLExtension("glDeleteProgram", glDeleteProgram);
	glGetProgramInfoLog        = (GLGETPROGRAMINFOLOGFN)SDL_GL_GetProcAddress( "glGetProgramInfoLog" ); SDL_CheckOpenGLExtension("glGetProgramInfoLog", glGetProgramInfoLog);
	glUseProgram               = (GLUSEPROGRAMFN)SDL_GL_GetProcAddress( "glUseProgram" ); SDL_CheckOpenGLExtension("glUseProgram", glUseProgram);
	glGetProgramiv             = (GLGETPROGRAMIVFN)SDL_GL_GetProcAddress( "glGetProgramiv" ); SDL_CheckOpenGLExtension("glGetProgramiv", glGetProgramiv);
  
	glCreateShader             = (GLCREATESHADERFN)SDL_GL_GetProcAddress( "glCreateShader" ); SDL_CheckOpenGLExtension("glCreateShader", glCreateShader);
	glShaderSource             = (GLSHADERSOURCEFN)SDL_GL_GetProcAddress( "glShaderSource" ); SDL_CheckOpenGLExtension("glShaderSource", glShaderSource);
	glCompileShader            = (GLCOMPILESHADERFN)SDL_GL_GetProcAddress( "glCompileShader" ); SDL_CheckOpenGLExtension("glCompileShader", glCompileShader);
	glGetShaderInfoLog         = (GLGETSHADERINFOLOGFN)SDL_GL_GetProcAddress( "glGetShaderInfoLog" ); SDL_CheckOpenGLExtension("glGetShaderInfoLog", glGetShaderInfoLog);
	glAttachShader             = (GLATTACHSHADERFN)SDL_GL_GetProcAddress( "glAttachShader" ); SDL_CheckOpenGLExtension("glAttachShader", glAttachShader);
 	glDeleteShader             = (GLDELETESHADERFN)SDL_GL_GetProcAddress( "glDeleteShader" ); SDL_CheckOpenGLExtension("glDeleteShader", glDeleteShader);
	glGetShaderiv              = (GLGETSHADERIVFN)SDL_GL_GetProcAddress( "glGetShaderiv" ); SDL_CheckOpenGLExtension("glGetShaderiv", glGetShaderiv);
  
  glGetUniformLocation       = (GLGETUNIFORMLOCATIONFN)SDL_GL_GetProcAddress( "glGetUniformLocation" ); SDL_CheckOpenGLExtension("glGetUniformLocation", glGetUniformLocation);
	glUniform1f                = (GLUNIFORM1FFN)SDL_GL_GetProcAddress( "glUniform1f" ); SDL_CheckOpenGLExtension("glUniform1f", glUniform1f);
	glUniform1i                = (GLUNIFORM1IFN)SDL_GL_GetProcAddress( "glUniform1i" ); SDL_CheckOpenGLExtension("glUniform1i", glUniform1i);
	glUniform2f                = (GLUNIFORM2FFN)SDL_GL_GetProcAddress( "glUniform2f" ); SDL_CheckOpenGLExtension("glUniform2f", glUniform2f);
	glUniform3f                = (GLUNIFORM3FFN)SDL_GL_GetProcAddress( "glUniform3f" ); SDL_CheckOpenGLExtension("glUniform3f", glUniform3f);
	glUniform4f                = (GLUNIFORM4FFN)SDL_GL_GetProcAddress( "glUniform4f" ); SDL_CheckOpenGLExtension("glUniform4f", glUniform4f);
	glUniform1fv               = (GLUNIFORM1FVFN)SDL_GL_GetProcAddress( "glUniform1fv" ); SDL_CheckOpenGLExtension("glUniform1fv", glUniform1fv);
	glUniform2fv               = (GLUNIFORM2FVFN)SDL_GL_GetProcAddress( "glUniform2fv" ); SDL_CheckOpenGLExtension("glUniform2fv", glUniform2fv);
	glUniformMatrix4fv         = (GLUNIFORMMATRIX4FVFN)SDL_GL_GetProcAddress( "glUniformMatrix4fv" ); SDL_CheckOpenGLExtension("glUniformMatrix4fv", glUniformMatrix4fv);
  
  glGetAttribLocation        = (GLGETATTRIBLOCATIONFN)SDL_GL_GetProcAddress( "glGetAttribLocation" ); SDL_CheckOpenGLExtension("glGetAttribLocation", glGetAttribLocation);
	glVertexAttribPointer      = (GLVERTEXATTRIBPOINTERFN)SDL_GL_GetProcAddress( "glVertexAttribPointer" ); SDL_CheckOpenGLExtension("glVertexAttribPointer", glVertexAttribPointer);
	glEnableVertexAttribArray  = (GLENABLEVERTEXATTRIBARRAYFN)SDL_GL_GetProcAddress( "glEnableVertexAttribArray" ); SDL_CheckOpenGLExtension("glEnableVertexAttribArray", glEnableVertexAttribArray);
	glDisableVertexAttribArray = (GLDISABLEVERTEXATTRIBARRAYFN)SDL_GL_GetProcAddress( "glDisableVertexAttribArray" ); SDL_CheckOpenGLExtension("glDisableVertexAttribArray", glDisableVertexAttribArray);
	glBindAttribLocation       = (GLBINDATTRIBLOCATIONFN)SDL_GL_GetProcAddress( "glBindAttribLocation" ); SDL_CheckOpenGLExtension("glBindAttribLocation", glBindAttribLocation);
  
  /* Textures */
  
  glActiveTexture            = (GLACTIVETEXTUREFN)SDL_GL_GetProcAddress( "glActiveTexture" ); SDL_CheckOpenGLExtension("glActiveTexture", glActiveTexture);
  glGenerateMipmap           = (GLGENERATEMIPMAPFN)SDL_GL_GetProcAddress( "glGenerateMipmap" ); SDL_CheckOpenGLExtension("glGenerateMipmap", glGenerateMipmap);
  glCompressedTexImage2D     = (GLCOMPRESSEDTEXIMAGE2DFN)SDL_GL_GetProcAddress( "glCompressedTexImage2D" ); SDL_CheckOpenGLExtension("glCompressedTexImage2D", glCompressedTexImage2D);
  glTexImage3D               = (GLTEXIMAGE3DFN)SDL_GL_GetProcAddress( "glTexImage3D" ); SDL_CheckOpenGLExtension("glTexImage3D", glTexImage3D);

  /* Buffers */
  
	glGenBuffers               = (GLGENBUFFERSFN)SDL_GL_GetProcAddress( "glGenBuffers" ); SDL_CheckOpenGLExtension("glGenBuffers", glGenBuffers);
	glBindBuffer               = (GLBINDBUFFERFN)SDL_GL_GetProcAddress( "glBindBuffer" ); SDL_CheckOpenGLExtension("glBindBuffer", glBindBuffer);
	glBufferData               = (GLBUFFERDATAFN)SDL_GL_GetProcAddress( "glBufferData" ); SDL_CheckOpenGLExtension("glBufferData", glBufferData);
	glDeleteBuffers            = (GLDELETEBUFFERSFN)SDL_GL_GetProcAddress( "glDeleteBuffers" ); SDL_CheckOpenGLExtension("glDeleteBuffers", glDeleteBuffers);
  glDrawBuffers              = (GLDRAWBUFFERSFN)SDL_GL_GetProcAddress( "glDrawBuffers" ); SDL_CheckOpenGLExtension("glDrawBuffers", glDrawBuffers);

  glGenRenderbuffers         = (GLGENRENDERBUFFERSFN)SDL_GL_GetProcAddress( "glGenRenderbuffers" ); SDL_CheckOpenGLExtension("glGenRenderbuffers", glGenRenderbuffers);
  glBindRenderbuffer         = (GLBINDRENDERBUFFERFN)SDL_GL_GetProcAddress( "glBindRenderbuffer" ); SDL_CheckOpenGLExtension("glBindRenderbuffer", glBindRenderbuffer);
  glRenderbufferStorage      = (GLRENDERBUFFERSTORAGEFN)SDL_GL_GetProcAddress( "glRenderbufferStorage" ); SDL_CheckOpenGLExtension("glRenderbufferStorage", glRenderbufferStorage);
	glDeleteRenderbuffers      = (GLDELETERENDERBUFFERSFN)SDL_GL_GetProcAddress( "glDeleteRenderbuffers" ); SDL_CheckOpenGLExtension("glDeleteRenderbuffers", glDeleteRenderbuffers);
  
	glGenFramebuffers          = (GLGENFRAMEBUFFERSFN)SDL_GL_GetProcAddress( "glGenFramebuffers" ); SDL_CheckOpenGLExtension("glGenFramebuffers", glGenFramebuffers);
	glBindFramebuffer          = (GLBINDFRAMEBUFFERFN)SDL_GL_GetProcAddress( "glBindFramebuffer" ); SDL_CheckOpenGLExtension("glBindFramebuffer", glBindFramebuffer);
	glFramebufferTexture2D     = (GLFRAMEBUFFERTEXTURE2DFN)SDL_GL_GetProcAddress( "glFramebufferTexture2D" ); SDL_CheckOpenGLExtension("glFramebufferTexture2D", glFramebufferTexture2D);
	glDeleteFramebuffers       = (GLDELETEFRAMEBUFFERSFN)SDL_GL_GetProcAddress( "glDeleteFramebuffers" ); SDL_CheckOpenGLExtension("glDeleteFramebuffers", glDeleteFramebuffers);
	glCheckFramebufferStatus   = (GLCHECKFRAMEBUFFERSTATUSFN)SDL_GL_GetProcAddress( "glCheckFramebufferStatus" ); SDL_CheckOpenGLExtension("glCheckFramebufferStatus", glCheckFramebufferStatus);
  glFramebufferRenderbuffer  = (GLFRAMEBUFFERRENDERBUFFERFN)SDL_GL_GetProcAddress( "glFramebufferRenderbuffer" ); SDL_CheckOpenGLExtension("glFramebufferRenderbuffer", glFramebufferRenderbuffer);
  
  printf("Done...\n");
  
}

void SDL_CheckOpenGLExtension(const char* name, void* function_pointer) {
  if (function_pointer == NULL) {
    error("Failed to load OpenGL extension %s", name);
  }  
}