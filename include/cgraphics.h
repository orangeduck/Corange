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

bool graphics_get_vsync();
int graphics_get_multisamples();
bool graphics_get_fullscreen();

void graphics_viewport_restart();
void graphics_viewport_set_title(char* title);
void graphics_viewport_set_height(int height);
void graphics_viewport_set_width(int width);
void graphics_viewport_set_dimensions(int width, int height);
void graphics_viewport_screenshot();

char* graphics_viewport_title();
int graphics_viewport_height();
int graphics_viewport_width();
float graphics_viewport_ratio();


#endif
