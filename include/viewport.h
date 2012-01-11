#ifndef viewport_h
#define viewport_h

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"

#include "bool.h"
#include "vector.h"

void viewport_init();
void viewport_finish();

void viewport_start();
void viewport_restart();

void viewport_set_title(char* title, char* icon_title);

void viewport_set_vsync(bool vsync);
void viewport_set_multisamples(int samples);
void viewport_set_fullscreen(bool fullscreen);

void viewport_set_height(int height);
void viewport_set_width(int width);
void viewport_set_dimensions(vector2 dim);

bool viewport_vsync();
int viewport_multisamples();
bool viewport_fullscreen();

int viewport_height();
int viewport_width();
vector2 viewport_dimensions();
float viewport_ratio();

void viewport_screenshot();

#endif
