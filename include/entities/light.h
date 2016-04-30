/**
*** :: Light ::
***
***   Currently this is so large because
***   it supports all types of light.
***
***   This means it holds data for shadow
***   mapping etc which may not be relevant
***   for some light types.
***
**/

#ifndef light_h
#define light_h

#include "cengine.h"

enum {
  LIGHT_TYPE_POINT        = 0,
  LIGHT_TYPE_DIRECTIONAL  = 1,
  LIGHT_TYPE_SUN          = 2,
  LIGHT_TYPE_SPOT         = 3,
};

typedef struct {
  vec3 position;
  vec3 target;
  
  vec3 diffuse_color;
  vec3 specular_color;
  vec3 ambient_color;
  
  float power;
  float falloff;
  
  bool enabled;
  bool cast_shadows;
  
  int type;
  
  /* Shadow Mapping */
  vec3 shadow_color;
  int shadow_map_width;
  int shadow_map_height;
  
  /* Orthographic Shadow Mapping */
  bool orthographic;
  float ortho_width;
  float ortho_height;
  
  /* Projection Shadow Mapping */
  float fov;
  float aspect_ratio;
  
} light;

light* light_new();
light* light_new_position(vec3 position);

/* Builds light using type's default values */
light* light_new_type(vec3 position, int type);
void light_delete(light* l);

void light_set_type(light* l, int type);

vec3 light_direction(light* l);

mat4 light_view_matrix(light* l);
mat4 light_proj_matrix(light* l);

#endif