#ifndef light_h
#define light_h

#include "cengine.h"

typedef struct {

  vec3 position;
  vec3 target;
  
  vec3 diffuse_color;
  vec3 specular_color;
  vec3 ambient_color;
  
  float power;
  float falloff;
  
  int type;
  
  bool enabled;
  bool cast_shadows;
  
  bool orthographic;
  float ortho_width;
  float ortho_height;
  
  float fov;
  float aspect_ratio;
  
  vec3 shadow_color;
  int shadow_map_width;
  int shadow_map_height;
  
} light;

static const int light_type_point = 0;
static const int light_type_directional = 1;
static const int light_type_sun = 2;
static const int light_type_spot = 3;

light* light_new(vec3 position);
light* light_new_type(vec3 position, int type);
void light_delete(light* l);

void light_set_type(light* l, int type);

vec3 light_direction(light* l);

mat4 light_view_matrix(light* l);
mat4 light_proj_matrix(light* l);

#endif