/**
*** :: Image ::
***
***   Functions to use in CPU side image processing.
***
**/

#ifndef image_h
#define image_h

#include "cengine.h"

typedef struct {
  int width;
  int height;
  unsigned char* data;
  int repeat_type;
  int sample_type;
} image;

static const int image_repeat_tile = 0;
static const int image_repeat_clamp = 1;
static const int image_repeat_mirror = 2;

static const  int image_sample_linear = 0;
static const int image_sample_nearest = 1;

image* image_new(int width, int height, unsigned char* data);
image* image_empty(int width, int height);
image* image_blank(int width, int height);

image* image_duplicate(image* src);
image* image_subimage(image* src, int left, int top, int width, int height);
image* image_subsample(image* src, vec2 top_left, vec2 bottom_right);

image* image_red_channel(image* src);
image* image_green_channel(image* src);
image* image_blue_channel(image* src);
image* image_alpha_channel(image* src);

image* image_flood_fill_mask(image* src, int base_u, int base_v, float tolerance);
image* image_intensity_mask(image* src, float boundry);
image* image_difference_mask(image* src, vec4 color, float tolerance);

void image_delete(image* i);

vec4 image_get_pixel(image* i, int u, int v);
void image_set_pixel(image* i, int u, int v, vec4 color);

vec4 image_sample(image* i, vec2 uv);
void image_paint(image* i, vec2 uv, vec4 color);

void image_bgr_to_rgb(image* i);

void image_rotate_90_clockwise(image* i);
void image_rotate_90_counterclockwise(image* i);
void image_rotate_180(image* i);

void image_flip_horizontal(image* i);
void image_flip_vertical(image* i);

void image_scale(image* i, vec2 scale);

void image_fill(image* i, vec4 color);
void image_fill_black(image* i);
void image_fill_white(image* i);

void image_copy(image* dst, image* src);
void image_copy_sub(image* dst, image* src, vec2 top_left);
void image_paste_sub(image* dst, image* src, vec2 top_left);

float image_intensity(image* i);

int image_mask_area_width(image* i);
int image_mask_area_height(image* i);

void image_mask_not(image* i);
void image_mask_or(image* i, image* i2);
void image_mask_and(image* i, image* i2);
void image_mask_xor(image* i, image* i2);
void image_mask_nor(image* i, image* i2);
void image_mask_nand(image* i, image* i2);
void image_mask_xnor(image* i, image* i2);

image* image_read_from_file(char* filename);
image* image_tga_load_file(char* filename);
image* image_bmp_load_file(char* filename);

void image_write_to_file(image* i, char* filename);
void image_tga_save_file(image* i, char* filename);
void image_bmp_save_file(image* i, char* filename);

#endif