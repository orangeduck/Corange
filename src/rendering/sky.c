
#include "rendering/sky.h"

bool sky_isday(float t) {
  t = fmod(t, 1.0);
  return (t > 0.0 && t < 0.5);
}

asset_hndl sky_mesh_sun(float t) {
  return asset_hndl_new_load(P("$CORANGE/resources/sun_sprite.obj"));
}

asset_hndl sky_mesh_moon(float t) {
  return asset_hndl_new_load(P("$CORANGE/resources/moon_sprite.obj"));
}

asset_hndl sky_mesh_stars(float t) {
  return asset_hndl_null();
}

asset_hndl sky_tex_sun(float t) {
  return asset_hndl_new_load(P("$CORANGE/resources/sun_sprite.dds"));
}

asset_hndl sky_tex_moon(float t) {
  return asset_hndl_new_load(P("$CORANGE/resources/moon_sprite.dds"));
}

asset_hndl sky_tex_stars(float t) {
  return asset_hndl_null();
}

mat4 sky_mesh_sun_world(float t) {
  vec3 dir = sky_sun_direction(t);
  vec3 axis = vec3_cross(dir, vec3_new(0, 0, -1));
  float angle = vec3_dot(dir, vec3_new(0, 0, -1));
  return mat4_rotation_axis_angle(axis, angle);
}

mat4 sky_mesh_moon_world(float t) {
  vec3 dir = sky_moon_direction(t);
  return mat4_rotation_axis_angle(vec3_neg(dir), 0);
}

mat4 sky_mesh_stars_world(float t) {
  return mat4_id();
}

float sky_sun_power(float t) {
  return max(sin(2 * M_PI * t), 0);
}

vec3 sky_sun_direction(float t) {
  float x = -0.2;
  float y = -sin(2 * M_PI * t);
  float z =  cos(2 * M_PI * t);
  return vec3_normalize(vec3_new(x, y, z));
}

vec3 sky_sun_diffuse(float t) {
  return vec3_new(1.0,  0.937, 0.8);
}

vec3 sky_sun_ambient(float t) {
  return vec3_mul(vec3_new(1.0,  0.937, 0.8), 0.2);
}

vec3 sky_sun_specular(float t) {
  return vec3_new(1.0,  0.894, 0.811);
}

vec3 sky_sky_direction(float t) {
  return vec3_neg(vec3_up());
}

float sky_moon_power(float t) {
  return clamp(sin(M_PI + 2 * M_PI * t), 0, 0.2);
}

vec3 sky_moon_direction(float t) {
  float x =  0.2 * cos(2 * M_PI * t * 0.0366);
  float y = -sin(0.56 * M_PI + 2 * M_PI * t * 0.0366);
  float z =  cos(0.42 * M_PI + 2 * M_PI * t * 0.0366);
  return vec3_normalize(vec3_new(x, y, z));
}

vec3 sky_moon_diffuse(float t) {
  return vec3_new(1, 1, 1);
}

vec3 sky_moon_ambient(float t) {
  return vec3_new(1, 1, 1);
}

vec3 sky_moon_specular(float t) {
  return vec3_new(1, 1, 1);
}

float sky_sky_power(float t) {
  float day_sky = sin(2 * M_PI * t) * 0.5 + 0.5;
  float night_sky = 0.1;
  return max(day_sky, night_sky);
}

vec3 sky_sky_diffuse(float t) {
  return vec3_mul(vec3_new(0.8, 0.93, 1.0), 0.1);
}

vec3 sky_sky_ambient(float t) {
  return vec3_mul(vec3_new(0.8, 0.93, 1.0), 0.4);
}

vec3 sky_sky_specular(float t) {
  return vec3_mul(vec3_new(0.8, 0.93, 1.0), 0.1);
}

vec3 sky_ground_direction(float t) {
  return vec3_up();
}

vec3 sky_ground_diffuse(float t);
vec3 sky_ground_ambient(float t);
vec3 sky_ground_specular(float t);
