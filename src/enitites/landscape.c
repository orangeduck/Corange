#include <stdlib.h>

#include "entities/landscape.h"
#include "assets/terrain.h"

landscape* landscape_new() {
  
  landscape* l = malloc(sizeof(landscape));
  
  l->heightmap = asset_hndl_null();
  l->attribmap = asset_hndl_null();
  
  l->scale = 0.25;
  l->size_x = 128;
  l->size_y = 128;
  
  l->ground0 = asset_hndl_null();
  l->ground1 = asset_hndl_null();
  l->ground2 = asset_hndl_null();
  l->ground3 = asset_hndl_null();
  
  l->ground0_nm = asset_hndl_null();
  l->ground1_nm = asset_hndl_null();
  l->ground2_nm = asset_hndl_null();
  l->ground3_nm = asset_hndl_null();
  
  return l;
  
}

void landscape_delete(landscape* l) {
  free(l);
}

float landscape_height(landscape* l, vec2 pos) {
  
  terrain* t = asset_hndl_ptr(l->heightmap);
  
  pos.x = (1 - ((pos.x / l->size_x) + 0.5)) * t->width;
  pos.y = (1 - ((pos.y / l->size_y) + 0.5)) * t->height;
  
  return l->scale * terrain_height(t, pos);
  
}

vec3  landscape_normal(landscape* l, vec2 pos) {
  
  terrain* t = asset_hndl_ptr(l->heightmap);

  pos.x = (1 - ((pos.x / l->size_x) + 0.5)) * t->width;
  pos.y = (1 - ((pos.y / l->size_y) + 0.5)) * t->height;
  
  return terrain_normal(t, pos);

}

void landscape_paint_height(landscape* l, vec2 pos, float radius, float value) {

}

void landscape_paint_color(landscape* l, vec2 pos, float radius, int type) {

}
