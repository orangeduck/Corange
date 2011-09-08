#ifndef image_h
#define image_h

#include "vector.h"

typedef struct {
  int width;
  int height;
  unsigned char* data;
  
  int repeat_type;
  int sample_type;
} image;


image* image_new(int width, int height, char* data);
image* image_blank(int width, int height);

void image_delete(image* i);

vector4 image_get_pixel(image* i, int u, int v);
void image_set_pixel(image* i, int u, int v, vector4 color);

vector4 image_sample(image* i, vector2 uv);

void image_rotate_90_clockwise(image* i);
void image_rotate_90_counterclockwise(image* i);
void image_rotate_180(image* i);

void image_flip_horizontal(image* i);
void image_flip_vertical(image* i);

image* bmp_load_file(char* filename);
image* png_load_file(char* filename);
image* tif_load_file(char* filename);
image* jpg_load_file(char* filename);

void tga_save_file(image* i, char* filename);

void image_write_to_file(image* i, char* filename);

static int image_repeat_tile = 0;
static int image_repeat_clamp = 1;

static int image_sample_linear = 0;
static int image_sample_nearest = 1;

#endif