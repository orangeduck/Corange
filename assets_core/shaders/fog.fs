#version 120

/* Prototypes */

vec3 apply_fog(vec3 pixel, vec3 position, vec3 camera_position, 
               float fog_distance, float fog_height,
               vec3 horizon_color, vec3 sky_color, 
               float fog_color_scale);

vec3 apply_fog_blue(vec3 pixel, vec3 position, vec3 camera_position);
               
/* End */

vec3 apply_fog(vec3 pixel, vec3 position, vec3 camera_position, 
               float fog_distance, float fog_height,
               vec3 horizon_color, vec3 sky_color, 
               float fog_color_scale) {
  
  float dist = distance(position, camera_position);
  float far_amount = clamp(dist / fog_distance - 0.25, 0.1, 1.1);
  float valley_amount = clamp(fog_height / (position.y + fog_height) - 0.5, 0.25, 0.75);
  float amount = clamp(far_amount * pow(valley_amount, 1.5) * 7.5, 0.0, 0.75);
  
  //vec3 fog_color = mix(horizon_color, sky_color, clamp(position.y / fog_color_scale, 0, 1));
  vec3 fog_color = sky_color;
  
  return mix(pixel, fog_color, amount);
}

vec3 apply_fog_blue(vec3 pixel, vec3 position, vec3 camera_position) {
  return apply_fog(pixel, position, camera_position, 500.1, 200.1, 5.0 * vec3(0.5098,0.6501,0.7529), 5.0 * vec3(0.066,0.2,0.450), 500.1);
}