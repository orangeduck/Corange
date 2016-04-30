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

enum {
  IMAGE_REPEAT_TILE   = 0,
  IMAGE_REPEAT_CLAMP  = 1,
  IMAGE_REPEAT_MIRROR = 2,
  IMAGE_REPEAT_ERROR  = 3,
  IMAGE_REPEAT_BLACK  = 4
};

enum {
  IMAGE_SAMPLE_LINEAR  = 0,
  IMAGE_SAMPLE_NEAREST = 1
};

image* image_new(int width, int height, unsigned char* data);
image* image_empty(int width, int height);
image* image_blank(int width, int height);
image* image_copy(image* src);
void image_delete(image* i);

vec4 image_get(image* i, int u, int v);
void image_set(image* i, int u, int v, vec4 c);
void image_map(image* i, vec4(*f)(vec4));

image* image_red_channel(image* src);
image* image_green_channel(image* src);
image* image_blue_channel(image* src);
image* image_alpha_channel(image* src);

void image_bgr_to_rgb(image* i);
void image_rotate_90_clockwise(image* i);
void image_rotate_90_counterclockwise(image* i);
void image_rotate_180(image* i);
void image_rotate_inplace(image* i, float amount);
void image_flip_horizontal(image* i);
void image_flip_vertical(image* i);

void image_fill(image* i, vec4 color);
void image_fill_black(image* i);
void image_fill_white(image* i);

void image_apply_gamma(image* i, float amount);
void image_to_gamma(image* i);
void image_from_gamma(image* i);

void image_rgb_to_hsv(image* i);
void image_hsv_to_rgb(image* i);
void image_hsv_scalar(image* i);

vec4 image_min(image* i);
vec4 image_max(image* i);
vec4 image_mean(image* i);
vec4 image_var(image* i);
vec4 image_std(image* i);
void image_auto_contrast(image* i);
void image_set_to_mask(image* i);
void image_set_brightness(image* i, float brightness);

vec4 image_alpha_mean(image* i);

image* image_get_subimage(image* src, int left, int top, int width, int height);

void image_set_subimage(image* i, int left, int top, image* src);
void image_paste_subimage(image* i, int left, int top, image* src);

vec4 image_sample(image* i, vec2 uv);
void image_paint(image* i, vec2 uv, vec4 color);

void image_scale(image* i, vec2 scale);

void image_mask_not(image* i);
void image_mask_binary(image* i0, image* i1, bool(*f)(bool,bool));
void image_mask_or(image* i0, image* i1);
void image_mask_and(image* i0, image* i1);
void image_mask_xor(image* i0, image* i1);
void image_mask_nor(image* i0, image* i1);
void image_mask_nand(image* i0, image* i1);
void image_mask_xnor(image* i0, image* i1);

image* image_mask_alpha(image* i);
image* image_mask_nearest(image* i);
image* image_mask_nearest(image* i);
image* image_mask_flood_fill(image* src, int u, int v, float tolerance);
image* image_mask_difference(image* src, vec4 color, float tolerance);

long image_mask_count(image* i);
void image_mask_median(image* i, int* u, int* v);
void image_mask_random(image* i, int* u, int* v);

image* image_read_from_file(char* filename);
image* image_tga_load_file(char* filename);
image* image_bmp_load_file(char* filename);

void image_write_to_file(image* i, char* filename);
void image_tga_save_file(image* i, char* filename);
void image_bmp_save_file(image* i, char* filename);

#endif