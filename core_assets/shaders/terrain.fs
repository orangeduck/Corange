uniform sampler2D normals;
uniform sampler2D color;

uniform sampler2D ground;
uniform sampler2D ground_normals;

uniform sampler2D ground_far;
uniform sampler2D ground_far_normals;

uniform sampler2D shadow_map;

uniform int lod_index;

uniform mat4 light_view;
uniform mat4 light_proj;

uniform vec3 camera_position;

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
varying vec3 normal;

/* Headers */

vec3 to_gamma(vec3 color);
vec3 from_gamma(vec3 color);
vec3 swap_red_green_inv(vec3 color);

float shadow_amount(vec4 light_pos, sampler2D light_depth);
float shadow_amount_pcf25(vec4 light_pos, sampler2D light_depth, float kernel);

vec3 apply_fog_blue(vec3 pixel, vec3 position, vec3 camera_position);

/* End */

void main() {
  
  vec2 world_uvs = vec2(position.x, position.z) / 1024.0;
  vec2 local_uvs = vec2(position.x, position.z) / 2.5;
  vec2 far_uvs = vec2(position.x, position.z) / 200.0;
  
  float total_dist = distance(camera_position, position);
  float dist_func = clamp(total_dist / 300.0, 0.0, 1.0);
  
  vec3 ground_color = from_gamma(texture2D(color, world_uvs).rgb);
  
  vec3 far_albedo = ground_color * from_gamma(texture2D(ground_far, far_uvs).rgb);
  vec3 near_albedo = ground_color * from_gamma(texture2D(ground, local_uvs).rgb);
  
  vec3 albedo = mix(near_albedo, far_albedo, dist_func);
  
  vec3 norm_near = normal;
  vec3 norm_far = texture2D(normals, world_uvs).rgb;
  norm_far.g = 1-norm_far.g;
  norm_far = (norm_far * 2.0) - 1.0;
  
  vec3 ground_normals_far = texture2D(ground_far_normals, far_uvs).rgb;
  ground_normals_far = (ground_normals_far * 2.0) - 1.0;
  vec3 ground_normals_near = texture2D(ground_normals, local_uvs).rgb;
  ground_normals_near = (ground_normals_near * 2.0) - 1.0;
  
  vec3 ground_normals = mix(ground_normals_near, ground_normals_far, dist_func);
  
  vec3 norm = (norm_far + ground_normals) / 2;
  
  vec4 light_pos = light_proj * light_view * vec4(position, 1.0);
  //float shadow = shadow_amount_pcf25(light_pos, shadow_map, 0.001);
  //float shadow = shadow_amount(light_pos, shadow_map);
  float shadow = 1.0;
  
  vec3 camera_vector = normalize(camera_position - position);
  
  vec3 diffuse = vec3(0,0,0);
  vec3 ambient = vec3(0,0,0);
  
  for(int i = 0; i < num_lights; i++) {
    vec3 light_vector = light_position[i] - position;
    float power = light_power[i] / pow(length(light_vector), light_falloff[i]);
    
    vec3 light_dir = normalize(light_target[i] - light_position[i]);
    
    float n_dot_l = max( dot( norm, light_dir) , 0.0);
    
    if (i == 0) {
      n_dot_l *= shadow;
    }
    
    diffuse += power * light_diffuse[i] * albedo * n_dot_l;
    ambient += power * light_ambient[i] * albedo;
  }
  
  float fres_amount = 0.05;
  float glossiness = 5.0;
  
  float n_dot_f = shadow * pow(clamp(1.0 - dot(camera_vector, normal), 0, 1), glossiness); 
  vec3 fresnel = light_ambient[0] * fres_amount * n_dot_f;
  
  vec3 final = to_gamma(diffuse + ambient + fresnel);
  
  final = apply_fog_blue(final, position, camera_position);
  
  gl_FragColor.rgb = final;
  //gl_FragColor.rgb = vec3(shadow, shadow, shadow);
  gl_FragColor.a = 1.0;
}
