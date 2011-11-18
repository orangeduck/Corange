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

const static int image_repeat_tile = 0;
const static int image_repeat_clamp = 1;
const static int image_repeat_mirror = 2;

const static int image_sample_linear = 0;
const static int image_sample_nearest = 1;

image* image_new(int width, int height, unsigned char* data);
image* image_empty(int width, int height);
image* image_blank(int width, int height);

image* image_duplicate(image* src);
image* image_subimage(image* src, int left, int top, int width, int height);
image* image_subsample(image* src, vector2 top_left, vector2 bottom_right);

image* image_red_channel(image* src);
image* image_green_channel(image* src);
image* image_blue_channel(image* src);
image* image_alpha_channel(image* src);

image* image_flood_fill_mask(image* src, int base_u, int base_v, float tolerance);
image* image_intensity_mask(image* src, float boundry);
image* image_difference_mask(image* src, vector4 color, float tolerance);

void image_delete(image* i);

vector4 image_get_pixel(image* i, int u, int v);
void image_set_pixel(image* i, int u, int v, vector4 color);

vector4 image_sample(image* i, vector2 uv);
void image_paint(image* i, vector2 uv, vector4 color);

void image_bgr_to_rgb(image* i);

void image_rotate_90_clockwise(image* i);
void image_rotate_90_counterclockwise(image* i);
void image_rotate_180(image* i);

void image_flip_horizontal(image* i);
void image_flip_vertical(image* i);

void image_scale(image* i, vector2 scale);

void image_fill(image* i, vector4 color);
void image_fill_black(image* i);
void image_fill_white(image* i);

void image_copy(image* dst, image* src);
void image_copy_sub(image* dst, image* src, vector2 top_left);
void image_paste_sub(image* dst, image* src, vector2 top_left);

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

image* tga_load_file(char* filename);
image* bmp_load_file(char* filename);

void tga_save_file(image* i, char* filename);

void image_write_to_file(image* i, char* filename);

#endif