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

typedef struct {
  GLuint handle;
  GLenum type;
} texture;

texture* texture_new();
texture* texture_new_handle(GLuint h);
void texture_delete(texture* t);

GLuint texture_handle(texture* t);
GLenum texture_type(texture* t);

void texture_set_image(texture* t, image* i);
image* texture_get_image(texture* t);

void texture_generate_mipmaps(texture* t);
void texture_set_filtering_nearest(texture* t);
void texture_set_filtering_linear(texture* t);
void texture_set_filtering_anisotropic(texture* t);

texture* bmp_load_file( char* filename );
texture* tga_load_file( char* filename );
texture* dds_load_file( char* filename );
texture* lut_load_file( char* filename );
texture* acv_load_file( char* filename );

void texture_write_to_file(texture* t, char* filename);
void texture3d_write_to_file(texture* t, char* filename);

#endif
