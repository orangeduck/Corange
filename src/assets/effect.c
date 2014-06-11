
#include "assets/effect.h"

effect_key effect_key_lerp(effect_key x, effect_key y, float amount) {
  effect_key ek;
  
  ek.time = lerp(x.time, y.time, amount);
  
  ek.rotation   = lerp(x.rotation,   y.rotation,   amount);
  ek.rotation_r = lerp(x.rotation_r, y.rotation_r, amount);
  
  ek.scale   = vec3_lerp(x.scale,   y.scale,   amount);
  ek.scale_r = vec3_lerp(x.scale_r, y.scale_r, amount);
  
  ek.color   = vec4_lerp(x.color,   y.color,   amount);
  ek.color_r = vec4_lerp(x.color_r, y.color_r, amount);
  
  ek.force   = vec3_lerp(x.force,   y.force,   amount);
  ek.force_r = vec3_lerp(x.force_r, y.force_r, amount);
  
  return ek;
}

effect* effect_new() {

  effect* e = malloc(sizeof(effect));
  
  e->texture = asset_hndl_null();
  e->texture_nm = asset_hndl_null();
  
  e->blend_src = GL_ONE;
  e->blend_dst = GL_ONE;
  
  e->count = 0;
  e->depth = 1.0;
  e->thickness = 1.0;
  e->bumpiness = 0.5;
  e->scattering = 0.3;
  
  e->lifetime = 1;
  e->output = 1;
  
  e->keys_num = 0;
  e->keys = NULL;
  
  return e;

}

static int SDL_RWreadline(SDL_RWops* file, char* buffer, int buffersize) {
  
  char c;
  int status = 0;
  int i = 0;
  while(1) {
    
    status = SDL_RWread(file, &c, 1, 1);
    
    if (status == -1) return -1;
    if (i == buffersize-1) return -1;
    if (status == 0) break;
    
    buffer[i] = c;
    i++;
    
    if (c == '\n') {
      buffer[i] = '\0';
      return i;
    }
  }
  
  if(i > 0) {
    buffer[i] = '\0';
    return i;
  } else {
    return 0;
  }
  
}

effect* effect_load_file(char* filename) {
  
  effect* e = effect_new();
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  if(file == NULL) {
    error("Cannot load file %s", filename);
  }
  
  char line[1024];
  while(SDL_RWreadline(file, line, 1024)) {
    
    if (line[0] == '#') continue;
    if (line[0] == '\r') continue;
    if (line[0] == '\n') continue;
    
    fpath texture, texture_nm;
    char blend_string[64];
    
    if (sscanf(line, "texture %s", texture.ptr)) {
      if (!strstr(line, "texture_nm")) {
        e->texture = asset_hndl_new(texture);
      }
    }
    
    if (sscanf(line, "texture_nm %s", texture_nm.ptr)) {
      e->texture_nm = asset_hndl_new(texture_nm);
    }
    
    if (sscanf(line, "blend_src %63s", blend_string)) {
      if (strstr(line, "blend_src")) {
        if (strcmp(blend_string, "one") == 0) { e->blend_src = GL_ONE; }
        if (strcmp(blend_string, "zero") == 0) { e->blend_src = GL_ZERO; }
        if (strcmp(blend_string, "src_alpha") == 0) { e->blend_src = GL_SRC_ALPHA; }
        if (strcmp(blend_string, "dst_alpha") == 0) { e->blend_src = GL_DST_ALPHA; }
        if (strcmp(blend_string, "src_inv_alpha") == 0) { e->blend_src = GL_ONE_MINUS_SRC_ALPHA; }
        if (strcmp(blend_string, "dst_inv_alpha") == 0) { e->blend_src = GL_ONE_MINUS_DST_ALPHA; }
      }
    }
    
    if (sscanf(line, "blend_dst %63s", blend_string)) {
      if (strstr(line, "blend_dst")) {
        if (strcmp(blend_string, "one") == 0) { e->blend_dst = GL_ONE; }
        if (strcmp(blend_string, "zero") == 0) { e->blend_dst = GL_ZERO; }
        if (strcmp(blend_string, "src_alpha") == 0) { e->blend_dst = GL_SRC_ALPHA; }
        if (strcmp(blend_string, "dst_alpha") == 0) { e->blend_dst = GL_DST_ALPHA; }
        if (strcmp(blend_string, "src_inv_alpha") == 0) { e->blend_dst = GL_ONE_MINUS_SRC_ALPHA; }
        if (strcmp(blend_string, "dst_inv_alpha") == 0) { e->blend_dst = GL_ONE_MINUS_DST_ALPHA; }
      }
    }
    
    sscanf(line, "count %i", &e->count);
    sscanf(line, "depth %f", &e->depth);
    sscanf(line, "thickness %f", &e->thickness);
    sscanf(line, "bumpiness %f", &e->bumpiness);
    sscanf(line, "scattering %f", &e->scattering);
    sscanf(line, "lifetime %f", &e->lifetime);
    sscanf(line, "output %f:%f", &e->output, &e->output_r);
    
    effect_key ek;
    if (sscanf(line, "key time=%f rotation=%f:%f scale=(%f:%f,%f:%f,%f:%f) color=(%f:%f,%f:%f,%f:%f,%f:%f) force=(%f:%f,%f:%f,%f:%f)",
      &ek.time,
      &ek.rotation, &ek.rotation_r,
      &ek.scale.x, &ek.scale_r.x,
      &ek.scale.y, &ek.scale_r.y,
      &ek.scale.z, &ek.scale_r.z,
      &ek.color.x, &ek.color_r.x, 
      &ek.color.y, &ek.color_r.y, 
      &ek.color.z, &ek.color_r.z, 
      &ek.color.w, &ek.color_r.w,
      &ek.force.x, &ek.force_r.x,
      &ek.force.y, &ek.force_r.y,
      &ek.force.z, &ek.force_r.z)) {
      
      e->keys_num++;
      e->keys = realloc(e->keys, sizeof(effect_key) * e->keys_num);
      e->keys[e->keys_num-1] = ek;
      
    }
    
  }
  
  SDL_RWclose(file);
  
  return e;
  
}

void effect_delete(effect* e) {
  
  free(e->keys);
  free(e);
  
}

effect_key effect_get_key(effect* e, float ptime) {

  float pos = saturate(ptime / e->lifetime);
  int fst = 0;
  int snd = e->keys_num-1;
  
  for (int j = 0; j < e->keys_num; j++) {
    if (e->keys[j].time < pos &&
        e->keys[j].time > e->keys[fst].time) {
      fst = j;
    }
    if (e->keys[j].time > pos &&
        e->keys[j].time < e->keys[snd].time) {
      snd = j;
    }
  }
  
  float amount = (pos - e->keys[fst].time) / (e->keys[snd].time - e->keys[fst].time);
  
  return effect_key_lerp(e->keys[fst], e->keys[snd], amount);

}