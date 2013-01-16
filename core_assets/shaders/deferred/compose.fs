#version 120

uniform sampler2D diffuse_texture;
uniform sampler2D positions_texture;
uniform sampler2D normals_texture;
uniform sampler2D random_texture;
uniform sampler2D depth_texture;
uniform sampler2D shadows_texture0;
uniform sampler2D shadows_texture1;
uniform sampler2D shadows_texture2;
uniform sampler2D ssao_texture;
uniform sampler2D env_texture;

uniform vec3 camera_position;

#define MAX_LIGHTS 32

uniform int lights_num;
uniform int light_shadows;
uniform float light_power[MAX_LIGHTS];
uniform float light_falloff[MAX_LIGHTS];
uniform vec3 light_position[MAX_LIGHTS];
uniform vec3 light_target[MAX_LIGHTS];
uniform vec3 light_diffuse[MAX_LIGHTS];
uniform vec3 light_ambient[MAX_LIGHTS];
uniform vec3 light_specular[MAX_LIGHTS];

uniform float light_start[3];
uniform mat4 light_view[3];
uniform mat4 light_proj[3];

varying vec2 fTexcoord;

#define MAT_DISCARD 1
#define MAT_REFLECT_MINOR 2
#define MAT_REFLECT_MAJOR 3
#define MAT_FLAT 4
#define MAT_REFLECT_NONE 5
#define MAT_LEAF 6

/* Headers */

vec3 to_gamma(vec3 color);
vec3 from_gamma(vec3 color);

/* End */

const vec3[32] shadow_sample_sphere = vec3[32](
  vec3(-0.00,  0.02, -0.03), vec3( 0.35, -0.04,  0.31), vec3( 0.66, -0.32,  0.53), 
  vec3(-0.04, -0.04,  0.01), vec3( 0.24, -0.22,  0.89), vec3(-0.09,  0.10, -0.54), 
  vec3( 0.24,  0.04,  0.01), vec3( 0.37,  0.88,  0.05), vec3( 0.02,  0.11, -0.19), 
  vec3(-0.04,  0.83, -0.01), vec3( 0.33,  0.11, -0.44), vec3( 0.21, -0.17,  0.28), 
  vec3( 0.48, -0.30,  0.34), vec3( 0.39, -0.72,  0.43), vec3( 0.19,  0.20,  0.03), 
  vec3( 0.35, -0.04, -0.01), vec3(-0.00, -0.02, -0.25), vec3(-0.07,  0.12, -0.04), 
  vec3( 0.00,  0.01, -0.40), vec3(-0.27,  0.41, -0.44), vec3( 0.13,  0.26, -0.14), 
  vec3( 0.15,  0.19, -0.26), vec3(-0.32,  0.29,  0.56), vec3(-0.00, -0.00,  0.13), 
  vec3(-0.36, -0.18,  0.07), vec3( 0.70,  0.21,  0.39), vec3(-0.36,  0.17,  0.91), 
  vec3(-0.11, -0.12,  0.26), vec3(-0.59, -0.67,  0.14), vec3(-0.24, -0.75,  0.27), 
  vec3( 0.18,  0.04, -0.58), vec3(-0.16,  0.11, -0.26));

const float shadow_bias = 0.001;

float shadow_amount(vec3 position, mat4 light_view, mat4 light_proj, sampler2D light_depth, const int samples, const float kernel, vec2 seed) {
 
  vec4 light_pos = light_proj * light_view * vec4(position, 1.0);
  light_pos = light_pos / light_pos.w;
  
  float pixel_depth = light_pos.z / 2 + 0.5;
  vec2  pixel_coords = vec2(light_pos.x, light_pos.y) / 2.0 + 0.5;
  
  float shade = 1.0;  
  for (int i = 0; i < samples; i++) {
    vec2 offset = reflect(shadow_sample_sphere[i].xy, seed);
    float shadow_depth = texture2D( light_depth, pixel_coords + offset * kernel ).r;
    shade = shade - sign(pixel_depth - shadow_depth - shadow_bias) * (float(1) / float(samples));
  }
  
  return shade;
  
}

void main() {

	vec4 position = texture2D( positions_texture, fTexcoord );
  int material = int(position.a);
  
  if (material == MAT_DISCARD) { discard; }
  
	vec4 diffuse_a = texture2D(diffuse_texture, fTexcoord );
	vec3 albedo = diffuse_a.rgb;
  float spec = diffuse_a.a;
  
  if (material == MAT_FLAT) { gl_FragColor.rgb = albedo; return; }
  
  float depth = texture2D( depth_texture, fTexcoord).r;
  vec4 normal_a = texture2D(normals_texture, fTexcoord);
	vec3 normal = normalize(normal_a.rgb);
  float glossiness = normal_a.a;
  
  const float noise_tile = 1.0;
  vec3 random = 
    abs(normal.x) * texture2D(random_texture, position.yz * noise_tile).rgb +
    abs(normal.y) * texture2D(random_texture, position.xz * noise_tile).rgb +
    abs(normal.z) * texture2D(random_texture, position.xy * noise_tile).rgb;
  random = normalize(random * 2.0 - 1.0);
  
  float shadow0 = shadow_amount(position.xyz, light_view[0], light_proj[0], shadows_texture0, 8, 0.00075, random.xy);
  float shadow1 = shadow_amount(position.xyz, light_view[1], light_proj[1], shadows_texture1, 6, 0.00075, random.xy);
  float shadow2 = shadow_amount(position.xyz, light_view[2], light_proj[2], shadows_texture2, 4, 0.00100, random.xy);
	float shadow = depth > light_start[2] ? shadow2 : (depth > light_start[1] ? shadow1 : shadow0);
    
  vec3 ssao = texture2DLod(ssao_texture, fTexcoord, 1.0).rgb;
  
  vec3 diffuse  = vec3(0.0, 0.0, 0.0);
  vec3 ambient  = vec3(0.0, 0.0, 0.0);
  vec3 specular = vec3(0.0, 0.0, 0.0);
  
  vec3 eye_dir = normalize(camera_position - position.xyz);
  
  for(int i = 0; i < lights_num; i++) {
    
    vec3 light_vector = light_position[i] - position.xyz;
    float power = light_power[i] / pow(length(light_vector), light_falloff[i]);
    
    vec3 light_dir = normalize(light_position[i] - light_target[i]);
    vec3 light_half = normalize(light_dir + eye_dir);
    
    float n_dot_l = max(dot(normal, light_dir), 0.0);
    float n_dot_h = spec * pow(max(dot(normal, light_half),0.0), glossiness);
    
    if (i == light_shadows) {
      n_dot_l *= shadow;
      n_dot_h *= shadow;
    }
    
    ambient  += power * light_ambient[i] * albedo * ssao;
    diffuse  += power * light_diffuse[i] * albedo * n_dot_l;
    specular += power * light_specular[i] * n_dot_h;
    
  }
  
  if ((material == MAT_REFLECT_MINOR) || 
      (material == MAT_REFLECT_MAJOR) || 
      (material == MAT_LEAF)) {
    
    float env_factor = (material == MAT_REFLECT_MAJOR) ? 2.0 : 1.0;
    
    vec3 reflected = normalize(reflect(eye_dir, normal));
    vec3 env = from_gamma(texture2D(env_texture, vec2(reflected.x, -reflected.y)).rgb) * ambient * env_factor;
    float env_amount = (1.0 - dot(eye_dir, normal)) * spec * env_factor;
    
    diffuse = mix(diffuse, env, clamp(env_amount, 0.0, 1.0));
  }
  
  //vec3 light_dir = normalize(light_position[0] - light_target[0]);
  //float n_dot_b = clamp(1-dot(normal, light_dir), 0.0, 0.25);
  
  //if (material == MAT_LEAF) {
  //  diffuse += albedo * n_dot_b;
  //  diffuse *= 0.9;
  //}
  
  vec3 final = diffuse + ambient + specular;  
  gl_FragColor.rgb = final;

} 
