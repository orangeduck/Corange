#ifdef _WIN32
  #include "SDL/SDL_syswm.h"
#endif

#include "bool.h"
#include "error.h"
#include "image.h"
#include "timing.h"

#include "graphics_manager.h"

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600

static SDL_Surface* screen;
static SDL_Surface* window_icon;

static int window_width;
static int window_height;

static int window_flags;
static int window_multisamples;
static bool window_vsync;

static void graphics_viewport_start() {

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, window_vsync);
  
  if (window_multisamples > 1) {
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, window_multisamples);
  } else {
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
  }
  
  screen = SDL_SetVideoMode(window_width, window_height, 0, window_flags);
  glViewport(0, 0, window_width, window_height);
  
}

void graphics_manager_init() {

  SDL_Init(SDL_INIT_VIDEO);

  window_width = DEFAULT_WIDTH;
  window_height = DEFAULT_HEIGHT;
  window_flags = SDL_OPENGL;
  window_multisamples = 16;
  window_vsync = 1;
  
  graphics_viewport_set_title("corange", "corange");
  graphics_viewport_start();
  
  SDL_GL_LoadExtensions();
  SDL_GL_PrintInfo();
  
}

void graphics_manager_finish() {

  SDL_FreeSurface(window_icon);
  SDL_FreeSurface(screen);

}

void graphics_viewport_restart() {

#ifdef _WIN32
  /* Built temp context and share resources */
  
  SDL_SysWMinfo info;
  SDL_VERSION(&info.version);
  if (SDL_GetWMInfo(&info) == -1) {
    error("Could not get SDL version info.");
  }

  /* get device context handle */
  HDC tempDC = GetDC( info.window );

  /* create temporary context */
  HGLRC tempRC = wglCreateContext( tempDC );
  if (tempRC == NULL) {
    error("Could not create OpenGL context");
  }

  /* share resources to temporary context */
  SetLastError(0);
  if (!wglShareLists(info.hglrc, tempRC)) {
    error("Could not get OpenGL share lists.");
  }
#endif
  
  graphics_viewport_start();
  
#ifdef _WIN32

  /* Get rid of temp context */

  SDL_VERSION(&info.version);
  if (SDL_GetWMInfo(&info) == -1) {
    error("Could not get SDL version info.");
  }

  /* share resources to new SDL-created context */
  if (!wglShareLists(tempRC, info.hglrc)) {
    error("Could not create OpenGL context");
  }

  /* we no longer need our temporary context */
  if (!wglDeleteContext(tempRC)) {
    error("Could not get OpenGL share lists.");
  }
#endif
  
}

void graphics_viewport_set_title(char* title, char* icon_title) {
  SDL_WM_SetCaption(title, icon_title);    
}

void graphics_set_vsync(bool vsync) {
  window_vsync = vsync;
  graphics_viewport_restart();
}

void graphics_set_fullscreen(bool fullscreen) {

  if (fullscreen) {
    window_flags |= SDL_FULLSCREEN;
  } else {
    window_flags &= !SDL_FULLSCREEN;
  }
  
  graphics_viewport_restart();
}

bool graphics_get_vsync() {
  return window_vsync;
}

int graphics_get_multisamples() {
  return window_multisamples;
}

bool graphics_get_fullscreen() {
  if (window_flags & SDL_FULLSCREEN) {
    return true;
  } else {
    return false;
  }
}


void graphics_set_multisamples(int samples) {
  
  window_multisamples = samples;
  graphics_viewport_restart();
  
}

void graphics_viewport_set_height(int height) {
  window_height = height;
  graphics_viewport_restart();
}

void graphics_viewport_set_width(int width) {
  window_width = width;
  graphics_viewport_restart();
}

void graphics_viewport_set_dimensions(int width, int height) {
  window_width = width;
  window_height = height;
  graphics_viewport_restart();
}

int graphics_viewport_height() {
  return window_height;
}

int graphics_viewport_width() {
  return window_width;
}

float graphics_viewport_ratio() {
  return (float)window_height / (float)window_width;
}

static char timestamp_string[64];
static char screenshot_string[256];

void graphics_viewport_screenshot() {
  
  unsigned char* image_data = malloc( sizeof(unsigned char) * graphics_viewport_width() * graphics_viewport_height() * 4 );
  glReadPixels( 0, 0, graphics_viewport_width(), graphics_viewport_height(), GL_RGBA, GL_UNSIGNED_BYTE, image_data ); 
  
  image* i = image_new(graphics_viewport_width(), graphics_viewport_height(), image_data);
  
  free(image_data);
  
  timestamp_sm(timestamp_string);

  screenshot_string[0] = '\0';
  strcat(screenshot_string, "./corange_");
  strcat(screenshot_string, timestamp_string);
  strcat(screenshot_string, ".tga");
  
  image_write_to_file(i, screenshot_string);
  
  image_delete(i);
  
}
