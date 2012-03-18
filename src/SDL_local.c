#include <string.h>

#include "error.h"

#include "SDL/SDL_rwops.h"
#include "SDL/SDL_local.h"

#ifdef _WIN32
  #include "SDL/SDL_syswm.h"
#endif

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
GLUNIFORM3FVFN glUniform3fv;
GLGETSHADERIVFN glGetShaderiv;
GLGETPROGRAMIVFN glGetProgramiv;
GLBINDATTRIBLOCATIONFN glBindAttribLocation;
GLGENFRAMEBUFFERSFN glGenFramebuffers;
GLBINDFRAMEBUFFERFN glBindFramebuffer;
GLBLITFRAMEBUFFERFN glBlitFramebuffer;
GLFRAMEBUFFERTEXTUREFN glFramebufferTexture;
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
GLGETBUFFERSUBDATAFN glGetBufferSubData;
GLFRAMEBUFFERRENDERBUFFERFN glFramebufferRenderbuffer;
GLGETATTRIBLOCATIONFN glGetAttribLocation;
GLRENDERBUFFERSTORAGEFN glRenderbufferStorage;
GLRENDERBUFFERSTORAGEMULTISAMPLEFN glRenderbufferStorageMultisample;
GLDRAWBUFFERSFN glDrawBuffers;
GLGENERATEMIPMAPFN glGenerateMipmap;
GLDRAWELEMENTSINSTANCEDFN glDrawElementsInstanced;

#ifdef _WIN32

void SDL_PathFullName(char* dst, char* path) {
  GetFullPathName(path, MAX_PATH, dst, NULL);
}

#elif __linux__

#include <stdlib.h>

void SDL_PathFullName(char* dst, char* path) {
  char* ret = realpath(path, dst);
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
  
  char c[10];
  int i = 0;
  int stat = 0;
  while(1) {
    
    stat = SDL_RWread(file, &c[0], 1, 1);
    
    if (stat == -1) error("Error reading file.");
    if (i == buffersize-1) error("Buffer not large enough to read line!");
    if (stat == 0) break;
    
    buffer[i] = c[0];
    i++;
    
    if (c[0] == '\n') {
      buffer[i] = '\0';
      return i;
    }
  }
  
  if(i > 0) {
    buffer[i] = '\0';
    return i;
  } else {
    return 0;
  }
  
}

#ifdef _WIN32

static HICON icon;

void SDL_WM_UseResourceIcon() {

  HINSTANCE handle = GetModuleHandle(NULL);
  icon = LoadIcon(handle, "icon");
  
  SDL_SysWMinfo wminfo;
  SDL_VERSION(&wminfo.version)
  if (SDL_GetWMInfo(&wminfo) != 1) {
    error("Incorrect SDL version!");
  }

  SetClassLong(wminfo.window, GCL_HICON, (LONG)icon);
  
}

void SDL_WM_DeleteResourceIcon() {
  DestroyIcon(icon);
}

#else

void SDL_WM_UseResourceIcon() {

}

void SDL_WM_DeleteResourceIcon() {

}

#endif

void SDL_GL_PrintInfo() {
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

void SDL_GL_PrintExtensions() {
  const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
  debug("OpenGL Extensions: %s\n", extensions);
}

bool SDL_GL_SupportsShaderLinkage() {
  
  const char* renderer = (const char*)glGetString(GL_RENDERER);
  if (strstr("ATI Mobility Radeon HD 5650", renderer)) {
    return false;
  }
  
  return true;
}

bool SDL_GL_SupportsVsync() {

  const char* renderer = (const char*)glGetString(GL_RENDERER);
  if (strstr("ATI Mobility Radeon HD 5650", renderer)) {
    return false;
  }
  
  return true;
}


void SDL_GL_CheckError(const char* name) {
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

#ifdef _WIN32

static HDC gl_thread_device = NULL;
static HGLRC gl_thread_context = NULL;
static void* gl_thread_data = NULL;
static int (*gl_thread_func)(void*) = NULL; 

static int gl_thread_create(void* unused) {
  
  BOOL err = wglMakeCurrent(gl_thread_device, gl_thread_context);
  if (err == 0) {
    error("Could not make context current");
  }
  
  int status = gl_thread_func(gl_thread_data);
  
  HGLRC context = wglGetCurrentContext();
  if (context == NULL) {
    error("Could not get current context");
  }
  
  err = wglDeleteContext(context);
  if (err == 0) {
    error("Could not delete context");
  }
  
  return status;
}

SDL_Thread* SDL_GL_CreateThread(int (*fn)(void *), void *data) {

  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);
  if (SDL_GetWMInfo(&info) == -1) {
    error("Could not get SDL version info.");
  }
  
  gl_thread_device = GetDC(info.window);

  gl_thread_context = wglCreateContext(gl_thread_device);
  if (gl_thread_context == NULL) {
    error("Could not create new OpenGL context");
  }
  
  BOOL err = wglShareLists(info.hglrc, gl_thread_context);
  if (err == 0) {
    int code = GetLastError();
    error("Could not get OpenGL share lists: %i", code);
  }
  
  gl_thread_func = fn;
  gl_thread_data = data;
  
  return SDL_CreateThread(gl_thread_create, NULL);

}

#endif

void SDL_GL_LoadExtensions() {

  debug("Loading OpenGL Extensions...");

  /* Shaders */
  
	glCreateProgram            = (GLCREATEPROGRAMFN)SDL_GL_GetProcAddress( "glCreateProgram" ); SDL_GL_CheckExtension("glCreateProgram", glCreateProgram);
	glLinkProgram              = (GLLINKPROGRAMFN)SDL_GL_GetProcAddress( "glLinkProgram" ); SDL_GL_CheckExtension("glLinkProgram", glLinkProgram);
	glDeleteProgram            = (GLDELETEPROGRAMFN)SDL_GL_GetProcAddress( "glDeleteProgram" ); SDL_GL_CheckExtension("glDeleteProgram", glDeleteProgram);
	glGetProgramInfoLog        = (GLGETPROGRAMINFOLOGFN)SDL_GL_GetProcAddress( "glGetProgramInfoLog" ); SDL_GL_CheckExtension("glGetProgramInfoLog", glGetProgramInfoLog);
	glUseProgram               = (GLUSEPROGRAMFN)SDL_GL_GetProcAddress( "glUseProgram" ); SDL_GL_CheckExtension("glUseProgram", glUseProgram);
	glGetProgramiv             = (GLGETPROGRAMIVFN)SDL_GL_GetProcAddress( "glGetProgramiv" ); SDL_GL_CheckExtension("glGetProgramiv", glGetProgramiv);
  
	glCreateShader             = (GLCREATESHADERFN)SDL_GL_GetProcAddress( "glCreateShader" ); SDL_GL_CheckExtension("glCreateShader", glCreateShader);
	glShaderSource             = (GLSHADERSOURCEFN)SDL_GL_GetProcAddress( "glShaderSource" ); SDL_GL_CheckExtension("glShaderSource", glShaderSource);
	glCompileShader            = (GLCOMPILESHADERFN)SDL_GL_GetProcAddress( "glCompileShader" ); SDL_GL_CheckExtension("glCompileShader", glCompileShader);
	glGetShaderInfoLog         = (GLGETSHADERINFOLOGFN)SDL_GL_GetProcAddress( "glGetShaderInfoLog" ); SDL_GL_CheckExtension("glGetShaderInfoLog", glGetShaderInfoLog);
	glAttachShader             = (GLATTACHSHADERFN)SDL_GL_GetProcAddress( "glAttachShader" ); SDL_GL_CheckExtension("glAttachShader", glAttachShader);
 	glDeleteShader             = (GLDELETESHADERFN)SDL_GL_GetProcAddress( "glDeleteShader" ); SDL_GL_CheckExtension("glDeleteShader", glDeleteShader);
	glGetShaderiv              = (GLGETSHADERIVFN)SDL_GL_GetProcAddress( "glGetShaderiv" ); SDL_GL_CheckExtension("glGetShaderiv", glGetShaderiv);
  
  glGetUniformLocation       = (GLGETUNIFORMLOCATIONFN)SDL_GL_GetProcAddress( "glGetUniformLocation" ); SDL_GL_CheckExtension("glGetUniformLocation", glGetUniformLocation);
	glUniform1f                = (GLUNIFORM1FFN)SDL_GL_GetProcAddress( "glUniform1f" ); SDL_GL_CheckExtension("glUniform1f", glUniform1f);
	glUniform1i                = (GLUNIFORM1IFN)SDL_GL_GetProcAddress( "glUniform1i" ); SDL_GL_CheckExtension("glUniform1i", glUniform1i);
	glUniform2f                = (GLUNIFORM2FFN)SDL_GL_GetProcAddress( "glUniform2f" ); SDL_GL_CheckExtension("glUniform2f", glUniform2f);
	glUniform3f                = (GLUNIFORM3FFN)SDL_GL_GetProcAddress( "glUniform3f" ); SDL_GL_CheckExtension("glUniform3f", glUniform3f);
	glUniform4f                = (GLUNIFORM4FFN)SDL_GL_GetProcAddress( "glUniform4f" ); SDL_GL_CheckExtension("glUniform4f", glUniform4f);
	glUniform1fv               = (GLUNIFORM1FVFN)SDL_GL_GetProcAddress( "glUniform1fv" ); SDL_GL_CheckExtension("glUniform1fv", glUniform1fv);
	glUniform2fv               = (GLUNIFORM2FVFN)SDL_GL_GetProcAddress( "glUniform2fv" ); SDL_GL_CheckExtension("glUniform2fv", glUniform2fv);
	glUniform3fv               = (GLUNIFORM3FVFN)SDL_GL_GetProcAddress( "glUniform3fv" ); SDL_GL_CheckExtension("glUniform3fv", glUniform3fv);
	glUniformMatrix4fv         = (GLUNIFORMMATRIX4FVFN)SDL_GL_GetProcAddress( "glUniformMatrix4fv" ); SDL_GL_CheckExtension("glUniformMatrix4fv", glUniformMatrix4fv);
  
  glGetAttribLocation        = (GLGETATTRIBLOCATIONFN)SDL_GL_GetProcAddress( "glGetAttribLocation" ); SDL_GL_CheckExtension("glGetAttribLocation", glGetAttribLocation);
	glVertexAttribPointer      = (GLVERTEXATTRIBPOINTERFN)SDL_GL_GetProcAddress( "glVertexAttribPointer" ); SDL_GL_CheckExtension("glVertexAttribPointer", glVertexAttribPointer);
	glEnableVertexAttribArray  = (GLENABLEVERTEXATTRIBARRAYFN)SDL_GL_GetProcAddress( "glEnableVertexAttribArray" ); SDL_GL_CheckExtension("glEnableVertexAttribArray", glEnableVertexAttribArray);
	glDisableVertexAttribArray = (GLDISABLEVERTEXATTRIBARRAYFN)SDL_GL_GetProcAddress( "glDisableVertexAttribArray" ); SDL_GL_CheckExtension("glDisableVertexAttribArray", glDisableVertexAttribArray);
	glBindAttribLocation       = (GLBINDATTRIBLOCATIONFN)SDL_GL_GetProcAddress( "glBindAttribLocation" ); SDL_GL_CheckExtension("glBindAttribLocation", glBindAttribLocation);
  
  /* Textures */
  
  glGenerateMipmap           = (GLGENERATEMIPMAPFN)SDL_GL_GetProcAddress( "glGenerateMipmap" ); SDL_GL_CheckExtension("glGenerateMipmap", glGenerateMipmap);
  #ifndef __linux__
  glActiveTexture            = (GLACTIVETEXTUREFN)SDL_GL_GetProcAddress( "glActiveTexture" ); SDL_GL_CheckExtension("glActiveTexture", glActiveTexture);
  glCompressedTexImage2D     = (GLCOMPRESSEDTEXIMAGE2DFN)SDL_GL_GetProcAddress( "glCompressedTexImage2D" ); SDL_GL_CheckExtension("glCompressedTexImage2D", glCompressedTexImage2D);
  glTexImage3D               = (GLTEXIMAGE3DFN)SDL_GL_GetProcAddress( "glTexImage3D" ); SDL_GL_CheckExtension("glTexImage3D", glTexImage3D);
  #endif
  
  /* Buffers */
  
	glGenBuffers               = (GLGENBUFFERSFN)SDL_GL_GetProcAddress( "glGenBuffers" ); SDL_GL_CheckExtension("glGenBuffers", glGenBuffers);
	glBindBuffer               = (GLBINDBUFFERFN)SDL_GL_GetProcAddress( "glBindBuffer" ); SDL_GL_CheckExtension("glBindBuffer", glBindBuffer);
	glBufferData               = (GLBUFFERDATAFN)SDL_GL_GetProcAddress( "glBufferData" ); SDL_GL_CheckExtension("glBufferData", glBufferData);
	glGetBufferSubData         = (GLGETBUFFERSUBDATAFN)SDL_GL_GetProcAddress( "glGetBufferSubData" ); SDL_GL_CheckExtension("glGetBufferSubData", glGetBufferSubData);
	glDeleteBuffers            = (GLDELETEBUFFERSFN)SDL_GL_GetProcAddress( "glDeleteBuffers" ); SDL_GL_CheckExtension("glDeleteBuffers", glDeleteBuffers);
  glDrawBuffers              = (GLDRAWBUFFERSFN)SDL_GL_GetProcAddress( "glDrawBuffers" ); SDL_GL_CheckExtension("glDrawBuffers", glDrawBuffers);

  glGenRenderbuffers                = (GLGENRENDERBUFFERSFN)SDL_GL_GetProcAddress( "glGenRenderbuffers" ); SDL_GL_CheckExtension("glGenRenderbuffers", glGenRenderbuffers);
  glBindRenderbuffer                = (GLBINDRENDERBUFFERFN)SDL_GL_GetProcAddress( "glBindRenderbuffer" ); SDL_GL_CheckExtension("glBindRenderbuffer", glBindRenderbuffer);
  glRenderbufferStorage             = (GLRENDERBUFFERSTORAGEFN)SDL_GL_GetProcAddress( "glRenderbufferStorage" ); SDL_GL_CheckExtension("glRenderbufferStorage", glRenderbufferStorage);
  glRenderbufferStorageMultisample             = (GLRENDERBUFFERSTORAGEMULTISAMPLEFN)SDL_GL_GetProcAddress( "glRenderbufferStorageMultisample" ); SDL_GL_CheckExtension("glRenderbufferStorageMultisample", glRenderbufferStorageMultisample);
	glDeleteRenderbuffers             = (GLDELETERENDERBUFFERSFN)SDL_GL_GetProcAddress( "glDeleteRenderbuffers" ); SDL_GL_CheckExtension("glDeleteRenderbuffers", glDeleteRenderbuffers);
  
	glGenFramebuffers          = (GLGENFRAMEBUFFERSFN)SDL_GL_GetProcAddress( "glGenFramebuffers" ); SDL_GL_CheckExtension("glGenFramebuffers", glGenFramebuffers);
	glBindFramebuffer          = (GLBINDFRAMEBUFFERFN)SDL_GL_GetProcAddress( "glBindFramebuffer" ); SDL_GL_CheckExtension("glBindFramebuffer", glBindFramebuffer);
	glBlitFramebuffer          = (GLBLITFRAMEBUFFERFN)SDL_GL_GetProcAddress( "glBlitFramebuffer" ); SDL_GL_CheckExtension("glBlitFramebuffer", glBlitFramebuffer);
	glFramebufferTexture     = (GLFRAMEBUFFERTEXTUREFN)SDL_GL_GetProcAddress( "glFramebufferTexture" ); SDL_GL_CheckExtension("glFramebufferTexture", glFramebufferTexture);
	glFramebufferTexture2D     = (GLFRAMEBUFFERTEXTURE2DFN)SDL_GL_GetProcAddress( "glFramebufferTexture2D" ); SDL_GL_CheckExtension("glFramebufferTexture2D", glFramebufferTexture2D);
	glDeleteFramebuffers       = (GLDELETEFRAMEBUFFERSFN)SDL_GL_GetProcAddress( "glDeleteFramebuffers" ); SDL_GL_CheckExtension("glDeleteFramebuffers", glDeleteFramebuffers);
	glCheckFramebufferStatus   = (GLCHECKFRAMEBUFFERSTATUSFN)SDL_GL_GetProcAddress( "glCheckFramebufferStatus" ); SDL_GL_CheckExtension("glCheckFramebufferStatus", glCheckFramebufferStatus);
  glFramebufferRenderbuffer  = (GLFRAMEBUFFERRENDERBUFFERFN)SDL_GL_GetProcAddress( "glFramebufferRenderbuffer" ); SDL_GL_CheckExtension("glFramebufferRenderbuffer", glFramebufferRenderbuffer);
  
  /* Misc */
  
  glDrawElementsInstanced = (GLDRAWELEMENTSINSTANCEDFN)SDL_GL_GetProcAddress( "glDrawElementsInstanced" ); SDL_GL_CheckExtension("glDrawElementsInstanced", glDrawElementsInstanced);
  
}

void SDL_GL_CheckExtension(const char* name, void* function_pointer) {
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
  
  fflush(stdout);
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
  
  typedef USHORT (WINAPI *CaptureStackBackTraceFn)(ULONG,ULONG,PVOID*,PULONG);
  typedef BOOL (WINAPI *SymInitializeFn)(HANDLE,PCTSTR,BOOL);
  typedef BOOL (WINAPI *SymFromAddrFn)(HANDLE,DWORD64,PDWORD64,PSYMBOL_INFO);
  
  CaptureStackBackTraceFn CaptureStackBackTrace = (CaptureStackBackTraceFn)(GetProcAddress(LoadLibrary("kernel32.dll"), "RtlCaptureStackBackTrace"));
  SymInitializeFn SymInitialize = (SymInitializeFn)(GetProcAddress(LoadLibrary("Dbghelp.dll"), "SymInitialize"));
  SymFromAddrFn SymFromAddr = (SymFromAddrFn)(GetProcAddress(LoadLibrary("Dbghelp.dll"), "SymFromAddr"));
  
  if ((CaptureStackBackTrace == NULL) || 
      (SymInitialize == NULL) ||
      (SymFromAddr == NULL)) {
    printf("[STACK] Could not retrieve functions for stack trace\n");
    return;
  }
  
  HANDLE process = GetCurrentProcess();
  if (process == 0) {
    printf("[STACK] Could not retrieve current process\n");
    return;
  }
  
  if (SymInitialize(process, NULL, TRUE) == 0) {
    printf("[STACK] Could not ilitialize symbols for process\n");
    return;
  }
  
  if (sizeof(void*) != sizeof(DWORD64)) {
    printf("[STACK] Cannot retrive stack symbols on 32-bit binary\n");
    return;
  }
  
  SYMBOL_INFO* symbol = calloc(sizeof(SYMBOL_INFO) + 256, 1);
  symbol->MaxNameLen = 255;
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
  
  void* stack[32];
  int frames = CaptureStackBackTrace(0, 32, stack, NULL);
  
  printf ("[STACK] (%i frames)\n", frames);
  for(int i = 0; i < frames; i++ ){
    
    DWORD64 address = PtrToUlong(stack[i]);
    if (SymFromAddr(process, address, 0, symbol)) {
      printf("  %i: %s - %08X\n", frames-i-1, symbol->Name, (unsigned int)symbol->Address );
    } else {
      DWORD error = GetLastError();
      printf("  %i: SymFromAddr returned error %d\n", frames-i-1, (int)error);
    }
    
  }
  
  free(symbol);
}

#else

void SDL_PrintStackTrace() {}

#endif


