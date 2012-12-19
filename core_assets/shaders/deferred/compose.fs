#version 120

uniform sampler2D diffuse_texture;
uniform sampler2D positions_texture;
uniform sampler2D normals_texture;
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

/* Headers */

float shadow_amount(vec3 position, mat4 light_view, mat4 light_proj, sampler2D light_depth, int samples, float kernel, vec2 seed);
float shadow_amount_soft_pcf25(vec4 light_pos, sampler2D light_depth, float hardness);

vec3 to_gamma(vec3 color);
vec3 from_gamma(vec3 color);

vec3 apply_fog_blue(vec3 pixel, vec3 position, vec3 camera_position);

/* End */

void main() {

	vec4 position = texture2D( positions_texture, fTexcoord );
  int material = int(position.a);
  
  if (material == 1) { discard; }
  
	vec4 diffuse_a = texture2D(diffuse_texture, fTexcoord );
	vec3 albedo = diffuse_a.rgb;
  float spec = diffuse_a.a;
  
  if (material == 4) { gl_FragColor.rgb = albedo; return; }
  
  float depth = texture2D( depth_texture, fTexcoord).r;
  
  float shadow0 = shadow_amount(position.xyz, light_view[0], light_proj[0], shadows_texture0, 8, 0.0, position.xz);
  float shadow1 = shadow_amount(position.xyz, light_view[1], light_proj[1], shadows_texture1, 4, 0.0, position.xz);
  float shadow2 = shadow_amount(position.xyz, light_view[2], light_proj[2], shadows_texture2, 1, 0.0, position.xz);
	float shadow = depth > light_start[2] ? shadow2 : (depth > light_start[1] ? shadow1 : shadow0);
  
  vec3 ssao = texture2DLod(ssao_texture, fTexcoord, 0.0).rgb;
	
  vec4 normal_a = texture2D(normals_texture, fTexcoord);
	vec3 normal = normalize(normal_a.rgb);
  float glossiness = normal_a.a;
  
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
  
  if ((material == 2) || (material == 3)) {
  
    float env_factor;
    
    if (material == 3) {
      env_factor = 1.5;
    } else {
      env_factor = 0.5;
    }
  
    vec3 reflected = normalize(reflect(eye_dir, normal));
    vec3 env = from_gamma(texture2D(env_texture, vec2(reflected.x, -reflected.y)).rgb) * ambient;
    float env_amount = (1.0 - dot(eye_dir, normal)) * spec * env_factor;
    
    diffuse = mix(diffuse, env, env_amount);
  }
  
  vec3 final = diffuse + ambient + specular;  
  gl_FragColor.rgb = final;

} 
