#ifndef effect_h
#define effect_h

#include "cengine.h"
#include "casset.h"

typedef struct {

  float time;
  
  float rotation;
  float rotation_r;
  
  vec3 scale;
  vec3 scale_r;
  
  vec4 color;
  vec4 color_r;
  
  vec3 force;
  vec3 force_r;
  
} effect_key;

effect_key effect_key_lerp(effect_key x, effect_key y, float amount);

typedef struct {

  asset_hndl texture;
  asset_hndl texture_nm;
  
  GLuint blend_src;
  GLuint blend_dst;
  
  int count;
  float depth;
  float thickness;
  float bumpiness;
  float scattering;
  
  float lifetime;
  float output;
  float output_r;
  
  bool alpha_decay;
  bool color_decay;
  
  int keys_num;
  effect_key* keys;
  
} effect;

effect* effect_new();
effect* effect_load_file(char* filename);
void effect_delete(effect* e);

effect_key effect_get_key(effect* e, float ptime);

#endif