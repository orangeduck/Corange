#include "texture.h"
#include "game.h"
#include "timing.h"

#include "viewport.h"

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600

static SDL_Surface* screen;
static SDL_Surface* window_icon;

static int window_width;
static int window_height;

static int window_flags;
static int window_multisamples;
static int window_vsync;

void viewport_init() {

  window_width = DEFAULT_WIDTH;
  window_height = DEFAULT_HEIGHT;
  window_flags = SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL;
  window_multisamples = 0;
  window_vsync = 0;
  
  viewport_set_title("corange", "corange");
  
  viewport_start();
  
}

void viewport_finish() {

  SDL_FreeSurface(window_icon);
  SDL_FreeSurface(screen);

}

void viewport_start() {

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

void viewport_restart() {

#ifdef _WIN32
  /* Built temp context and share resources */
  SDL_SysWMinfo info;

  // get window handle from SDL
  SDL_VERSION(&info.version);
  if (SDL_GetWMInfo(&info) == -1) {
  printf("SDL_GetWMInfo #1 failed\n");
  }

  // get device context handle
  HDC tempDC = GetDC( info.window );

  // create temporary context
  HGLRC tempRC = wglCreateContext( tempDC );
  if (tempRC == NULL) {
  printf("wglCreateContext failed\n");
  }

  // share resources to temporary context
  SetLastError(0);
  if (!wglShareLists(info.hglrc, tempRC)) {
  printf("wglShareLists #1 failed\n");
  }
#endif
  
  /* Init new video mode */
  
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
  
  /* Get rid of temp context */
  
#ifdef _WIN32
  SDL_VERSION(&info.version);
  if (SDL_GetWMInfo(&info) == -1) {
  printf("SDL_GetWMInfo #2 failed\n");
  }

  // share resources to new SDL-created context
  if (!wglShareLists(tempRC, info.hglrc)) {
  printf("wglShareLists #2 failed\n");
  }

  // we no longer need our temporary context
  if (!wglDeleteContext(tempRC)) {
  printf("wglDeleteContext failed\n");
  }
#endif
  
}

void viewport_set_title(char* title, char* icon_title) {
  SDL_WM_SetCaption(title, icon_title);    
}

void viewport_set_vsync(int vsync) {
  window_vsync = vsync;
  viewport_restart();
}

void viewport_set_fullscreen(int fullscreen) {

  if (fullscreen) {
    window_flags |= SDL_FULLSCREEN;
  } else {
    window_flags &= !SDL_FULLSCREEN;
  }
  
  viewport_restart();
  
}


void viewport_set_multisamples(int samples) {
  
  window_multisamples = samples;
  viewport_restart();
  
}

void viewport_set_height(int height) {
  window_height = height;
  viewport_restart();
}

void viewport_set_width(int width) {
  window_width = width;
  viewport_restart();
}

void viewport_set_dimensions(vector2 dim) {
  window_width = dim.x;
  window_height = dim.y;
  viewport_restart();
}

int viewport_vsync() {
  return window_vsync;
}

int viewport_multisamples() {
  return window_multisamples;
}

int viewport_fullscreen() {
  if (window_flags & SDL_FULLSCREEN) {
    return 1;
  } else {
    return 0;
  }
}

int viewport_height() {
  return window_height;
}

int viewport_width() {
  return window_width;
}

vector2 viewport_dimensions() {
  return v2(window_width, window_height);
}

float viewport_ratio() {
  return (float)window_height / (float)window_width;
}

static char timestamp_string[64];
static char screenshot_string[256];

void viewport_screenshot() {
  
  unsigned char* image_data = malloc( sizeof(unsigned char) * viewport_width() * viewport_height() * 4 );
  glReadPixels( 0, 0, viewport_width(), viewport_height(), GL_BGRA, GL_UNSIGNED_BYTE, image_data ); 
  
  int xa= viewport_width() % 256;
  int xb= (viewport_width()-xa)/256;

  int ya= viewport_height() % 256;
  int yb= (viewport_height()-ya)/256;
  unsigned char header[18]={0,0,2,0,0,0,0,0,0,0,0,0,(char)xa,(char)xb,(char)ya,(char)yb,32,0};
  
  timestamp_sm(timestamp_string);

  screenshot_string[0] = '\0';
  strcat(screenshot_string, "./");
  strcat(screenshot_string, game_name());
  strcat(screenshot_string, "_");
  strcat(screenshot_string, timestamp_string);
  strcat(screenshot_string, ".tga");
  
  SDL_RWops* file = SDL_RWFromFile(screenshot_string, "wb");
  SDL_RWwrite(file, header, sizeof(char) * 18, 1);
  SDL_RWwrite(file, image_data, sizeof(char) * viewport_width() * viewport_height() * 4, 1 );
  SDL_RWclose(file);
  
  free(image_data);
  
}