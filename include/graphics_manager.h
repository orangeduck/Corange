#ifndef graphics_manager_h
#define graphics_manager_h

#include "SDL/SDL.h"
#include "SDL/SDL_local.h"
#include "SDL/SDL_opengl.h"

#include "bool.h"

void graphics_manager_init();
void graphics_manager_finish();

void graphics_set_vsync(bool vsync);
void graphics_set_multisamples(int samples);
void graphics_set_fullscreen(bool fullscreen);

bool graphics_get_vsync();
int graphics_get_multisamples();
bool graphics_get_fullscreen();

void graphics_viewport_restart();
void graphics_viewport_set_title(char* title, char* icon_title);
void graphics_viewport_set_height(int height);
void graphics_viewport_set_width(int width);
void graphics_viewport_set_dimensions(int width, int height);
void graphics_viewport_screenshot();

int graphics_viewport_height();
int graphics_viewport_width();
float graphics_viewport_ratio();


#endif
