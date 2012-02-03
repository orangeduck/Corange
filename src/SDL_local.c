#include <string.h>

#include "error.h"

#include "SDL/SDL_rwops.h"
#include "SDL/SDL_local.h"

#ifdef __linux__
  #include <execinfo.h>
#endif
  
#ifdef _WIN32
  #include <windows.h>
  #include <winbase.h>
#endif

#ifndef __linux__
  GLACTIVETEXTUREFN glActiveTexture;
  GLCOMPRESSEDTEXIMAGE2DFN glCompressedTexImage2D;
  GLTEXIMAGE3DFN glTexImage3D;
#endif
GLCREATESHADERFN glCreateShader;
GLCREATEPROGRAMFN glCreateProgram;
GLSHADERSOURCEFN glShaderSource;
GLCOMPILESHADERFN glCompileShader;
GLGETSHADERINFOLOGFN glGetShaderInfoLog;
GLATTACHSHADERFN glAttachShader;
GLLINKPROGRAMFN glLinkProgram;
GLGETPROGRAMINFOLOGFN glGetProgramInfoLog;
GLGETUNIFORMLOCATIONFN glGetUniformLocation;
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

#ifdef _WIN32

void SDL_PathFullName(char* dst, char* path) {
  GetFullPathName(path, MAX_PATH, dst, NULL);
}

#elif __linux__

#include <stdlib.h>

void SDL_PathFullName(char* dst, char* path) {
  realpath(path, dst);
}

#endif

void SDL_PathFileName(char* dst, char* path) {
  
  int i = strlen(path);
  int ext_loc = 0;
  while( i > 0) {
    if (path[i] == '/') { break; }
    if (path[i] == '\\') { break; }
    if (path[i] == '.') { ext_loc = i; }
    i--;
  }
  
  char* file = path + i + 1;
  int len = ext_loc - i - 1;
  
  strncpy(dst, file, len);
  dst[len] = '\0';
}

void SDL_PathFileExtension(char* dst, char* path) {

  int ext_len = 0;
  int i = strlen(path);
  while( i >= 0) {
    if (path[i] != '.') { ext_len++; }
    if (path[i] == '.') { break; }
    i--;
  }
  
  int prev = strlen(path) - ext_len + 1;
  char* f_ext = path + prev;
  strcpy(dst, f_ext);
}

void SDL_PathFileLocation(char* dst, char* path) {
  
  int i = strlen(path);
  while( i > 0) {
    if (path[i] == '/') { break; }
    if (path[i] == '\\') { break; }
    i--;
  }
  i++;
  
  strncpy(dst, path, i);
  dst[i] = '\0';
}


#ifdef _WIN32
  #include <direct.h>
  #define getcwd _getcwd
  #define chdir _chdir
#else
  #include <unistd.h>
#endif

 
static char curr_dir[FILENAME_MAX];
char* SDL_GetWorkingDir() {

  if (!getcwd(curr_dir, sizeof(curr_dir))) {
    error("Could not get working directory!");
  }
  
  return curr_dir;
}

void SDL_SetWorkingDir(char* dir) {
  
  if (chdir(dir)) {
    error("Could not change working directory!");
  }
  
}

void SDL_RWsize(SDL_RWops* file, int* size) {
  int pos = SDL_RWtell(file);
  *size = SDL_RWseek(file, 0, SEEK_END);
  SDL_RWseek(file, pos, SEEK_SET);
}

int SDL_RWreadline(SDL_RWops* file, char* buffer, int buffersize) {
  
  char c = ' ';
  int i = 0;
  while( SDL_RWread(file, &c, 1, 1) ) {
    
    if (i >= buffersize) {
      error("Buffer not large enough to read line!");
    }
    
    buffer[i] = c;
    i++;
    
    if (c == '\n') {
      buffer[i] = '\0';
      return i;
    }
  }
  
  if(i > 0) {
    /* End of file but actually data on the final line */
    buffer[i] = '\0';
    return i;
  } else {
    /* Actual end of file */
    return 0;
  }
  
}

void SDL_PrintOpenGLInfo() {
  debug("OpenGL Info");
  const char* vendor = (const char*)glGetString(GL_VENDOR);
  debug("Vendor: %s", vendor);
  const char* renderer = (const char*)glGetString(GL_RENDERER);
  debug("Renderer: %s", renderer);
  const char* version = (const char*)glGetString(GL_VERSION);
  debug("Version: %s", version);
  const char* shader_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
  debug("Shader Version: %s", shader_version);
}

void SDL_PrintOpenGLExtensions() {
  const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
  debug("OpenGL Extensions: %s\n", extensions);
}

bool SDL_OpenGLSupportsShaderLinkage() {
  
  const char* renderer = (const char*)glGetString(GL_RENDERER);
  if (strstr("ATI Mobility Radeon HD 5650", renderer)) {
    return false;
  }
  
  return true;
}

bool SDL_OpenGLSupportsVsync() {

  const char* renderer = (const char*)glGetString(GL_RENDERER);
  if (strstr("ATI Mobility Radeon HD 5650", renderer)) {
    return false;
  }
  
  return true;
}


void SDL_CheckOpenGLError(const char* name) {
  switch (glGetError()) {
    case GL_INVALID_ENUM:
      error("OpenGL Error on function %s: Invalid Enum", name);
    case GL_INVALID_VALUE:
      error("OpenGL Error on function %s: Invalid Value", name);
    case GL_INVALID_OPERATION:
      error("OpenGL Error on function %s: Invalid Operation", name);
    case GL_OUT_OF_MEMORY:
      error("OpenGL Error on function %s: Out of Memory", name);
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      error("OpenGL Error on function %s: Invalid FrameBuffer Operation", name);
    case GL_STACK_OVERFLOW:
      error("OpenGL Error on function %s: Stack Overflow", name);
    case GL_STACK_UNDERFLOW:
      error("OpenGL Error on function %s: Stack Underflow", name);
    case GL_TABLE_TOO_LARGE:
      error("OpenGL Error on function %s: Table Too Large", name);
  }
}

void SDL_LoadOpenGLExtensions() {

  debug("Loading OpenGL Extensions...");

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
  
  glGenerateMipmap           = (GLGENERATEMIPMAPFN)SDL_GL_GetProcAddress( "glGenerateMipmap" ); SDL_CheckOpenGLExtension("glGenerateMipmap", glGenerateMipmap);
  #ifndef __linux__
  glActiveTexture            = (GLACTIVETEXTUREFN)SDL_GL_GetProcAddress( "glActiveTexture" ); SDL_CheckOpenGLExtension("glActiveTexture", glActiveTexture);
  glCompressedTexImage2D     = (GLCOMPRESSEDTEXIMAGE2DFN)SDL_GL_GetProcAddress( "glCompressedTexImage2D" ); SDL_CheckOpenGLExtension("glCompressedTexImage2D", glCompressedTexImage2D);
  glTexImage3D               = (GLTEXIMAGE3DFN)SDL_GL_GetProcAddress( "glTexImage3D" ); SDL_CheckOpenGLExtension("glTexImage3D", glTexImage3D);
  #endif

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
  
}

void SDL_CheckOpenGLExtension(const char* name, void* function_pointer) {
  if (function_pointer == NULL) {
    error("Failed to load OpenGL extension %s", name);
  }  
}

#ifdef __linux__

void SDL_PrintStackTrace() {
  
  const int DEPTH = 10;
  void* stack[DEPTH];
     
  int size = backtrace(stack, DEPTH);
  char** strings = backtrace_symbols(stack, size);

  printf ("[STACK] (%i frames)\n", size);
  
  for (int i = 0; i < size; i++) {
    printf ("  %s\n", strings[i]);
  }

  free(strings);
}

#elif _WIN32
#include <windows.h>

typedef struct _SYMBOL_INFO {
  ULONG   SizeOfStruct;
  ULONG   TypeIndex;
  ULONG64 Reserved[2];
  ULONG   Index;
  ULONG   Size;
  ULONG64 ModBase;
  ULONG   Flags;
  ULONG64 Value;
  ULONG64 Address;
  ULONG   Register;
  ULONG   Scope;
  ULONG   Tag;
  ULONG   NameLen;
  ULONG   MaxNameLen;
  TCHAR   Name[1];
} SYMBOL_INFO, *PSYMBOL_INFO;

#define PCTSTR const char*

void SDL_PrintStackTrace() {
  
  /* For some reason this is giving the incorrect symbol names */
  
  typedef USHORT (WINAPI *CaptureStackBackTraceType)(ULONG,ULONG,PVOID*,PULONG);
  typedef BOOL (WINAPI *SymInitializeType)(HANDLE,PCTSTR,BOOL);
  typedef BOOL (WINAPI *SymFromAddrType)(HANDLE,DWORD64,PDWORD64,PSYMBOL_INFO);
  
  CaptureStackBackTraceType CaptureStackBackTrace = (CaptureStackBackTraceType)(GetProcAddress(LoadLibrary("kernel32.dll"), "RtlCaptureStackBackTrace"));
  SymInitializeType SymInitialize = (SymInitializeType)(GetProcAddress(LoadLibrary("Dbghelp.dll"), "SymInitialize"));
  SymFromAddrType SymFromAddr = (SymFromAddrType)(GetProcAddress(LoadLibrary("Dbghelp.dll"), "SymFromAddr"));
  
  if ((CaptureStackBackTrace == NULL) || 
      (SymInitialize == NULL) ||
      (SymFromAddr == NULL)) {
    printf("[STACK] Could not retrieve functions for stack trace\n");
    return;
  }
  
  void* stack[62];
  
  HANDLE process = GetCurrentProcess();
  if (!SymInitialize(process, NULL, TRUE)) {
    printf("[STACK] Could not retrieve functions for stack trace\n");
  }
   
  int max_frames = CaptureStackBackTrace( 0, 62, stack, NULL );
  SYMBOL_INFO* symbol = calloc(sizeof(SYMBOL_INFO) + 256, 1);
  symbol->MaxNameLen   = 255;
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
  
  DWORD64 displacement = 0;
  
  if (sizeof(void*) != sizeof(DWORD64)) {
    printf("[STACK] Cannot retrive stack symbols on 32-bit binary\n");
    return;
  }
  for(int i = 0; i < max_frames; i++ ){
    
    DWORD64 address = 0;
    address = PtrToUlong(stack[i]);
    if (SymFromAddr(process, address, &displacement, symbol)) {
      printf("  %i: %s - %08X\n", max_frames-i-1, symbol->Name, (unsigned int)symbol->Address );
    } else {
      DWORD error = GetLastError();
      printf("  %i: SymFromAddr returned error %d\n", max_frames-i-1, (int)error);
    }
    
  }

  free(symbol);
  
}

#else

void SDL_PrintStackTrace() {}

#endif

#ifdef _WIN32

#include <Commdlg.h>

static char returned_filename[FILENAME_MAX];
char* SDL_OpenFileDialog(char* format_string, int default_format) {
  
  /* This dialogue has a habit of changing the working directory and messing stuff up. */
  char* working_dir = SDL_GetWorkingDir();
  
  if (format_string == NULL) {
    format_string = "All Files\0*.*\0\0";
  }
  
  returned_filename[0] = '\0';
  
  OPENFILENAME opf;
  opf.hwndOwner = 0;
  
  opf.lpstrFilter = format_string;
  opf.lpstrCustomFilter = NULL;
  opf.nMaxCustFilter = 0;
  
  /* For some reason microsoft count from 1 here */
  opf.nFilterIndex = default_format + 1;
  
  opf.lpstrFile = returned_filename;
  opf.nMaxFile = FILENAME_MAX;
  
  opf.lpstrFileTitle = NULL;
  opf.nMaxFileTitle = 0;
  
  opf.lpstrInitialDir = working_dir;
  opf.lpstrTitle = NULL;
  opf.nFileOffset = 0;
  opf.nFileExtension = 0;
  opf.lpstrDefExt = NULL;
  opf.lpfnHook = NULL;
  opf.lCustData = 0;
  opf.Flags = OFN_PATHMUSTEXIST;
  opf.lStructSize = sizeof(OPENFILENAME);
  GetOpenFileName(&opf);
  
  SDL_SetWorkingDir(working_dir);
  
  return returned_filename;
}

#else 

char* SDL_OpenFileDialog(char* format_string, int default_format) {
  error("Cannot open file dialog on platform that is not windows.");
}

#endif
