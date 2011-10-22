#include "SDL/SDL_local.h"

void check_extension(const char* name, void* function_pointer) {
  if (function_pointer == NULL) {
    printf("Failed to load OpenGL extension %s\n", name);
    exit(EXIT_FAILURE);
  }  
}

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

void SDL_LocalInit() {

  printf("Loading openGL extensions...\n");

  /* Shaders */
  
	glCreateProgram            = (GLCREATEPROGRAMFN)SDL_GL_GetProcAddress( "glCreateProgram" ); check_extension("glCreateProgram", glCreateProgram);
	glLinkProgram              = (GLLINKPROGRAMFN)SDL_GL_GetProcAddress( "glLinkProgram" ); check_extension("glLinkProgram", glLinkProgram);
	glDeleteProgram            = (GLDELETEPROGRAMFN)SDL_GL_GetProcAddress( "glDeleteProgram" ); check_extension("glDeleteProgram", glDeleteProgram);
	glGetProgramInfoLog        = (GLGETPROGRAMINFOLOGFN)SDL_GL_GetProcAddress( "glGetProgramInfoLog" ); check_extension("glGetProgramInfoLog", glGetProgramInfoLog);
	glUseProgram               = (GLUSEPROGRAMFN)SDL_GL_GetProcAddress( "glUseProgram" ); check_extension("glUseProgram", glUseProgram);
	glGetProgramiv             = (GLGETPROGRAMIVFN)SDL_GL_GetProcAddress( "glGetProgramiv" ); check_extension("glGetProgramiv", glGetProgramiv);
  
	glCreateShader             = (GLCREATESHADERFN)SDL_GL_GetProcAddress( "glCreateShader" ); check_extension("glCreateShader", glCreateShader);
	glShaderSource             = (GLSHADERSOURCEFN)SDL_GL_GetProcAddress( "glShaderSource" ); check_extension("glShaderSource", glShaderSource);
	glCompileShader            = (GLCOMPILESHADERFN)SDL_GL_GetProcAddress( "glCompileShader" ); check_extension("glCompileShader", glCompileShader);
	glGetShaderInfoLog         = (GLGETSHADERINFOLOGFN)SDL_GL_GetProcAddress( "glGetShaderInfoLog" ); check_extension("glGetShaderInfoLog", glGetShaderInfoLog);
	glAttachShader             = (GLATTACHSHADERFN)SDL_GL_GetProcAddress( "glAttachShader" ); check_extension("glAttachShader", glAttachShader);
 	glDeleteShader             = (GLDELETESHADERFN)SDL_GL_GetProcAddress( "glDeleteShader" ); check_extension("glDeleteShader", glDeleteShader);
	glGetShaderiv              = (GLGETSHADERIVFN)SDL_GL_GetProcAddress( "glGetShaderiv" ); check_extension("glGetShaderiv", glGetShaderiv);
  
  glGetUniformLocation       = (GLGETUNIFORMLOCATIONFN)SDL_GL_GetProcAddress( "glGetUniformLocation" ); check_extension("glGetUniformLocation", glGetUniformLocation);
	glUniform1f                = (GLUNIFORM1FFN)SDL_GL_GetProcAddress( "glUniform1f" ); check_extension("glUniform1f", glUniform1f);
	glUniform1i                = (GLUNIFORM1IFN)SDL_GL_GetProcAddress( "glUniform1i" ); check_extension("glUniform1i", glUniform1i);
	glUniform2f                = (GLUNIFORM2FFN)SDL_GL_GetProcAddress( "glUniform2f" ); check_extension("glUniform2f", glUniform2f);
	glUniform3f                = (GLUNIFORM3FFN)SDL_GL_GetProcAddress( "glUniform3f" ); check_extension("glUniform3f", glUniform3f);
	glUniform4f                = (GLUNIFORM4FFN)SDL_GL_GetProcAddress( "glUniform4f" ); check_extension("glUniform4f", glUniform4f);
	glUniform1fv               = (GLUNIFORM1FVFN)SDL_GL_GetProcAddress( "glUniform1fv" ); check_extension("glUniform1fv", glUniform1fv);
	glUniform2fv               = (GLUNIFORM2FVFN)SDL_GL_GetProcAddress( "glUniform2fv" ); check_extension("glUniform2fv", glUniform2fv);
	glUniformMatrix4fv         = (GLUNIFORMMATRIX4FVFN)SDL_GL_GetProcAddress( "glUniformMatrix4fv" ); check_extension("glUniformMatrix4fv", glUniformMatrix4fv);
  
  glGetAttribLocation        = (GLGETATTRIBLOCATIONFN)SDL_GL_GetProcAddress( "glGetAttribLocation" ); check_extension("glGetAttribLocation", glGetAttribLocation);
	glVertexAttribPointer      = (GLVERTEXATTRIBPOINTERFN)SDL_GL_GetProcAddress( "glVertexAttribPointer" ); check_extension("glVertexAttribPointer", glVertexAttribPointer);
	glEnableVertexAttribArray  = (GLENABLEVERTEXATTRIBARRAYFN)SDL_GL_GetProcAddress( "glEnableVertexAttribArray" ); check_extension("glEnableVertexAttribArray", glEnableVertexAttribArray);
	glDisableVertexAttribArray = (GLDISABLEVERTEXATTRIBARRAYFN)SDL_GL_GetProcAddress( "glDisableVertexAttribArray" ); check_extension("glDisableVertexAttribArray", glDisableVertexAttribArray);
	glBindAttribLocation       = (GLBINDATTRIBLOCATIONFN)SDL_GL_GetProcAddress( "glBindAttribLocation" ); check_extension("glBindAttribLocation", glBindAttribLocation);
  
  /* Textures */
  
  glActiveTexture            = (GLACTIVETEXTUREFN)SDL_GL_GetProcAddress( "glActiveTexture" ); check_extension("glActiveTexture", glActiveTexture);
  glGenerateMipmap           = (GLGENERATEMIPMAPFN)SDL_GL_GetProcAddress( "glGenerateMipmap" ); check_extension("glGenerateMipmap", glGenerateMipmap);
  glCompressedTexImage2D     = (GLCOMPRESSEDTEXIMAGE2DFN)SDL_GL_GetProcAddress( "glCompressedTexImage2D" ); check_extension("glCompressedTexImage2D", glCompressedTexImage2D);

  /* Buffers */
  
	glGenBuffers               = (GLGENBUFFERSFN)SDL_GL_GetProcAddress( "glGenBuffers" ); check_extension("glGenBuffers", glGenBuffers);
	glBindBuffer               = (GLBINDBUFFERFN)SDL_GL_GetProcAddress( "glBindBuffer" ); check_extension("glBindBuffer", glBindBuffer);
	glBufferData               = (GLBUFFERDATAFN)SDL_GL_GetProcAddress( "glBufferData" ); check_extension("glBufferData", glBufferData);
	glDeleteBuffers            = (GLDELETEBUFFERSFN)SDL_GL_GetProcAddress( "glDeleteBuffers" ); check_extension("glDeleteBuffers", glDeleteBuffers);
  glDrawBuffers              = (GLDRAWBUFFERSFN)SDL_GL_GetProcAddress( "glDrawBuffers" ); check_extension("glDrawBuffers", glDrawBuffers);

  glGenRenderbuffers         = (GLGENRENDERBUFFERSFN)SDL_GL_GetProcAddress( "glGenRenderbuffers" ); check_extension("glGenRenderbuffers", glGenRenderbuffers);
  glBindRenderbuffer         = (GLBINDRENDERBUFFERFN)SDL_GL_GetProcAddress( "glBindRenderbuffer" ); check_extension("glBindRenderbuffer", glBindRenderbuffer);
  glRenderbufferStorage      = (GLRENDERBUFFERSTORAGEFN)SDL_GL_GetProcAddress( "glRenderbufferStorage" ); check_extension("glRenderbufferStorage", glRenderbufferStorage);
	glDeleteRenderbuffers      = (GLDELETERENDERBUFFERSFN)SDL_GL_GetProcAddress( "glDeleteRenderbuffers" ); check_extension("glDeleteRenderbuffers", glDeleteRenderbuffers);
  
	glGenFramebuffers          = (GLGENFRAMEBUFFERSFN)SDL_GL_GetProcAddress( "glGenFramebuffers" ); check_extension("glGenFramebuffers", glGenFramebuffers);
	glBindFramebuffer          = (GLBINDFRAMEBUFFERFN)SDL_GL_GetProcAddress( "glBindFramebuffer" ); check_extension("glBindFramebuffer", glBindFramebuffer);
	glFramebufferTexture2D     = (GLFRAMEBUFFERTEXTURE2DFN)SDL_GL_GetProcAddress( "glFramebufferTexture2D" ); check_extension("glFramebufferTexture2D", glFramebufferTexture2D);
	glDeleteFramebuffers       = (GLDELETEFRAMEBUFFERSFN)SDL_GL_GetProcAddress( "glDeleteFramebuffers" ); check_extension("glDeleteFramebuffers", glDeleteFramebuffers);
	glCheckFramebufferStatus   = (GLCHECKFRAMEBUFFERSTATUSFN)SDL_GL_GetProcAddress( "glCheckFramebufferStatus" ); check_extension("glCheckFramebufferStatus", glCheckFramebufferStatus);
  glFramebufferRenderbuffer  = (GLFRAMEBUFFERRENDERBUFFERFN)SDL_GL_GetProcAddress( "glFramebufferRenderbuffer" ); check_extension("glFramebufferRenderbuffer", glFramebufferRenderbuffer);
  
  printf("Done...\n");
  
}