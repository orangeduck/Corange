/**
*** :: Graphics ::
***
***   Basic layer to Graphics System
***
***   See 'renderers' folder for actual graphical rendering.
***
**/

#ifndef cgraphics_h
#define cgraphics_h

#include "cengine.h"

void graphics_init();
void graphics_finish();

void graphics_set_vsync(bool vsync);
void graphics_set_multisamples(int samples);
void graphics_set_fullscreen(bool fullscreen);
void graphics_set_antialiasing(int quality);

SDL_GLContext* graphics_context_new();
void graphics_context_delete(SDL_GLContext* context);
void graphics_context_current(SDL_GLContext* context);

int graphics_get_multisamples();
bool graphics_get_fullscreen();
int graphics_get_antialiasing();

void graphics_viewport_set_title(const char* title);
void graphics_viewport_set_icon(fpath icon);
void graphics_viewport_set_position(int x, int y);
void graphics_viewport_set_size(int w, int h);
void graphics_viewport_screenshot();

const char* graphics_viewport_title();
int graphics_viewport_height();
int graphics_viewport_width();
float graphics_viewport_ratio();

void graphics_set_cursor_hidden(bool hidden);
bool graphics_get_cursor_hidden();

void graphics_swap();

#endif
