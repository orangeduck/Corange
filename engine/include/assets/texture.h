/**
*** :: Texture ::
***
***   OpenGL stored texture
***
***   Supports the loading of LUT files
***   as 3d textures for color correction
***
**/

#ifndef texture_h
#define texture_h

#include "cengine.h"
#include "assets/image.h"

typedef GLuint texture;

texture* texture_new();
void texture_delete(texture* t);

GLuint texture_handle(texture* t);

void texture_set_image(texture* t, image* i);
image* texture_get_image(texture* t);

void texture_generate_mipmaps(texture* t);
void texture_set_filtering_nearest(texture* t);
void texture_set_filtering_linear(texture* t);
void texture_set_filtering_anisotropic(texture* t);

texture* dds_load_file( char* filename );
texture* lut_load_file( char* filename );

void texture_write_to_file(texture* t, char* filename);
void texture3d_write_to_file(texture* t, char* filename);

#endif