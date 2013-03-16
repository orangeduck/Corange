#version 120

uniform sampler2D normals;
uniform sampler2D color;
uniform sampler2D attribs;
uniform sampler2D random;

uniform sampler2D surface_diffuse;
uniform sampler2D surface_diffuse_far;

uniform sampler2D surface_bump;
uniform sampler2D surface_bump_far;

uniform vec3 camera_position;
uniform vec3 camera_direction;

#define MAX_LIGHTS 32

uniform int num_lights;
uniform float light_power[MAX_LIGHTS];
uniform float light_falloff[MAX_LIGHTS];
uniform vec3 light_position[MAX_LIGHTS];
uniform vec3 light_target[MAX_LIGHTS];
uniform vec3 light_diffuse[MAX_LIGHTS];
uniform vec3 light_ambient[MAX_LIGHTS];
uniform vec3 light_specular[MAX_LIGHTS];

varying vec3 position;

/* Headers */

vec3 to_gamma(vec3 color);
vec3 from_gamma(vec3 color);
vec3 swap_red_green_inv(vec3 color);

float shadow_amount(vec4 light_pos, sampler2D light_depth);
float shadow_amount_pcf25(vec4 light_pos, sampler2D light_depth, float kernel);

vec3 apply_fog_blue(vec3 pixel, vec3 position, vec3 camera_position);

/* End */

void main() {
  
  vec2 random_off = texture2D(random, vec2(position.x, position.z) / 512.0).xy - 0.5;
  
  vec2 world_uvs = vec2(position.x, position.z) / 1024.0 + random_off * 0.0025;
  vec2 local_uvs = vec2(position.x, position.z) / 3;
  vec2 far_uvs = vec2(position.x, position.z) / 100.0;
  
  float total_dist = distance(camera_position, position);
  float dist_func = clamp(total_dist / 200.0, 0.0, 1.0);
  
  vec3 ground_color = texture2D(color, world_uvs).rgb;
  vec4 ground_attribs = texture2D(attribs, world_uvs);
  
  float near_albedo = dot(ground_attribs, texture2D(surface_diffuse, local_uvs));
  float far_albedo = dot(ground_attribs, texture2D(surface_diffuse_far, far_uvs));
  vec3 albedo = from_gamma(ground_color * mix(near_albedo, far_albedo, dist_func));
  
  float bump_near_c = dot(ground_attribs, texture2D(surface_bump, local_uvs));
  float bump_near_x = dot(ground_attribs, texture2D(surface_bump, local_uvs + vec2(0.001, 0) ));
  float bump_near_y = dot(ground_attribs, texture2D(surface_bump, local_uvs + vec2(0, 0.001) ));
  
  float bump_far_c = dot(ground_attribs, texture2D(surface_bump_far, far_uvs));
  float bump_far_x = dot(ground_attribs, texture2D(surface_bump_far, far_uvs + vec2(0.001, 0) ));
  float bump_far_y = dot(ground_attribs, texture2D(surface_bump_far, far_uvs + vec2(0, 0.001) ));
  
  float bump_c =  mix(bump_near_c, bump_far_c, dist_func);
  float bump_x =  mix(bump_near_x, bump_far_x, dist_func);
  float bump_y =  mix(bump_near_y, bump_far_y, dist_func);
  
  float flatness = 1000.0;
  
  vec3 basis_x = normalize(vec3(flatness, bump_x, 0) - vec3(0, bump_c, 0));
  vec3 basis_z = normalize(vec3(0, bump_y, flatness) - vec3(0, bump_c, 0));
  vec3 basis_y = cross(basis_x, basis_z);
  mat3 basis = mat3(basis_x, basis_y, basis_z);
  
  vec3 norm_up = texture2D(normals, world_uvs).rgb;
  float temp = norm_up.g;
  norm_up.g = 1-norm_up.b;
  norm_up.b = temp;
  norm_up.r = 1-norm_up.r;
  norm_up = (norm_up * 2.0) - 1.0;
  
  vec3 normal = norm_up;
  //vec3 normal = basis * norm_up;
  
  vec3 camera_vector = normalize(camera_position - position);
  
  vec3 diffuse = vec3(0,0,0);
  vec3 ambient = vec3(0,0,0);
  
  for(int i = 0; i < num_lights; i++) {
    vec3 light_vector = light_position[i] - position;
    float power = light_power[i] / pow(length(light_vector), light_falloff[i]);
    
    vec3 light_dir = normalize(light_target[i] - light_position[i]);
    
    float n_dot_l = max( dot( normal, light_dir) , 0.0);
    
    diffuse += power * light_diffuse[i] * albedo * n_dot_l;
    ambient += power * light_ambient[i] * albedo;
  }
  
  vec3 final = to_gamma(diffuse + ambient);
  
  gl_FragColor.rgb = apply_fog_blue(final, position, camera_position);
  gl_FragColor.a = 1.0;
}
