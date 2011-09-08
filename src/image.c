#include "image.h"

#include "asset_manager.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define NO_SDL_GLEXT
#include "SDL/SDL.h"
#include "SDL/SDL_Image.h"

image* image_new(int width, int height, char* data) {
  
  image* i = malloc(sizeof(image));
  i->data = malloc(width * height);
  memcpy(data, i->data, width * height);
  i->width = width;
  i->height = height;
  
  i->repeat_type = image_repeat_clamp;
  i->sample_type = image_sample_linear;
  
}

image* image_blank(int width, int height) {
  
  image* i = malloc(sizeof(image));
  i->data = calloc(width * height, 1);
  i->width = width;
  i->height = height;
  
  i->repeat_type = image_repeat_clamp;
  i->sample_type = image_sample_linear;
  
}

void image_delete(image* i) {
  free(i->data);
  free(i);
}

vector4 image_sample_pixel(image* i, int u, int v) {
  
  float r = (float)i->data[u * 4 + v * i->width * 4 + 0] / 255;
  float g = (float)i->data[u * 4 + v * i->width * 4 + 1] / 255;
  float b = (float)i->data[u * 4 + v * i->width * 4 + 2] / 255;
  float a = (float)i->data[u * 4 + v * i->width * 4 + 3] / 255;

  return v4(r,g,b,a);
  
}

vector4 image_sample(image* i, vector2 uv) {

  if ( i->repeat_type == image_repeat_tile ) {
    uv = v2_fmod(uv, 1.0);
  } else if ( i->repeat_type == image_repeat_clamp ) {
    uv = v2_saturate(uv);
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
  
  float amount_x = fmod(u, 1.0);
  float amount_y = fmod(v, 1.0);
  
  vector4 s1, s2, s3, s4;
  
  s1 = image_sample_pixel(i, s1_u, s1_v);
  s2 = image_sample_pixel(i, s2_u, s2_v);
  s3 = image_sample_pixel(i, s3_u, s3_v);
  s4 = image_sample_pixel(i, s4_u, s4_v);
  
  if ( i->sample_type == image_sample_linear ) {
    return v4_bilinear_interpolation(s1, s2, s3, s4, amount_x, amount_y);
  } else if ( i->sample_type == image_sample_nearest ) {
    return v4_binearest_neighbor_interpolation(s1, s2, s3, s4, amount_x, amount_y);
  }
  
}

void tga_save_file(image* i, char* filename) {

    int xa= i->width % 256;
    int xb= (i->width-xa)/256;

    int ya= i->height % 256;
    int yb= (i->height-ya)/256;
    unsigned char header[18]={0,0,2,0,0,0,0,0,0,0,0,0,(char)xa,(char)xb,(char)ya,(char)yb,32,0};
    
    SDL_RWops* file = SDL_RWFromFile(filename, "wb");
    SDL_RWwrite(file, header, sizeof(header), 1);
    SDL_RWwrite(file, i->data, i->width * i->height * 4, 1 );
    SDL_RWclose(file);

}

void texture_write_to_file(image* i, char* filename) {
  
  char* ext = asset_file_extension(filename);
  
  if ( strcmp(ext, "tga") == 0 ) {
    tga_save_file(i, filename);
  } else {
    printf("Error: Cannot save texture to &s, unknown file extension %s. Try .tga!\n", filename, ext);
  }
  
  free(ext);
}

image* bmp_load_file(char* filename) {
  
  SDL_Surface *surface;
   
  surface = SDL_LoadBMP(filename);
   
  if (!surface) {
    printf("Error: Could not load file %s: %s\n",filename , SDL_GetError());
    return NULL;
  }
  
  if (surface->format->BytesPerPixel != 4) {
    printf("Error loading %s. Needs four channels!");
  }

  image* i = image_new(surface->w, surface->h, surface->pixels);
  
  SDL_FreeSurface(surface);
  
  return i;
}

image* image_load_file(char* filename) {

  SDL_Surface *surface;
  
  surface = IMG_Load(filename);
   
  if (!surface) {
    printf("Error: Could not load file %s: %s\n",filename , SDL_GetError());
    return NULL;
  }
  
  if (surface->format->BytesPerPixel != 4) {
    printf("Error loading %s. Needs four channels!");
  }

  image* i = image_new(surface->w, surface->h, surface->pixels);
  
  SDL_FreeSurface(surface);
  
  return i;
}

image* png_load_file(char* filename) {
  return image_load_file(filename);
}

image* tif_load_file(char* filename) {
  return image_load_file(filename);
}

image* jpg_load_file(char* filename) {
  return image_load_file(filename);
}