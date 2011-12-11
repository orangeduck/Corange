#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "asset_manager.h"

#include "error.h"
#include "int_list.h"

#include "SDL/SDL.h"

#include "image.h"


image* image_new(int width, int height, unsigned char* data) {
  
  image* i = malloc(sizeof(image));
  i->data = malloc(width * height * 4);
  memcpy(i->data, data, width * height * 4);
  i->width = width;
  i->height = height;
  
  i->repeat_type = image_repeat_tile;
  i->sample_type = image_sample_linear;
  
  return i;
}

image* image_empty(int width, int height) {

  image* i = malloc(sizeof(image));
  i->data = malloc(width * height * 4);
  i->width = width;
  i->height = height;
  
  i->repeat_type = image_repeat_tile;
  i->sample_type = image_sample_linear;
  
  return i;
}


image* image_blank(int width, int height) {
  
  image* i = malloc(sizeof(image));
  i->data = calloc(width * height * 4, 1);
  i->width = width;
  i->height = height;
  
  i->repeat_type = image_repeat_tile;
  i->sample_type = image_sample_linear;
  
  return i;
}

image* image_duplicate(image* src) {

  image* i = malloc(sizeof(image));
  i->data = malloc(src->width * src->height * 4);
  memcpy(i->data, src->data, src->width * src->height * 4);
  i->width = src->width;
  i->height = src->height;
  
  i->repeat_type = src->repeat_type;
  i->sample_type = src->sample_type;

  return i;
  
}

image* image_red_channel(image* src) {
  
  image* new = image_empty(src->width, src->height);
  
  int x, y;
  
  for( x = 0; x < src->width; x++)
  for( y = 0; y < src->height; y++) {
    float c = image_get_pixel( src, x, y ).r;
    image_set_pixel( new, x, y, v4(c,c,c,c) );
  }
  
  return new;
  
}

image* image_green_channel(image* src) {

  image* new = image_empty(src->width, src->height);
  
  int x, y;
  
  for( x = 0; x < src->width; x++)
  for( y = 0; y < src->height; y++) {
    float c = image_get_pixel( src, x, y ).g;
    image_set_pixel( new, x, y, v4(c,c,c,c) );
  }
  
  return new;

}

image* image_blue_channel(image* src) {

  image* new = image_empty(src->width, src->height);
  
  int x, y;
  
  for( x = 0; x < src->width; x++)
  for( y = 0; y < src->height; y++) {
    float c = image_get_pixel( src, x, y ).b;
    image_set_pixel( new, x, y, v4(c,c,c,c) );
  }
  
  return new;

}

image* image_alpha_channel(image* src) {

  image* new = image_empty(src->width, src->height);
  
  int x, y;
  
  for( x = 0; x < src->width; x++)
  for( y = 0; y < src->height; y++) {
    float c = image_get_pixel( src, x, y ).a;
    image_set_pixel( new, x, y, v4(c,c,c,c) );
  }
  
  return new;

}

image* image_subimage(image* src, int left, int top, int width, int height) {
  
  assert(left >= 0);
  assert(top >= 0);
  assert(left + width < src->width);
  assert(top + height < src->height);
  
  image* i = malloc(sizeof(image));
  i->width = width;
  i->height = height;
  i->data = malloc(i->width * i->height * 4);
  
  i->repeat_type = src->repeat_type;
  i->sample_type = src->sample_type;
  
  int x,y;
  for( x=0; x < i->width; x++)
  for( y=0; y < i->height; y++){
    vector4 pix = image_get_pixel( src, left+x, top+y );
    image_set_pixel( i, x, y, pix );
  }
  
  return i;
}

image* image_subsample(image* src, vector2 top_left, vector2 bottom_right) {

  image* i = malloc(sizeof(image));
  
  float s_width = ( bottom_right.x - top_left.x );
  float s_height = ( bottom_right.y - top_left.y );
  
  int width = floor(src->width * s_width);
  int height = floor(src->height * s_height);
  
  i->width = width;
  i->height = height;
  i->data = malloc(width * height * 4);
  
  int x,y;
  for( x = 0; x < width; x++)
  for( y = 0; y < height; y++) {
    
    float u = ((float)x / src->width) + top_left.x;
    float v = ((float)y / src->height) + top_left.y;
    
    vector4 pix = image_sample( src, v2(u,v) );
    image_set_pixel(i, x, y, pix);
    
  }
  
  return i;  
  
}

image* image_flood_fill_mask(image* src, int base_u, int base_v, float tolerance) {
  
  image* mask = image_blank(src->width, src->height);
  
  int_list* q_x = int_list_new_blocksize(256);
  int_list* q_y = int_list_new_blocksize(256);
  
  vector4 base = image_get_pixel(src, base_u, base_v);
  float base_val = (base.r + base.g + base.b + base.a) / 4;
  
  int_list_push_back(q_x, base_u);
  int_list_push_back(q_y, base_v);
  
  while ( !int_list_is_empty(q_x) ) {
    
    int u = int_list_pop_back(q_x);
    int v = int_list_pop_back(q_y);
    
    image_set_pixel(mask, u, v, v4_one() );
    
    if (u > 0) {
      vector4 left = image_get_pixel(src, u-1, v);
      vector4 left_mask = image_get_pixel(mask, u-1, v);
      float left_val = (left.r + left.g + left.b + left.a) / 4;
      
      if ( ( fabs( base_val - left_val ) <= tolerance ) && (left_mask.r != 1.0) ) {
        int_list_push_back(q_x, u-1);
        int_list_push_back(q_y, v);
      }
    }
    
    if (u < src->width-1) {
      vector4 right = image_get_pixel(src, u+1, v);
      vector4 right_mask = image_get_pixel(mask, u+1, v);
      float right_val = (right.r + right.g + right.b + right.a) / 4;
      
      if ( ( fabs( base_val - right_val ) <= tolerance ) && (right_mask.r != 1.0) ) {
        int_list_push_back(q_x, u+1);
        int_list_push_back(q_y, v);
      }
    }
    
    if (v > 0) {
      vector4 top = image_get_pixel(src, u, v-1);
      vector4 top_mask = image_get_pixel(mask, u, v-1);
      float top_val = (top.r + top.g + top.b + top.a) / 4;
      
      if ( ( fabs( base_val - top_val ) <= tolerance ) && (top_mask.r != 1.0) ) {
        int_list_push_back(q_x, u);
        int_list_push_back(q_y, v-1);
      }
    }
    
    if (v < src->height-1) {
      vector4 bottom = image_get_pixel(src, u, v+1);
      vector4 bottom_mask = image_get_pixel(mask, u, v+1);
      float bottom_val = (bottom.r + bottom.g + bottom.b + bottom.a) / 4;
      
      if ( ( fabs( base_val - bottom_val ) <= tolerance ) && (bottom_mask.r != 1.0) ) {
        int_list_push_back(q_x, u);
        int_list_push_back(q_y, v+1);
      }
    }
    
  }
  
  int_list_delete(q_x);
  int_list_delete(q_y);
  
  return mask;
  
}

image* image_intensity_mask(image* src, float boundry) {

  image* mask = image_blank(src->width, src->height);
  
  int x,y;
  for (x = 0; x < src->width; x++)
  for (y = 0; y < src->height; y++) {
    
    vector4 col = image_get_pixel(src, x, y);
    float val = (col.r + col.g + col.b + col.a) / 4;
    
    if (val > boundry) {
      image_set_pixel( mask, x, y, v4_one() );
    }
    
  }
  
  return mask;

}

image* image_difference_mask(image* src, vector4 color, float tolerance){

  image* mask = image_blank(src->width, src->height);
  
  float base_val = (color.r + color.g + color.b + color.a) / 4;
  
  int x,y;
  for (x = 0; x < src->width; x++)
  for (y = 0; y < src->height; y++) {
  
    vector4 col = image_get_pixel(src, x, y);
    float val = (col.r + col.g + col.b + col.a) / 4;
    
    if ( fabs(val - base_val) > tolerance) {
      image_set_pixel( mask, x, y, v4_one() );
    }
  
  }
  
  return mask;

}

void image_delete(image* i) {
  free(i->data);
  free(i);
}

vector4 image_get_pixel(image* i, int u, int v) {
  
  v = i->height - v - 1;
  
  assert( u >= 0 );
  assert( v >= 0 );
  assert( u < i->width );
  assert( v < i->height );
  
  float r = (float)i->data[u * 4 + v * i->width * 4 + 0] / 255;
  float g = (float)i->data[u * 4 + v * i->width * 4 + 1] / 255;
  float b = (float)i->data[u * 4 + v * i->width * 4 + 2] / 255;
  float a = (float)i->data[u * 4 + v * i->width * 4 + 3] / 255;

  return v4(r,g,b,a);
  
}

void image_set_pixel(image* i, int u, int v, vector4 color) {
  
  v = i->height - v - 1;
  i->data[u * 4 + v * i->width * 4 + 0] = (color.r * 255);
  i->data[u * 4 + v * i->width * 4 + 1] = (color.g * 255);
  i->data[u * 4 + v * i->width * 4 + 2] = (color.b * 255);
  i->data[u * 4 + v * i->width * 4 + 3] = (color.a * 255);
  
}

vector4 image_sample(image* i, vector2 uv) {

  if ( i->repeat_type == image_repeat_tile ) {
    uv = v2_fmod(uv, 1.0);
    if (uv.x < 0) { uv.x = 1 + uv.x; }
    if (uv.y < 0) { uv.y = 1 + uv.y; }
  } else if ( i->repeat_type == image_repeat_clamp ) {
    uv = v2_saturate(uv);
  } else if ( i->repeat_type == image_repeat_mirror) {
    
    if ( ((int)floor(uv.x) % 2) == 1) {
      uv.x = fmod(uv.x, 1);
      uv.x = 1 - uv.x;
    } else {
      uv.x = fmod(uv.x, 1);
    }
    
    if ( ((int)floor(uv.y) % 2) == 1) {
      uv.y = fmod(uv.y, 1);
      uv.y = 1 - uv.y;
    } else {
      uv.y = fmod(uv.y, 1);
    }
    
  }

  float u = i->width * uv.x;
  float v = i->height * uv.y;
  
  int s1_u = floor(u);
  int s1_v = floor(v);
  
  int s2_u = ceil(u);
  int s2_v = floor(v);
  
  int s3_u = floor(u);
  int s3_v = ceil(v);
  
  int s4_u = ceil(u);
  int s4_v = ceil(v);
  
  s1_u = (s1_u == i->width)  ? 0 : s1_u;
  s1_v = (s1_v == i->height) ? 0 : s1_v;
  s2_u = (s2_u == i->width)  ? 0 : s2_u;
  s2_v = (s2_v == i->height) ? 0 : s2_v;
  s3_u = (s3_u == i->width)  ? 0 : s3_u;
  s3_v = (s3_v == i->height) ? 0 : s3_v;
  s4_u = (s4_u == i->width)  ? 0 : s4_u;
  s4_v = (s4_v == i->height) ? 0 : s4_v;
  
  float amount_x = fmod(u, 1.0);
  float amount_y = fmod(v, 1.0);
  
  vector4 s1, s2, s3, s4;
  
  s1 = image_get_pixel(i, s1_u, s1_v);
  s2 = image_get_pixel(i, s2_u, s2_v);
  s3 = image_get_pixel(i, s3_u, s3_v);
  s4 = image_get_pixel(i, s4_u, s4_v);
  
  if ( i->sample_type == image_sample_linear ) {
    return v4_bilinear_interpolation(s1, s2, s3, s4, amount_x, amount_y);
  } else if ( i->sample_type == image_sample_nearest ) {
    return v4_binearest_neighbor_interpolation(s1, s2, s3, s4, amount_x, amount_y);
  } else {
    error("Unknown Sampling type %i\n", i->sample_type);
  }
  
  return v4_zero();
}

void image_paint(image* i, vector2 uv, vector4 color) {
  
  if ( i->repeat_type == image_repeat_tile ) {
    uv = v2_fmod(uv, 1.0);
    if (uv.x < 0) { uv.x = 1 - uv.x; }
    if (uv.y < 0) { uv.y = 1 - uv.y; }
  } else if ( i->repeat_type == image_repeat_clamp ) {
    uv = v2_saturate(uv);
  } else if ( i->repeat_type == image_repeat_mirror) {
    
    if ( ((int)floor(uv.x) % 2) == 1) {
      uv.x = fmod(uv.x, 1);
      uv.x = 1 - uv.x;
    } else {
      uv.x = fmod(uv.x, 1);
    }
    
    if ( ((int)floor(uv.y) % 2) == 1) {
      uv.y = fmod(uv.y, 1);
      uv.y = 1 - uv.y;
    } else {
      uv.y = fmod(uv.y, 1);
    }
    
  }
  
  float u = i->width * uv.x;
  float v = i->height * uv.y;
  
  int s1_u = floor(u);
  int s1_v = floor(v);
  
  int s2_u = ceil(u);
  int s2_v = floor(v);
  
  int s3_u = floor(u);
  int s3_v = ceil(v);
  
  int s4_u = ceil(u);
  int s4_v = ceil(v);
  
  s1_u = (s1_u == i->width)  ? 0 : s1_u;
  s1_v = (s1_v == i->height) ? 0 : s1_v;
  s2_u = (s2_u == i->width)  ? 0 : s2_u;
  s2_v = (s2_v == i->height) ? 0 : s2_v;
  s3_u = (s3_u == i->width)  ? 0 : s3_u;
  s3_v = (s3_v == i->height) ? 0 : s3_v;
  s4_u = (s4_u == i->width)  ? 0 : s4_u;
  s4_v = (s4_v == i->height) ? 0 : s4_v;
  
  float amount_x = fmod(u, 1.0);
  float amount_y = fmod(v, 1.0);
  
  vector4 s1, s2, s3, s4;
  
  s1 = image_get_pixel(i, s1_u, s1_v);
  s2 = image_get_pixel(i, s2_u, s2_v);
  s3 = image_get_pixel(i, s3_u, s3_v);
  s4 = image_get_pixel(i, s4_u, s4_v);
  
  if ( i->sample_type == image_sample_linear ) {
  
    s1 = v4_lerp(s1, color, (1-amount_x + 1-amount_y)/2);
    s2 = v4_lerp(s2, color, (amount_x + 1-amount_y)/2);
    s3 = v4_lerp(s3, color, (amount_x + amount_y)/2);
    s3 = v4_lerp(s4, color, (1-amount_x + amount_y)/2);
    
    image_set_pixel(i, s1_u, s1_v, s1);
    image_set_pixel(i, s2_u, s2_v, s2);
    image_set_pixel(i, s3_u, s3_v, s3);
    image_set_pixel(i, s4_u, s4_v, s4);
  
  } else if ( i->sample_type == image_sample_nearest ) {

    s1 = v4_nearest_neighbor_interpolation(s1, color, (1-amount_x + 1-amount_y)/2);
    s2 = v4_nearest_neighbor_interpolation(s2, color, (amount_x + 1-amount_y)/2);
    s3 = v4_nearest_neighbor_interpolation(s3, color, (amount_x + amount_y)/2);
    s3 = v4_nearest_neighbor_interpolation(s4, color, (1-amount_x + amount_y)/2);
    
    image_set_pixel(i, s1_u, s1_v, s1);
    image_set_pixel(i, s2_u, s2_v, s2);
    image_set_pixel(i, s3_u, s3_v, s3);
    image_set_pixel(i, s4_u, s4_v, s4);  
  
  }
  
}

static void swap(void **x, void **y) {
	void *t = *x;
	*x = *y;
	*y = t;
}

void image_rotate_90_clockwise(image* i) {
  
  /* Height and width swapped on purpose! */
  image* new = image_blank(i->height, i->width);
  
  int x, y;
  for( x = 0; x < new->width; x++)
  for( y = 0; y < new->height; y++) {
    vector4 p = image_get_pixel(i, (new->height-1) - y, x);
    image_set_pixel( new, x, y, p );
  }
  
  i->width = new->width;
  i->height = new->height;
  swap((void**)&i->data, (void**)&new->data);
  
  image_delete(new);
  
}

void image_bgr_to_rgb(image* i) {
  
  int x, y;
  for (x = 0; x < i->width; x++)
  for (y = 0; y < i->height; y++) {
    vector4 pix = image_get_pixel(i, x, y);
    vector4 rev = v4(pix.b, pix.g, pix.r, pix.a);
    
    image_set_pixel(i, x, y, rev);
  }
  
}

void image_rotate_90_counterclockwise(image* i) {

  /* Height and width swapped on purpose! */
  image* new = image_blank(i->height, i->width);
  
  int x, y;
  for( x = 0; x < new->width; x++)
  for( y = 0; y < new->height; y++) {
    vector4 p = image_get_pixel(i, y, (new->width-1) - x);
    image_set_pixel( new, x, y, p );
  }
  
  i->width = new->width;
  i->height = new->height;
  swap((void**)&i->data, (void**)&new->data);
  
  image_delete(new);

}

void image_rotate_180(image* i) {

  image_flip_vertical(i);
  image_flip_horizontal(i);

}

void image_flip_horizontal(image* i) {

  int x, y;
  for (y = 0; y < i->height; y++)
  for (x = 0; x < i->width / 2; x++) {
     
     vector4 left = image_get_pixel(i, x, y);
     vector4 right = image_get_pixel(i, (i->width-1) - x, y);
     
     image_set_pixel(i, x, y, right);
     image_set_pixel(i, (i->width-1) - x, y, left);
  }

}

void image_flip_vertical(image* i) {

  int x, y;
  for (x = 0; x < i->width; x++)
  for (y = 0; y < i->height / 2; y++) {
     
     vector4 top = image_get_pixel(i, x, y);
     vector4 bottom = image_get_pixel(i, x, (i->height-1) - y);
     
     image_set_pixel(i, x, y, bottom);
     image_set_pixel(i, x, (i->height-1) - y, top);
  }

}

void image_scale(image* i, vector2 scale) {
  
  image* new = image_empty( i->width * scale.x , i->height * scale.y );
  
  int x, y;
  for (x = 0; x < new->width; x++)
  for (y = 0; y < new->height; y++) {
     vector4 sample = image_sample( i, v2((float)x / new->width, (float)y / new->height) );
     image_set_pixel(new, x, y, sample);
  }
  
  i->width = new->width;
  i->height = new->height;
  swap((void**)&i->data, (void**)&new->data);
  
  image_delete(new);
}

void image_fill(image* i, vector4 color) {
  int x, y;
  for (x = 0; x < i->width; x++)
  for (y = 0; y < i->height; y++) {
    image_set_pixel(i, x, y, color);
  }
}

void image_fill_black(image* i) {
  memset(i->data, 0, i->width * i->height * 4);
}

void image_fill_white(image* i) {
  memset(i->data, 1, i->width * i->height * 4);
}

void image_copy(image* dst, image* src) {
  
  assert( dst->width == src->width );
  assert( dst->height == src->height );
  
  memcpy( dst->data, src->data, dst->width * dst->height * 4 );
}

void image_copy_sub(image* dst, image* src, vector2 top_left) {
  
  int x, y;
  for (x = 0; x < dst->width; x++)
  for (y = 0; y < dst->height; y++) {
    
    vector2 uv = v2_add( v2_new( (float)x / src->width, (float)y / src->height ), top_left);
    vector4 pix = image_sample(src, uv);
    
    image_set_pixel(dst, x, y, pix);
  
  }  
}

void image_paste_sub(image* dst, image* src, vector2 top_left) {
  
  int x, y;
  for (x = 0; x < src->width; x++)
  for (y = 0; y < src->height; y++) {
    
    vector2 uv = v2_add( v2_new( (float)x / dst->width, (float)y / dst->height ), top_left);
    vector4 col = image_get_pixel(src, x, y);
    
    image_paint(src, uv, col);
  
  }  
  
}


float image_intensity(image* i) {
  
  float total = 0;
  int x, y;
  for (x = 0; x < i->width; x++)
  for (y = 0; y < i->height; y++) {
    
    vector4 col = image_get_pixel(i, x, y);
    total += (col.r + col.g + col.b + col.a) / 4;
  
  }
  
  return total;
}

int image_mask_area_width(image* i) {
  
  int x_min, x_max;
  x_min = i->width; x_max = 0;
  
  int x,y;
  for (x = 0; x < i->width; x++)
  for (y = 0; y < i->height; y++) {
    
    vector4 col = image_get_pixel(i, x, y);
    if (col.a == 1.0) {
      x_min = min(x, x_min);
      x_max = max(x, x_max);
    }
  
  }
  
  return (x_max - x_min);
  
}

int image_mask_area_height(image* i) {

  int y_min, y_max;
  y_min = i->height; y_max = 0;
  
  int x,y;
  for (x = 0; x < i->width; x++)
  for (y = 0; y < i->height; y++) {
    
    vector4 col = image_get_pixel(i, x, y);
    if (col.a == 1) {
      y_min = min(y, y_min);
      y_max = max(y, y_max);
    }
  
  }
  
  return (y_max - y_min);

}

void image_mask_not(image* i) {
  int x, y;
  for( x = 0; x < i->width; x++) 
  for( y = 0; y < i->height; y++) {
    vector4 p = image_get_pixel(i, x, y);
    if ( p.a == 1.0 ) {
      image_set_pixel(i, x, y, v4_zero() );
    } else {
      image_set_pixel(i, x, y, v4_one() );
    }
  }
}

void image_mask_or(image* i, image* i2) {
  
  assert(i2->width >= i->width);
  assert(i2->height >= i->height);
  
  int x, y;
  for( x = 0; x < i->width; x++) 
  for( y = 0; y < i->height; y++) {
    
    vector4 p1 = image_get_pixel(i, x, y);
    vector4 p2 = image_get_pixel(i2, x, y);
    
    if( (p1.a == 1.0) || (p2.a == 1.0) ) {
      image_set_pixel( i, x, y, v4_one() );
    } else {
      image_set_pixel( i, x, y, v4_zero() );
    }
  
  }
  
}

void image_mask_and(image* i, image* i2) {

  assert(i2->width >= i->width);
  assert(i2->height >= i->height);
  
  int x, y;
  for( x = 0; x < i->width; x++) 
  for( y = 0; y < i->height; y++) {
    
    vector4 p1 = image_get_pixel(i, x, y);
    vector4 p2 = image_get_pixel(i2, x, y);
  
    if( (p1.a == 1.0) && (p2.a == 1.0) ) {
      image_set_pixel( i, x, y, v4_one() );
    } else {
      image_set_pixel( i, x, y, v4_zero() );
    }
  
  }

}

static int xor( float a, float b ) {
    
  if ( ((a == 1.0) || (b == 1.0)) && !((a == 1.0) && (b == 1.0)) ) {
    return 1;
  } else {
    return 0;
  }
}

void image_mask_xor(image* i, image* i2) {

  assert(i2->width >= i->width);
  assert(i2->height >= i->height);
  
  int x, y;
  for( x = 0; x < i->width; x++) 
  for( y = 0; y < i->height; y++) {
    
    vector4 p1 = image_get_pixel(i, x, y);
    vector4 p2 = image_get_pixel(i2, x, y);
  
    if( xor( p1.a, p2.a ) ) {
      image_set_pixel( i, x, y, v4_one() );
    } else {
      image_set_pixel( i, x, y, v4_zero() );
    }
  
  }

}

void image_mask_nor(image* i, image* i2) {

  assert(i2->width >= i->width);
  assert(i2->height >= i->height);
  
  int x, y;
  for( x = 0; x < i->width; x++) 
  for( y = 0; y < i->height; y++) {
    
    vector4 p1 = image_get_pixel(i, x, y);
    vector4 p2 = image_get_pixel(i2, x, y);
  
    if( (p1.a == 1.0) || (p2.a == 1.0) ) {
      image_set_pixel( i, x, y, v4_zero() );
    } else {
      image_set_pixel( i, x, y, v4_one() );
    }
  
  }

}

void image_mask_nand(image* i, image* i2) {

  assert(i2->width >= i->width);
  assert(i2->height >= i->height);
  
  int x, y;
  for( x = 0; x < i->width; x++) 
  for( y = 0; y < i->height; y++) {
    
    vector4 p1 = image_get_pixel(i, x, y);
    vector4 p2 = image_get_pixel(i2, x, y);
  
    if( (p1.a == 1.0) && (p2.a == 1.0) ) {
      image_set_pixel( i, x, y, v4_zero() );
    } else {
      image_set_pixel( i, x, y, v4_one() );
    }
  
  }

}

void image_mask_xnor(image* i, image* i2) {

  assert(i2->width >= i->width);
  assert(i2->height >= i->height);
  
  int x, y;
  for( x = 0; x < i->width; x++) 
  for( y = 0; y < i->height; y++) {
    
    vector4 p1 = image_get_pixel(i, x, y);
    vector4 p2 = image_get_pixel(i2, x, y);
  
    if( xor( p1.a, p2.a ) ) {
      image_set_pixel( i, x, y, v4_zero() );
    } else {
      image_set_pixel( i, x, y, v4_one() );
    }
  
  }

}

void tga_save_file(image* i, char* filename) {
  
  image_bgr_to_rgb(i);
  
  int xa= i->width % 256;
  int xb= (i->width-xa)/256;

  int ya= i->height % 256;
  int yb= (i->height-ya)/256;
  unsigned char header[18]={0,0,2,0,0,0,0,0,0,0,0,0,(char)xa,(char)xb,(char)ya,(char)yb,32,0};
  
  SDL_RWops* file = SDL_RWFromFile(filename, "wb");
  SDL_RWwrite(file, header, sizeof(header), 1);
  SDL_RWwrite(file, i->data, i->width * i->height * 4, 1 );
  SDL_RWclose(file);

  image_bgr_to_rgb(i);
  
}

void image_write_to_file(image* i, char* filename) {
  
  char* ext = asset_file_extension(filename);
  
  if ( strcmp(ext, "tga") == 0 ) {
    tga_save_file(i, filename);
  } else {
    warning("Cannot save texture to %s, unknown file extension %s. Try .tga!\n", filename, ext);
  }
  
  free(ext);
}

image* tga_load_file(char* filename) {

  SDL_RWops* file = SDL_RWFromFile(filename, "rb");
	if (file == NULL) {
		error("Cannot open file %s", filename);
	}
	
  uint16_t width, height, depth;
	
	/* Seek to the width */
	SDL_RWseek(file, 12, SEEK_SET);
	SDL_RWread(file, &width, sizeof(uint16_t), 1);
	
	/* Seek to the height */
	SDL_RWseek(file, 14, SEEK_SET);
	SDL_RWread(file, &height, sizeof(uint16_t), 1);
	
	/* Seek to the depth */
	SDL_RWseek(file, 16, SEEK_SET);
	SDL_RWread(file, &depth, sizeof(uint16_t), 1);
  
  image* i = image_empty(width, height);
  
  int channels;
	if(depth == 24) {
		channels = 3;
	} else {
		channels = 4;
	}

	int size = height * width * channels;
	unsigned char* image_data = malloc(sizeof(unsigned char) * size);

	/* Seek to the image data. */
	SDL_RWseek(file, 18, SEEK_SET);
	SDL_RWread(file, image_data, sizeof(unsigned char), size);

  SDL_RWclose(file);

  if(channels == 4) {
    
    //memcpy(i->data, image_data, size);
    
  } else if (channels == 3) {
    
    int x, y;
    for( x = 0; x < i->width; x++)
    for( y = 0; y < i->width; y++) {
      i->data[x * 4 + y * i->width * 4 + 0] = image_data[x * 3 + y * width * 3 + 0];
      i->data[x * 4 + y * i->width * 4 + 1] = image_data[x * 3 + y * width * 3 + 1];
      i->data[x * 4 + y * i->width * 4 + 2] = image_data[x * 3 + y * width * 3 + 2];
      i->data[x * 4 + y * i->width * 4 + 3] = 0;
    }
    
  }
    
  free(image_data);
  
  image_bgr_to_rgb(i);
  
  return i;
  
}

image* bmp_load_file(char* filename) {
  
  SDL_Surface *surface;
   
  surface = SDL_LoadBMP(filename);
   
  if (!surface) {
    error("Could not load file %s\n", filename);
  }
  
  unsigned char* image_data = malloc(sizeof(unsigned char) * 4 * surface->w * surface->h);
  
  if (surface->format->BytesPerPixel == 3) {
    
    int x, y;
    for(x = 0; x < surface->w; x++)
    for(y = 0; y < surface->h; y++) {
      image_data[x * 4 + y * surface->w * 4 + 0] = ((unsigned char*)surface->pixels)[x * 3 + y * surface->w * 3 + 0];
      image_data[x * 4 + y * surface->w * 4 + 1] = ((unsigned char*)surface->pixels)[x * 3 + y * surface->w * 3 + 1];
      image_data[x * 4 + y * surface->w * 4 + 2] = ((unsigned char*)surface->pixels)[x * 3 + y * surface->w * 3 + 2];
      image_data[x * 4 + y * surface->w * 4 + 3] = 0;
    }
    
  } else if (surface->format->BytesPerPixel == 4) {
    memcpy(image_data, surface->pixels, sizeof(unsigned char) * 4 * surface->w * surface->h);
  } else {
    error("File %s has %i channels, don't know how to load it!", filename, surface->format->BytesPerPixel);
  }

  image* i = image_new(surface->w, surface->h, image_data);
  
  free(image_data);
  
  SDL_FreeSurface(surface);
  
  return i;
}