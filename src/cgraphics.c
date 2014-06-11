#include "cgraphics.h"
#include "casset.h"

#include "assets/image.h"

static SDL_Window* screen = NULL;
static SDL_GLContext* context = NULL; 

static int window_flags = 0;
static int window_multisamples = 0;
static int window_multisamplesbuffs = 0;
static int window_antialiasing = 0;

static void graphics_viewport_start() {
  
  screen = SDL_CreateWindow("Corange",
                          SDL_WINDOWPOS_UNDEFINED,
                          SDL_WINDOWPOS_UNDEFINED,
                          800, 600,
                          window_flags);

  if (screen == NULL) {
    error("Could not create SDL window: %s", SDL_GetError());
  }

  graphics_viewport_set_icon(P("$CORANGE/ui/corange.bmp"));
  
  SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
  context = SDL_GL_CreateContext(screen);
  
  if (context == NULL) {
    error("Could not create SDL context: %s", SDL_GetError());
  }
  
  SDL_GL_SetSwapInterval(1);
  SDL_GL_LoadExtensions();

  glViewport(0, 0, 800, 600);
  
}

void graphics_init() {

  int error = SDL_InitSubSystem(SDL_INIT_VIDEO);
  
  if (error == -1) {
    error("Cannot initialize SDL video!");
  }

  window_flags = SDL_WINDOW_OPENGL;
  window_multisamples = 4;
  window_multisamplesbuffs = 1;
  window_antialiasing = 1;
  
  graphics_viewport_start();
  
  SDL_GL_PrintInfo();
  SDL_GL_PrintExtensions();
}

SDL_GLContext* graphics_context_new() {
  return SDL_GL_CreateContext(screen);
}

void graphics_context_delete(SDL_GLContext* context) {
  SDL_GL_DeleteContext(context);
}

void graphics_context_current(SDL_GLContext* context) {
  SDL_GL_MakeCurrent(screen, context);
  SDL_GL_LoadExtensions();
}

void graphics_set_antialiasing(int quality) {
  window_antialiasing = quality;
}

int graphics_get_antialiasing() {
  return window_antialiasing;
}

void graphics_finish() {
  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(screen);
}

void graphics_set_multisamples(int multisamples) {
  window_multisamples = multisamples;
  if (multisamples > 0) {
    window_multisamplesbuffs = 1;
  } else {
    window_multisamplesbuffs = 0;
  }
}

int graphics_get_multisamples() {
  return window_multisamples;
}

void graphics_set_vsync(bool vsync) {
  SDL_GL_SetSwapInterval(vsync);
}

void graphics_set_fullscreen(bool fullscreen) {

  if (fullscreen) {
    window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  } else {
    window_flags &= !SDL_WINDOW_FULLSCREEN_DESKTOP;
  }
  
}

bool graphics_get_fullscreen() {
  if (window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
    return true;
  } else {
    return false;
  }
}

void graphics_viewport_set_size(int w, int h) {
  SDL_SetWindowSize(screen, w, h);
  glViewport(0, 0, w, h);
}

int graphics_viewport_height() {
  int w, h;
  SDL_GetWindowSize(screen, &w, &h);
  return h;
}

int graphics_viewport_width() {
  int w, h;
  SDL_GetWindowSize(screen, &w, &h);
  return w;
}

float graphics_viewport_ratio() {
  int w, h;
  SDL_GetWindowSize(screen, &w, &h);
  return (float)h / (float)w;
}

void graphics_viewport_set_title(const char* title) {
  SDL_SetWindowTitle(screen, title);
}

void graphics_viewport_set_icon(fpath icon) {
  SDL_Surface* window_icon = SDL_LoadBMP(asset_hndl_new(icon).path.ptr);
  SDL_SetWindowIcon(screen, window_icon);
  SDL_FreeSurface(window_icon);
}

const char* graphics_viewport_title() {
  return SDL_GetWindowTitle(screen); 
}

static char timestamp_string[256];
static char screenshot_string[256];

void graphics_viewport_screenshot() {
  
  unsigned char* image_data = malloc( sizeof(unsigned char) * graphics_viewport_width() * graphics_viewport_height() * 4 );
  glReadPixels( 0, 0, graphics_viewport_width(), graphics_viewport_height(), GL_BGRA, GL_UNSIGNED_BYTE, image_data ); 
  
  image* i = image_new(graphics_viewport_width(), graphics_viewport_height(), image_data);
  image_flip_vertical(i);
  image_bgr_to_rgb(i);
  
  free(image_data);
  
  timestamp(timestamp_string);

  screenshot_string[0] = '\0';
  strcat(screenshot_string, "./corange_");
  strcat(screenshot_string, timestamp_string);
  strcat(screenshot_string, ".tga");
  
  image_write_to_file(i, screenshot_string);
  
  image_delete(i);
  
}

void graphics_set_cursor_hidden(bool hidden) {
  SDL_ShowCursor(hidden ? SDL_DISABLE : SDL_ENABLE);
}

bool graphics_get_cursor_hidden() {
  return (SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE ? false : true);
}

void graphics_swap() {
  SDL_GL_SwapWindow(screen);
}
