#version 120

uniform sampler2D diffuse_texture;
uniform sampler2D positions_texture;
uniform sampler2D normals_texture;
uniform sampler2D depth_texture;

uniform sampler2D shadows_texture;

uniform sampler2D ssao_texture;
uniform sampler2D env_texture;

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

uniform mat4 light_view;
uniform mat4 light_proj;

/* Headers */

float ssao_depth(vec2 texcoords, sampler2D depth_texture, sampler2D random_texture);
float shadow_amount_soft_pcf25(vec4 light_pos, sampler2D light_depth, float hardness);

vec3 to_gamma(vec3 color);
vec3 from_gamma(vec3 color);

vec3 apply_fog_blue(vec3 pixel, vec3 position, vec3 camera_position);

/* End */

void main() {

	vec4 position = texture2D( positions_texture, gl_TexCoord[0].xy );
  int mat_id = int(position.a);
  
  if (mat_id == 1) {
    gl_FragColor = texture2D( diffuse_texture, gl_TexCoord[0].xy );
    return;
  }
  
  vec4 pos_light = light_proj * light_view * vec4(position.xyz,1.0);
  float shadow = shadow_amount_soft_pcf25(pos_light, shadows_texture, 0.0005);
	float ssao = texture2DLod(ssao_texture, gl_TexCoord[0].xy, 1.0).r;
  
	vec4 diffuse_a = texture2D( diffuse_texture, gl_TexCoord[0].xy );
	vec3 albedo = diffuse_a.rgb;
  float spec = diffuse_a.a;
	
  vec4 normal_a = texture2D(normals_texture, gl_TexCoord[0].xy);
	vec3 normal = normalize(normal_a.rgb);
  float glossiness = normal_a.a;
  
  vec3 diffuse = vec3(0.0,0.0,0.0);
  vec3 ambient = vec3(0.0,0.0,0.0);
  vec3 specular = vec3(0.0,0.0,0.0);
  
  vec3 eye_dir = normalize(camera_position - position.xyz);
  
  for(int i = 0; i < num_lights; i++) {
    
    vec3 light_vector = light_position[i] - position.xyz;
    float power = light_power[i] / pow(length(light_vector), light_falloff[i]);
    
    vec3 light_dir = normalize(light_position[i] - light_target[i]);
    vec3 light_half = normalize(light_dir + eye_dir);
    
    float n_dot_l = max(dot(normal, light_dir), 0.0);
    float n_dot_h = spec * pow(max(dot(normal, light_half),0.0), glossiness);
    
    if (i == 0) {
      n_dot_l *= shadow;
      n_dot_h *= shadow;
    }
    
    ambient += power * light_ambient[i] * albedo * ssao;
    diffuse += power * light_diffuse[i] * albedo * n_dot_l;
    specular += power * light_specular[i] * n_dot_h;
    
  }
  
  if ((mat_id == 2) || (mat_id == 3)) {
  
    float env_factor;
    
    if (mat_id == 3) {
      env_factor = 2.0;
    } else {
      env_factor = 1.0;
    }
  
    vec3 reflected = normalize(reflect(eye_dir, normal));
    vec3 env = from_gamma(texture2D(env_texture, vec2(reflected.x, -reflected.y)).rgb) * ambient * 2.0;
    float env_amount = (1.0 - dot(eye_dir, normal)) * spec * env_factor;
    
    diffuse = mix(diffuse, env, env_amount);
  } 
  
  vec3 final = to_gamma(diffuse + ambient + specular);  
  gl_FragColor.rgb = final;
  
} 
