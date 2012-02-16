#ifndef SDL_LOCAL_H
#define SDL_LOCAL_H

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "bool.h"

#ifndef MAX_PATH
  #include <limits.h>
  
  #ifdef PATH_MAX
    #define MAX_PATH PATH_MAX
  #else
    #define MAX_PATH 256
  #endif
#endif

#ifndef GLchar
  #define GLchar char
#endif
#ifndef GLsizeiptr
  #define GLsizeiptr int
#endif

void SDL_PathFullName(char* dst, char* path);
void SDL_PathFileName(char* dst, char* path);
void SDL_PathFileExtension(char* dst, char* path);
void SDL_PathFileLocation(char* dst, char* path);

char* SDL_GetWorkingDir();
void SDL_SetWorkingDir(char* dir);

void SDL_RWsize(SDL_RWops* file, int* size);
int SDL_RWreadline(SDL_RWops* file, char* buffer, int buffersize);

void SDL_PrintOpenGLInfo();
void SDL_PrintOpenGLExtensions();

bool SDL_OpenGLSupportsShaderLinkage();
bool SDL_OpenGLSupportsVsync();

void SDL_LoadOpenGLExtensions();
void SDL_CheckOpenGLExtension(const char* name, void* function_pointer);
void SDL_CheckOpenGLError(const char* name);

void SDL_PrintStackTrace();

typedef GLuint (APIENTRY * GLCREATESHADERFN)( GLenum type );
typedef GLuint (APIENTRY * GLCREATEPROGRAMFN)( void );
typedef void (APIENTRY * GLSHADERSOURCEFN)( GLuint shader, GLsizei count, const GLchar** string, const GLint* length );
typedef void (APIENTRY * GLCOMPILESHADERFN)( GLuint shader );
typedef void (APIENTRY * GLGETSHADERINFOLOGFN)( GLuint shader, GLsizei bufsize,GLsizei* length, GLchar* infolog );
typedef void (APIENTRY * GLATTACHSHADERFN)( GLuint program, GLuint shader );
typedef void (APIENTRY * GLLINKPROGRAMFN)( GLuint program );
typedef void (APIENTRY * GLGETPROGRAMINFOLOGFN)( GLuint program, GLsizei bufsize,GLsizei* length, GLchar* infolog );
typedef GLint (APIENTRY * GLGETUNIFORMLOCATIONFN)( GLuint program, const GLchar* name );
typedef void (APIENTRY * GLACTIVETEXTUREFN)( GLenum texture );
typedef void (APIENTRY * GLUNIFORM1FFN)( GLint location, GLfloat x );
typedef void (APIENTRY * GLUNIFORM1IFN)( GLint location, GLint x );
typedef void (APIENTRY * GLDELETESHADERFN)( GLuint shader );
typedef void (APIENTRY * GLDELETEPROGRAMFN)( GLuint program );
typedef void (APIENTRY * GLUSEPROGRAMFN)( GLuint program );
typedef void (APIENTRY * GLVERTEXATTRIBPOINTERFN)( GLuint indx, GLint size, GLenum type,GLboolean normalized, GLsizei stride, const GLvoid* ptr );
typedef void (APIENTRY * GLENABLEVERTEXATTRIBARRAYFN)( GLuint index );
typedef void (APIENTRY * GLDISABLEVERTEXATTRIBARRAYFN)( GLuint index );
typedef void (APIENTRY * GLUNIFORM2FFN)( GLint location, GLfloat x, GLfloat y);
typedef void (APIENTRY * GLUNIFORM3FFN)( GLint location, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * GLUNIFORM4FFN)( GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w );
typedef void (APIENTRY * GLUNIFORMMATRIX4FVFN)( GLint location, GLsizei count,GLboolean transpose, const GLfloat* value );
typedef void (APIENTRY * GLUNIFORM1FVFN)( GLint location, GLsizei count, const GLfloat* value );
typedef void (APIENTRY * GLUNIFORM2FVFN)( GLint location, GLsizei count, const GLfloat* value );
typedef void (APIENTRY * GLUNIFORM3FVFN)( GLint location, GLsizei count, const GLfloat* value );
typedef void (APIENTRY * GLGETSHADERIVFN)( GLuint shader, GLenum pname, GLint* params );
typedef void (APIENTRY * GLGETPROGRAMIVFN)( GLuint program, GLenum pname, GLint* params );
typedef void (APIENTRY * GLBINDATTRIBLOCATIONFN)( GLuint program, GLuint index, const GLchar* name );
typedef void (APIENTRY * GLGENFRAMEBUFFERSFN)( GLsizei n, GLuint* ids );
typedef void (APIENTRY * GLBINDFRAMEBUFFERFN)( GLenum target, GLuint framebuffer );
typedef void (APIENTRY * GLFRAMEBUFFERTEXTURE2DFN)( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level );
typedef void (APIENTRY * GLDELETEFRAMEBUFFERSFN)( GLsizei n, GLuint* framebuffers );
typedef GLenum (APIENTRY * GLCHECKFRAMEBUFFERSTATUSFN)( GLenum target );
typedef void (APIENTRY * GLGENBUFFERSFN)( GLsizei n, GLuint* buffers );
typedef void (APIENTRY * GLGENRENDERBUFFERSFN)( GLsizei n, GLuint* buffers );
typedef void (APIENTRY * GLDELETEBUFFERSFN)( GLsizei n, const GLuint* buffers );
typedef void (APIENTRY * GLDELETERENDERBUFFERSFN)( GLsizei n, const GLuint* buffers );
typedef void (APIENTRY * GLBINDBUFFERFN)( GLenum target, GLuint buffer );
typedef void (APIENTRY * GLBINDRENDERBUFFERFN)( GLenum target, GLuint buffer );
typedef void (APIENTRY * GLBUFFERDATAFN)( GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage );
typedef void (APIENTRY * GLFRAMEBUFFERRENDERBUFFERFN)( GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer );
typedef GLint (APIENTRY * GLGETATTRIBLOCATIONFN)( GLuint program, const GLchar* name );
typedef void (APIENTRY * GLRENDERBUFFERSTORAGEFN)( GLenum target, GLenum format, GLsizei width, GLsizei height);
typedef void (APIENTRY * GLDRAWBUFFERSFN)(GLsizei n, const GLenum* buffers);
typedef void (APIENTRY * GLGENERATEMIPMAPFN)(GLenum target);
typedef void (APIENTRY * GLCOMPRESSEDTEXIMAGE2DFN)(GLenum target, GLint level, GLenum format, GLsizei width, GLsizei height, GLint border, GLsizei imagesize, const GLvoid* data);
typedef void (APIENTRY * GLTEXIMAGE3DFN)(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* data);

#ifndef __linux__
  extern GLACTIVETEXTUREFN glActiveTexture;
  extern GLCOMPRESSEDTEXIMAGE2DFN glCompressedTexImage2D;
  extern GLTEXIMAGE3DFN glTexImage3D;
#endif
extern GLCREATESHADERFN glCreateShader;
extern GLCREATEPROGRAMFN glCreateProgram;
extern GLSHADERSOURCEFN glShaderSource;
extern GLCOMPILESHADERFN glCompileShader;
extern GLGETSHADERINFOLOGFN glGetShaderInfoLog;
extern GLATTACHSHADERFN glAttachShader;
extern GLLINKPROGRAMFN glLinkProgram;
extern GLGETPROGRAMINFOLOGFN glGetProgramInfoLog;
extern GLGETUNIFORMLOCATIONFN glGetUniformLocation;
extern GLUNIFORM1FFN glUniform1f;
extern GLUNIFORM1IFN glUniform1i;
extern GLDELETESHADERFN glDeleteShader;
extern GLDELETEPROGRAMFN glDeleteProgram;
extern GLUSEPROGRAMFN glUseProgram;
extern GLVERTEXATTRIBPOINTERFN glVertexAttribPointer;
extern GLENABLEVERTEXATTRIBARRAYFN glEnableVertexAttribArray;
extern GLDISABLEVERTEXATTRIBARRAYFN glDisableVertexAttribArray;
extern GLUNIFORM2FFN glUniform2f;
extern GLUNIFORM3FFN glUniform3f;
extern GLUNIFORM4FFN glUniform4f;
extern GLUNIFORMMATRIX4FVFN glUniformMatrix4fv;
extern GLUNIFORM1FVFN glUniform1fv;
extern GLUNIFORM2FVFN glUniform2fv;
extern GLUNIFORM2FVFN glUniform3fv;
extern GLGETSHADERIVFN glGetShaderiv;
extern GLGETPROGRAMIVFN glGetProgramiv;
extern GLBINDATTRIBLOCATIONFN glBindAttribLocation;
extern GLGENFRAMEBUFFERSFN glGenFramebuffers;
extern GLBINDFRAMEBUFFERFN glBindFramebuffer;
extern GLFRAMEBUFFERTEXTURE2DFN glFramebufferTexture2D;
extern GLDELETEFRAMEBUFFERSFN glDeleteFramebuffers;
extern GLCHECKFRAMEBUFFERSTATUSFN glCheckFramebufferStatus;
extern GLGENBUFFERSFN glGenBuffers;
extern GLGENRENDERBUFFERSFN glGenRenderbuffers;
extern GLDELETEBUFFERSFN glDeleteBuffers;
extern GLDELETERENDERBUFFERSFN glDeleteRenderbuffers;
extern GLBINDBUFFERFN glBindBuffer;
extern GLBINDRENDERBUFFERFN glBindRenderbuffer;
extern GLBUFFERDATAFN glBufferData;
extern GLFRAMEBUFFERRENDERBUFFERFN glFramebufferRenderbuffer;
extern GLGETATTRIBLOCATIONFN glGetAttribLocation;
extern GLRENDERBUFFERSTORAGEFN glRenderbufferStorage;
extern GLDRAWBUFFERSFN glDrawBuffers;
extern GLGENERATEMIPMAPFN glGenerateMipmap;

/* Extension Constants - Found these from glew and Google */

#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_FRAMEBUFFER 0x8D40
#define GL_RENDERBUFFER 0x8D41
#define GL_MAX_COLOR_ATTACHMENTS 0x8CDF
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_COLOR_ATTACHMENT1 0x8CE1
#define GL_COLOR_ATTACHMENT2 0x8CE2
#define GL_COLOR_ATTACHMENT3 0x8CE3
#define GL_COLOR_ATTACHMENT4 0x8CE4
#define GL_COLOR_ATTACHMENT5 0x8CE5
#define GL_COLOR_ATTACHMENT6 0x8CE6
#define GL_COLOR_ATTACHMENT7 0x8CE7
#define GL_COLOR_ATTACHMENT8 0x8CE8
#define GL_COLOR_ATTACHMENT9 0x8CE9
#define GL_COLOR_ATTACHMENT10 0x8CEA
#define GL_COLOR_ATTACHMENT11 0x8CEB
#define GL_COLOR_ATTACHMENT12 0x8CEC
#define GL_COLOR_ATTACHMENT13 0x8CED
#define GL_COLOR_ATTACHMENT14 0x8CEE
#define GL_COLOR_ATTACHMENT15 0x8CEF
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_STENCIL_ATTACHMENT 0x8D20
#define GL_RGBA32F 0x8814
#define GL_RGBA16F 0x881A
#define GL_BGRA 0x80E1
#define GL_BGR 0x80E0
#define GL_COMPRESSED_RGBA_S3TC_DXT1 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5 0x83F3
#define GL_UNSIGNED_SHORT_1_5_5_5_REV 0x8366
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_TEXTURE0 0x84C0
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_MULTISAMPLE 0x809D
#define GL_STATIC_DRAW 0x88E4
#define GL_GENERATE_MIPMAP 0x8191
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_TEXTURE_MAX_LEVEL 0x813D
#define GL_DYNAMIC_COPY 0x88EA
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_TEXTURE_3D 0x806F
#define GL_TABLE_TOO_LARGE 0x8031
#define GL_INVALID_FRAMEBUFFER_OPERATION 0x0506
#define GL_TEXTURE_WRAP_R 0x8072
#define GL_MIRRORED_REPEAT 0x8370
#define GL_TEXTURE_DEPTH 0x8071
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF


#endif
