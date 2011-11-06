#ifndef texture_h
#define texture_h

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_local.h"

#include "image.h"

typedef GLuint texture;

texture* texture_new();
void texture_delete(texture* t);

void texture_set_image(texture* t, image* i);
image* texture_get_image(texture* t);

void texture_generate_mipmaps(texture* t);
void texture_set_filtering_nearest(texture* t);

texture* dds_load_file( char* filename );
texture* lut_load_file( char* filename );

void texture_write_to_file(texture* t, char* filename);
void texture3d_write_to_file(texture* t, char* filename);

#endif