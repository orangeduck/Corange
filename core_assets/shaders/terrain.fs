uniform sampler2D normals;
uniform sampler2D color;
uniform sampler2D attribs;
uniform sampler2D random;

uniform sampler2D surface_diffuse0;
uniform sampler2D surface_normals0;
uniform sampler2D surface_diffuse_far0;
uniform sampler2D surface_normals_far0;

uniform sampler2D surface_diffuse1;
uniform sampler2D surface_normals1;
uniform sampler2D surface_diffuse_far1;
uniform sampler2D surface_normals_far1;

uniform sampler2D surface_diffuse2;
uniform sampler2D surface_normals2;
uniform sampler2D surface_diffuse_far2;
uniform sampler2D surface_normals_far2;

uniform sampler2D surface_diffuse3;
uniform sampler2D surface_normals3;
uniform sampler2D surface_diffuse_far3;
uniform sampler2D surface_normals_far3;

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

vec3 attribute_sum(vec4 weights, vec3 p0, vec3 p1, vec3 p2, vec3 p3) {
  float scale = weights.r + weights.g + weights.b + weights.a;
  return (p0 * weights.r + p1 * weights.g + p2 * weights.b + p3 * weights.a) / scale;
}

vec3 normal_scale(vec3 n) {
  float temp = n.g;
  n.g = n.b;
  n.b = temp;
  n.r = 1-n.r;
  return ((n*2)-1);
}

void main() {
  
  vec2 random_off = texture2D(random, vec2(position.x, position.z) / 512.0).xy - 0.5;
  
  vec2 world_uvs = vec2(position.x, position.z) / 1024.0 + random_off * 0.0025;
  vec2 local_uvs = vec2(position.x, position.z) / 3;
  vec2 far_uvs = vec2(position.x, position.z) / 100.0;
  
  float total_dist = distance(camera_position, position);
  float dist_func = clamp(total_dist / 200.0, 0.0, 1.0);
  
  vec3 ground_color = texture2D(color, world_uvs).rgb;
  vec4 ground_attribs = texture2D(attribs, world_uvs);
  
  vec3 surdiff0 = texture2D(surface_diffuse0, local_uvs).rgb;
  vec3 surdiff1 = texture2D(surface_diffuse1, local_uvs).rgb;
  vec3 surdiff2 = texture2D(surface_diffuse2, local_uvs).rgb;
  vec3 surdiff3 = texture2D(surface_diffuse3, local_uvs).rgb;
  vec3 near_albedo = attribute_sum(ground_attribs, surdiff0, surdiff1, surdiff2, surdiff3);
  
  vec3 surdifffar0 = texture2D(surface_diffuse_far0, far_uvs).rgb;
  vec3 surdifffar1 = texture2D(surface_diffuse_far1, far_uvs).rgb;
  vec3 surdifffar2 = texture2D(surface_diffuse_far2, far_uvs).rgb;
  vec3 surdifffar3 = texture2D(surface_diffuse_far3, far_uvs).rgb;
  vec3 far_albedo = attribute_sum(ground_attribs, surdifffar0, surdifffar1, surdifffar2, surdifffar3);
  
  vec3 surnorm0 = texture2D(surface_normals0, local_uvs).rgb;
  vec3 surnorm1 = texture2D(surface_normals1, local_uvs).rgb;
  vec3 surnorm2 = texture2D(surface_normals2, local_uvs).rgb;
  vec3 surnorm3 = texture2D(surface_normals3, local_uvs).rgb;
  vec3 near_surface_norm = attribute_sum(ground_attribs, surnorm0, surnorm1, surnorm2, surnorm3);
  
  vec3 surnormfar0 = texture2D(surface_normals_far0, far_uvs).rgb;
  vec3 surnormfar1 = texture2D(surface_normals_far1, far_uvs).rgb;
  vec3 surnormfar2 = texture2D(surface_normals_far2, far_uvs).rgb;
  vec3 surnormfar3 = texture2D(surface_normals_far3, far_uvs).rgb;
  vec3 far_surface_norm = attribute_sum(ground_attribs, surnormfar0, surnormfar1, surnormfar2, surnormfar3);
  
  vec3 albedo = from_gamma(ground_color * mix(near_albedo, far_albedo, dist_func));
  vec3 surface_normals =  normal_scale(mix(near_surface_norm, far_surface_norm, dist_func));
  
  vec3 norm_up = texture2D(normals, world_uvs).rgb;
  float temp = norm_up.g;
  norm_up.g = 1-norm_up.b;
  norm_up.b = temp;
  norm_up.r = 1-norm_up.r;
  norm_up = (norm_up * 2.0) - 1.0;
  
  vec3 norm_left = cross(norm_up, vec3(0,1,0));
  vec3 norm_down = cross(norm_up, norm_left);
  mat3 basis = mat3(norm_down, norm_up, norm_left);
  
  vec3 normal = basis * surface_normals;
  
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
