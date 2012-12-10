/**
*** :: Light ::
***
***   -- WIP -- 
***
***   Object for rendering an instance of a terrain
***   asset using particular textures and attributes.
***
**/

#ifndef landscape_h
#define landscape_h

#include "cengine.h"
#include "casset.h"

typedef struct {
  
  asset_hndl heightmap;
  asset_hndl attribmap;
  
  float scale;
  float size_x;
  float size_y;
  
  asset_hndl ground0;
  asset_hndl ground1;
  asset_hndl ground2;
  asset_hndl ground3;
  
  asset_hndl ground0_nm;
  asset_hndl ground1_nm;
  asset_hndl ground2_nm;
  asset_hndl ground3_nm;
  
} landscape;

landscape* landscape_new();
void landscape_delete(landscape* l);

float landscape_height(landscape* l, vec2 pos);
vec3  landscape_normal(landscape* l, vec2 pos);

void landscape_paint_height(landscape* l, vec2 pos, float radius, float value);
void landscape_paint_color(landscape* l, vec2 pos, float radius, int type);

#endif
