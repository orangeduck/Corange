#include "SDL/SDL_local.h"
#include "SDL/SDL_rwops.h"

#ifdef _WIN32
  #include "SDL/SDL_syswm.h"
  #include <windows.h>
  #include <winbase.h>
#endif

#ifdef __unix__
  #include <execinfo.h>
#endif

#ifdef _WIN32

void SDL_PathFullName(char* dst, const char* path) {
  GetFullPathName(path, MAX_PATH, dst, NULL);
}

#elif __unix__

void SDL_PathFullName(char* dst, const char* path) {
  char* ret = realpath(path, dst);
}

#endif

void SDL_PathFileName(char* dst, const char* path) {
  
  int i = strlen(path);
  int ext_loc = 0;
  while( i > 0) {
    if (path[i] == '/') { break; }
    if (path[i] == '\\') { break; }
    if (path[i] == '.') { ext_loc = i; }
    i--;
  }
  
  const char* file = path + i + 1;
  int len = ext_loc - i - 1;
  
  strncpy(dst, file, len);
  dst[len] = '\0';
}

void SDL_PathFileExtension(char* dst, const char* path) {

  int ext_len = 0;
  int i = strlen(path);
  while( i >= 0) {
    if (path[i] != '.') { ext_len++; }
    if (path[i] == '.') { break; }
    i--;
  }
  
  int prev = strlen(path) - ext_len + 1;
  const char* f_ext = path + prev;
  strcpy(dst, f_ext);
}

void SDL_PathFileLocation(char* dst, const char* path) {
  
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

void SDL_PathRelative(char* dst, const char* path) {
  char* curr = SDL_GetWorkingDir();
  char* sub = strstr(path, curr);
  
  if (sub == NULL) {
    strcpy(dst, path);
  } else {
    strcpy(dst, ".");
    strcat(dst, path + strlen(curr));
  }
  
}

#else

void SDL_PathRelative(char* dst, const char* path) {
  error("Unimplemented!");
}

#endif

void SDL_PathForwardSlashes(char* path) {
  for(int i = 0; i < strlen(path); i++) {
    if (path[i] == '\\') { path[i] = '/'; }
  }
}

#ifdef _WIN32
  #include <direct.h>
  #define getcwd _getcwd
  #define chdir _chdir
#else
  #include <unistd.h>
#endif

 
static char curr_dir[MAX_PATH];
char* SDL_GetWorkingDir() {
  getcwd(curr_dir, sizeof(curr_dir));
  return curr_dir;
}

void SDL_SetWorkingDir(char* dir) {
  chdir(dir);
}

void SDL_RWsize(SDL_RWops* file, int* size) {
  int pos = SDL_RWtell(file);
  *size = SDL_RWseek(file, 0, SEEK_END);
  SDL_RWseek(file, pos, SEEK_SET);
}

int SDL_RWreadline(SDL_RWops* file, char* buffer, int buffersize) {
  
  char c;
  int status = 0;
  int i = 0;
  while(1) {
    
    status = SDL_RWread(file, &c, 1, 1);
    
    if (status == -1) return -1;
    if (i == buffersize-1) return -1;
    if (status == 0) break;
    
    buffer[i] = c;
    i++;
    
    if (c == '\n') {
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

#ifndef GCL_HICON
  #define GCL_HICON -14
  #define GCL_HICONSM -34
#endif

int SDL_WM_UseResourceIcon() {

  HINSTANCE handle = GetModuleHandle(NULL);
  icon = LoadIcon(handle, "icon");
  
  SDL_SysWMinfo wminfo;
  SDL_VERSION(&wminfo.version)
  if (SDL_GetWMInfo(&wminfo) != 1) {
    return 0;
  }

  SetClassLong(wminfo.window, GCL_HICON, (LONG)icon);
  SetClassLong(wminfo.window, GCL_HICONSM, (LONG)icon);
  
  return 1;
}

void SDL_WM_DeleteResourceIcon() {
  DestroyIcon(icon);
}

#else

void SDL_WM_UseResourceIcon() {}
void SDL_WM_DeleteResourceIcon() {}

#endif

#ifdef _WIN32

static HDC temp_device;
static HGLRC temp_context;

int SDL_WM_CreateTempContext() {
  
  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);
  if (SDL_GetWMInfo(&info) == -1) {
    // Could not get SDL version info.
    return 1;
  }
  
  temp_device = GetDC(info.window);

  temp_context = wglCreateContext(temp_device);
  if (temp_context == NULL) {
    // Could not create OpenGL context
    return 2;
  }

  if (!wglShareLists(info.hglrc, temp_context)) {
    // Could not share lists with temp context.
    return 3;
  }
  
  return 0;
  
}

int SDL_WM_DeleteTempContext() {

  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);
  if (SDL_GetWMInfo(&info) == -1) {
    // Could not get SDL version info
    return 1;
  }

  if (!wglShareLists(temp_context, info.hglrc)) {
    // Could share lists with OpenGL context
    return 2;
  }

  if (!wglDeleteContext(temp_context)) {
    // Could delete OpenGL context
    return 3;
  }
  
  return 0;
}

#else

int SDL_WM_CreateTempContext() {}
int SDL_WM_DeleteTempContext() {}

#endif


void SDL_GL_PrintInfo() {
  const char* vendor = (const char*)glGetString(GL_VENDOR);
  const char* renderer = (const char*)glGetString(GL_RENDERER);
  const char* version = (const char*)glGetString(GL_VERSION);
  const char* shader_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
  
  printf("OpenGL Info\n");
  printf("Vendor: %s\n", vendor);
  printf("Renderer: %s\n", renderer);
  printf("Version: %s\n", version);
  printf("Shader Version: %s\n", shader_version);
}

void SDL_GL_PrintExtensions() {
  const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
  printf("OpenGL Extensions: %s\n", extensions);
}

static const char* gl_error_string_invalid_enum = "Invalid Enum";
static const char* gl_error_string_invalid_value = "Invalid Value";
static const char* gl_error_string_invalid_operation = "Invalid Operation";
static const char* gl_error_string_out_of_memory = "Out of Memory";
static const char* gl_error_string_invalid_framebuffer_operation = "Invalid Framebuffer Operation";
static const char* gl_error_string_stack_overflow = "Stack Overflow";
static const char* gl_error_string_stack_underflow = "Stack Underflow";
static const char* gl_error_string_table_too_large = "Table Too Large";
static const char* gl_error_string_no_error = "No Error";

const char* SDL_GL_ErrorString(GLenum error) {
  switch (error) {
    case GL_INVALID_ENUM:
      return gl_error_string_invalid_enum;
    case GL_INVALID_VALUE:
      return gl_error_string_invalid_value;
    case GL_INVALID_OPERATION:
      return gl_error_string_invalid_operation;
    case GL_OUT_OF_MEMORY:
      return gl_error_string_out_of_memory;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      return gl_error_string_invalid_framebuffer_operation;
    case GL_STACK_OVERFLOW:
      return gl_error_string_stack_overflow;
    case GL_STACK_UNDERFLOW:
      return gl_error_string_stack_underflow;
    case GL_TABLE_TOO_LARGE:
      return gl_error_string_table_too_large;
  }
  return gl_error_string_no_error;
}

static const char* gl_error_string_framebuffer_complete = "Framebuffer Complete";
static const char* gl_error_string_framebuffer_undefined = "Framebuffer Undefined";
static const char* gl_error_string_framebuffer_incomplete_attach = "Framebuffer Incomplete Attachment";
static const char* gl_error_string_framebuffer_incomplete_missing_attach = "Framebuffer No Attachments";
static const char* gl_error_string_framebuffer_incomplete_draw = "Framebuffer Incomplete Draw";
static const char* gl_error_string_framebuffer_incomplete_read = "Framebuffer Incomplete Read";
static const char* gl_error_string_framebuffer_unsupported = "Framebuffer Unsupported";
static const char* gl_error_string_framebuffer_incomplete_multisample = "Framebuffer Badly Configured Multisamples";
static const char* gl_error_string_framebuffer_incomplete_layer_targets = "Framebuffer Badly Configured Layer Targets";

const char* SDL_GL_FrameBufferErrorString(GLenum error) {
  switch(error) {
    case GL_FRAMEBUFFER_COMPLETE:
      return gl_error_string_framebuffer_complete;
    case GL_FRAMEBUFFER_UNDEFINED:
      return gl_error_string_framebuffer_undefined;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      return gl_error_string_framebuffer_incomplete_attach;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      return gl_error_string_framebuffer_incomplete_missing_attach;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      return gl_error_string_framebuffer_incomplete_draw;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      return gl_error_string_framebuffer_incomplete_read;
    case GL_FRAMEBUFFER_UNSUPPORTED:
      return gl_error_string_framebuffer_unsupported;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
      return gl_error_string_framebuffer_incomplete_multisample;
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
      return gl_error_string_framebuffer_incomplete_layer_targets;
  }
  return gl_error_string_no_error;
}

#ifdef _WIN32

static HDC gl_thread_device = NULL;
static HGLRC gl_thread_context = NULL;
static void* gl_thread_data = NULL;
static int (*gl_thread_func)(void*) = NULL; 

static int gl_thread_create(void* unused) {
  
  BOOL err = wglMakeCurrent(gl_thread_device, gl_thread_context);
  if (err == 0) {
    // Could not make context current
    return -1;
  }
  
  int status = gl_thread_func(gl_thread_data);
  
  HGLRC context = wglGetCurrentContext();
  if (context == NULL) {
    // Could not get current context
    return -2;
  }
  
  wglMakeCurrent(NULL, NULL);
  
  err = wglDeleteContext(context);
  if (err == 0) {
    // Could not delete context
    return -3;
  }
  
  return status;
}

SDL_Thread* SDL_GL_CreateThread(int (*fn)(void *), void *data) {

  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);
  if (SDL_GetWMInfo(&info) == -1) {
    // Could not get SDL version info.
    return NULL;
  }
  
  gl_thread_device = GetDC(info.window);

  gl_thread_context = wglCreateContext(gl_thread_device);
  if (gl_thread_context == NULL) {
    // Could not create new OpenGL context
    return NULL;
  }
  
  BOOL err = wglShareLists(info.hglrc, gl_thread_context);
  if (err == 0) {
    int code = GetLastError();
    //Could not get OpenGL share lists: %i
    return NULL;
  }
  
  gl_thread_func = fn;
  gl_thread_data = data;
  
  return SDL_CreateThread(gl_thread_create, NULL);

}

#elif __unix__

#include <GL/glx.h>

static Display* gl_thread_display = NULL;
static GLXContext gl_thread_context = NULL;
static GLXDrawable gl_thread_drawable = None;
static void* gl_thread_data = NULL;
static int (*gl_thread_func)(void*) = NULL; 

static int gl_thread_create(void* unused) {
  
  int err = glXMakeCurrent(gl_thread_display, gl_thread_drawable, gl_thread_context);
  if (err == 0) {
    // Could not make context current
    return -1
  }
  
  int status = gl_thread_func(gl_thread_data);
  
  Display* display = glXGetCurrentDisplay();
  if (display == NULL) {
    // Could not get current display
    return -2;
  }
  
  GLXContext context = glXGetCurrentContext();
  if (context == NULL) {
    // Could not get current context
    return -3;
  }
  
  err = glXMakeCurrent(display, None, NULL);
  if (err == 0) {
    // Could not make context current
    return -4;
  }
  
  glXDestroyContext(display, context);
  
  return status;
}

SDL_Thread* SDL_GL_CreateThread(int (*fn)(void *), void *data) {
  
  int attribs[8] = {GLX_RGBA, GLX_RED_SIZE, 4, GLX_GREEN_SIZE, 4, GLX_BLUE_SIZE, 4, None};
  
  GLXContext context = glXGetCurrentContext();
  if (context == NULL) {
    // Could not get current context
    return NULL;
  }
  
  Display* display = glXGetCurrentDisplay();
  if (display == NULL) {
    // Could not get current display
    return NULL;
  }
  
  GLXDrawable drawable = glXGetCurrentDrawable();
  if (drawable == None) {
    // Could not get current drawable
    return NULL;
  }
  
  XVisualInfo* info = malloc(sizeof(XVisualInfo));
  info = glXChooseVisual(display, 0, attribs);
  if (info == NULL) {
    // Could not create thread with required visuals.
    return NULL;
  }
  
  gl_thread_display = display;
  gl_thread_drawable = drawable;
  gl_thread_context = glXCreateContext(display, info, context, True);
  
  gl_thread_func = fn;
  gl_thread_data = data;
  
  XFree(info);
  
  return SDL_CreateThread(gl_thread_create, NULL);

}

#endif

bool SDL_GL_ExtensionPresent(char* name) {
  const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
  if (strstr(extensions, name)) {
    return true;
  } else {
    return false;
  }
}

bool SDL_GL_ExtensionFuncionLoaded(void* function) {
  if (function == NULL) {
    return false;
  } else {
    return true;
  }
}

#ifndef __unix__
GLACTIVETEXTUREFN glActiveTexture = NULL;
GLCOMPRESSEDTEXIMAGE2DFN glCompressedTexImage2D = NULL;
GLTEXIMAGE3DFN glTexImage3D = NULL;
#endif
GLCREATESHADERFN glCreateShader = NULL;
GLCREATEPROGRAMFN glCreateProgram = NULL;
GLSHADERSOURCEFN glShaderSource = NULL;
GLCOMPILESHADERFN glCompileShader = NULL;
GLGETSHADERINFOLOGFN glGetShaderInfoLog = NULL;
GLATTACHSHADERFN glAttachShader = NULL;
GLLINKPROGRAMFN glLinkProgram = NULL;
GLGETPROGRAMINFOLOGFN glGetProgramInfoLog = NULL;
GLISPROGRAMFN glIsProgram = NULL;
GLISSHADERFN glIsShader = NULL;
GLGETATTACHEDSHADERSFN glGetAttachedShaders = NULL;
GLGETUNIFORMLOCATIONFN glGetUniformLocation = NULL;
GLUNIFORM1FFN glUniform1f = NULL;
GLUNIFORM1IFN glUniform1i = NULL;
GLDELETESHADERFN glDeleteShader = NULL;
GLDELETEPROGRAMFN glDeleteProgram = NULL;
GLUSEPROGRAMFN glUseProgram = NULL;
GLVERTEXATTRIBPOINTERFN glVertexAttribPointer = NULL;
GLENABLEVERTEXATTRIBARRAYFN glEnableVertexAttribArray = NULL;
GLDISABLEVERTEXATTRIBARRAYFN glDisableVertexAttribArray = NULL;
GLUNIFORM2FFN glUniform2f = NULL;
GLUNIFORM3FFN glUniform3f = NULL;
GLUNIFORM4FFN glUniform4f = NULL;
GLUNIFORMMATRIX4FVFN glUniformMatrix4fv = NULL;
GLUNIFORM1FVFN glUniform1fv = NULL;
GLUNIFORM2FVFN glUniform2fv = NULL;
GLUNIFORM3FVFN glUniform3fv = NULL;
GLGETSHADERIVFN glGetShaderiv = NULL;
GLPROGRAMPARAMETERIFN glProgramParameteri = NULL;
GLGETPROGRAMIVFN glGetProgramiv = NULL;
GLBINDATTRIBLOCATIONFN glBindAttribLocation = NULL;
GLGENFRAMEBUFFERSFN glGenFramebuffers = NULL;
GLBINDFRAMEBUFFERFN glBindFramebuffer = NULL;
GLBLITFRAMEBUFFERFN glBlitFramebuffer = NULL;
GLFRAMEBUFFERTEXTUREFN glFramebufferTexture = NULL;
GLFRAMEBUFFERTEXTURE2DFN glFramebufferTexture2D = NULL;
GLDELETEFRAMEBUFFERSFN glDeleteFramebuffers = NULL;
GLCHECKFRAMEBUFFERSTATUSFN glCheckFramebufferStatus = NULL;
GLGENBUFFERSFN glGenBuffers = NULL;
GLGENRENDERBUFFERSFN glGenRenderbuffers = NULL;
GLDELETEBUFFERSFN glDeleteBuffers = NULL;
GLDELETERENDERBUFFERSFN glDeleteRenderbuffers = NULL;
GLBINDBUFFERFN glBindBuffer = NULL;
GLBINDRENDERBUFFERFN glBindRenderbuffer = NULL;
GLBUFFERDATAFN glBufferData = NULL;
GLGETBUFFERSUBDATAFN glGetBufferSubData = NULL;
GLFRAMEBUFFERRENDERBUFFERFN glFramebufferRenderbuffer = NULL;
GLGETATTRIBLOCATIONFN glGetAttribLocation = NULL;
GLRENDERBUFFERSTORAGEFN glRenderbufferStorage = NULL;
GLRENDERBUFFERSTORAGEMULTISAMPLEFN glRenderbufferStorageMultisample = NULL;
GLDRAWBUFFERSFN glDrawBuffers = NULL;
GLGENERATEMIPMAPFN glGenerateMipmap = NULL;
GLDRAWELEMENTSINSTANCEDFN glDrawElementsInstanced = NULL;
GLPATCHPARAMETERIFN glPatchParameteri = NULL;
GLPATCHPARAMETERFVFN glPatchParameterfv = NULL;

GLBROKENEXTENSIONFN glBrokenExtension = NULL;

#define SDL_GL_LoadExtension(type, name) \
name = (type)SDL_GL_GetProcAddress(#name); \
if (name == NULL) { \
  fprintf(stderr, "Failed to load function '%s', looking for function '%s'...", #name, #name"EXT"); \
  name = (type)SDL_GL_GetProcAddress(#name"EXT"); \
} \
if (name == NULL) { \
  fprintf(stderr, "Failed to load function '%s', looking for function '%s'...", #name"EXT", #name"ARB"); \
  name = (type)SDL_GL_GetProcAddress(#name"ARB"); \
} \
if (name == NULL) { fprintf(stderr, "Completely failed to load OpenGL extension function '%s'. Use of this function will crash Corange", #name); }
  
void SDL_GL_LoadExtensions() {

  /* Shaders */
  
  SDL_GL_LoadExtension(GLCREATEPROGRAMFN, glCreateProgram);
  SDL_GL_LoadExtension(GLLINKPROGRAMFN, glLinkProgram);
  SDL_GL_LoadExtension(GLDELETEPROGRAMFN, glDeleteProgram);
  SDL_GL_LoadExtension(GLGETPROGRAMINFOLOGFN, glGetProgramInfoLog);
  SDL_GL_LoadExtension(GLUSEPROGRAMFN, glUseProgram);
  SDL_GL_LoadExtension(GLGETPROGRAMIVFN, glGetProgramiv);
  SDL_GL_LoadExtension(GLPROGRAMPARAMETERIFN, glProgramParameteri);
  
  SDL_GL_LoadExtension(GLCREATESHADERFN, glCreateShader);
  SDL_GL_LoadExtension(GLSHADERSOURCEFN, glShaderSource);
  SDL_GL_LoadExtension(GLCOMPILESHADERFN, glCompileShader);
  SDL_GL_LoadExtension(GLGETSHADERINFOLOGFN, glGetShaderInfoLog);
  SDL_GL_LoadExtension(GLATTACHSHADERFN, glAttachShader);
  SDL_GL_LoadExtension(GLDELETESHADERFN, glDeleteShader);
  SDL_GL_LoadExtension(GLGETSHADERIVFN, glGetShaderiv);
  SDL_GL_LoadExtension(GLISPROGRAMFN, glIsProgram);
  SDL_GL_LoadExtension(GLISSHADERFN, glIsShader);
  SDL_GL_LoadExtension(GLGETATTACHEDSHADERSFN, glGetAttachedShaders);
  
  SDL_GL_LoadExtension(GLGETUNIFORMLOCATIONFN, glGetUniformLocation);
  SDL_GL_LoadExtension(GLUNIFORM1FFN, glUniform1f);
  SDL_GL_LoadExtension(GLUNIFORM1IFN, glUniform1i);
  SDL_GL_LoadExtension(GLUNIFORM2FFN, glUniform2f);
  SDL_GL_LoadExtension(GLUNIFORM3FFN, glUniform3f);
  SDL_GL_LoadExtension(GLUNIFORM4FFN, glUniform4f);
  SDL_GL_LoadExtension(GLUNIFORM1FVFN, glUniform1fv);
  SDL_GL_LoadExtension(GLUNIFORM2FVFN, glUniform2fv);
  SDL_GL_LoadExtension(GLUNIFORM3FVFN, glUniform3fv);
  SDL_GL_LoadExtension(GLUNIFORMMATRIX4FVFN, glUniformMatrix4fv);
  
  /* Attributes */
  
  SDL_GL_LoadExtension(GLGETATTRIBLOCATIONFN, glGetAttribLocation);
  SDL_GL_LoadExtension(GLVERTEXATTRIBPOINTERFN, glVertexAttribPointer);
  SDL_GL_LoadExtension(GLENABLEVERTEXATTRIBARRAYFN, glEnableVertexAttribArray);
  SDL_GL_LoadExtension(GLDISABLEVERTEXATTRIBARRAYFN, glDisableVertexAttribArray);
  SDL_GL_LoadExtension(GLBINDATTRIBLOCATIONFN, glBindAttribLocation);
  
  /* Textures */
  
  SDL_GL_LoadExtension(GLGENERATEMIPMAPFN, glGenerateMipmap);
  #ifndef __unix__
  SDL_GL_LoadExtension(GLACTIVETEXTUREFN, glActiveTexture);
  SDL_GL_LoadExtension(GLCOMPRESSEDTEXIMAGE2DFN, glCompressedTexImage2D);
  SDL_GL_LoadExtension(GLTEXIMAGE3DFN, glTexImage3D);
  #endif
  
  /* Buffers */
  
  SDL_GL_LoadExtension(GLGENBUFFERSFN, glGenBuffers);
  SDL_GL_LoadExtension(GLBINDBUFFERFN, glBindBuffer);
  SDL_GL_LoadExtension(GLBUFFERDATAFN, glBufferData);
  SDL_GL_LoadExtension(GLGETBUFFERSUBDATAFN, glGetBufferSubData);
  SDL_GL_LoadExtension(GLDELETEBUFFERSFN, glDeleteBuffers);
  SDL_GL_LoadExtension(GLDRAWBUFFERSFN, glDrawBuffers);
  
  SDL_GL_LoadExtension(GLGENRENDERBUFFERSFN, glGenRenderbuffers);
  SDL_GL_LoadExtension(GLBINDRENDERBUFFERFN, glBindRenderbuffer);
  SDL_GL_LoadExtension(GLRENDERBUFFERSTORAGEFN, glRenderbufferStorage);
  SDL_GL_LoadExtension(GLRENDERBUFFERSTORAGEMULTISAMPLEFN, glRenderbufferStorageMultisample);
  SDL_GL_LoadExtension(GLDELETERENDERBUFFERSFN, glDeleteRenderbuffers);
  
  SDL_GL_LoadExtension(GLGENFRAMEBUFFERSFN, glGenFramebuffers);
  SDL_GL_LoadExtension(GLBINDFRAMEBUFFERFN, glBindFramebuffer);
  SDL_GL_LoadExtension(GLBLITFRAMEBUFFERFN, glBlitFramebuffer);
  SDL_GL_LoadExtension(GLFRAMEBUFFERTEXTUREFN, glFramebufferTexture);
  SDL_GL_LoadExtension(GLFRAMEBUFFERTEXTURE2DFN, glFramebufferTexture2D);
  SDL_GL_LoadExtension(GLDELETEFRAMEBUFFERSFN, glDeleteFramebuffers);
  SDL_GL_LoadExtension(GLCHECKFRAMEBUFFERSTATUSFN, glCheckFramebufferStatus);
  SDL_GL_LoadExtension(GLFRAMEBUFFERRENDERBUFFERFN, glFramebufferRenderbuffer);
  
  /* Tessellation */
  
  SDL_GL_LoadExtension(GLPATCHPARAMETERIFN, glPatchParameteri);
  SDL_GL_LoadExtension(GLPATCHPARAMETERFVFN, glPatchParameterfv);
  
  /* Misc */
  
  SDL_GL_LoadExtension(GLDRAWELEMENTSINSTANCEDFN, glDrawElementsInstanced);
  
  /* Test for missing Extension */
  
  //SDL_GL_LoadExtension(GLBROKENEXTENSIONFN, glBrokenExtension);
  
}

#ifdef __unix__

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


