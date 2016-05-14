#ifndef sky_h
#define sky_h

#include "cengine.h"
#include "casset.h"

static const float TIME_SUNRISE  = 0.00;
static const float TIME_MORNING  = 0.10;
static const float TIME_MIDDAY   = 0.25;
static const float TIME_EVENING  = 0.40;
static const float TIME_SUNSET   = 0.50;
static const float TIME_MIDNIGHT = 0.75;

typedef struct {
  
  float time;
  uint32_t seed;
  
  asset_hndl cloud_mesh[14];
  asset_hndl cloud_tex[14];
  float cloud_opacity[14];
  
  asset_hndl sun_sprite;
  asset_hndl sun_tex;
  
  asset_hndl moon_sprite;
  asset_hndl moon_tex;
  
  asset_hndl stars_sprite;
  asset_hndl stars_tex;
  
  bool is_day;
  vec3 wind;
  
  mat4 world_sun;
  mat4 world_moon;
  mat4 world_stars;
  
  float moon_power;
  vec3 moon_direction;
  vec3 moon_diffuse;
  vec3 moon_ambient;
  vec3 moon_specular;
  
  float sun_power;
  vec3 sun_direction;
  vec3 sun_diffuse;
  vec3 sun_ambient;
  vec3 sun_specular;
  
  float sky_power;
  vec3 sky_direction;
  vec3 sky_diffuse;
  vec3 sky_ambient;
  vec3 sky_specular;
  
  float ground_power;
  vec3 ground_direction;
  vec3 ground_diffuse;
  vec3 ground_ambient;
  vec3 ground_specular;
  
} sky;

sky* sky_new();
void sky_delete(sky* s);
void sky_update(sky* s, float t, uint32_t seed);

#endif