#version 120

uniform mat4 world_matrix;

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
 
uniform float glossiness;
uniform float bumpiness;
uniform float env_amount;

uniform float specular_level;
uniform float alpha_test;

uniform int recieve_shadows;

uniform sampler2D diffuse_map;
uniform sampler2D bump_map;
uniform sampler2D spec_map;
uniform sampler2D env_map;
uniform sampler2D shadow_map;

varying vec2 uvs;
varying vec4 world_position;
varying mat4 TBN;
varying float depth;

/* Headers */

float shadow_amount_soft_pcf25(vec4 light_pos, sampler2D light_depth, float hardness);

vec3 from_gamma(vec3 color);
vec3 swap_red_green_inv(vec3 color);

vec3 apply_fog_blue(vec3 pixel, vec3 position, vec3 camera_position);

/* End */

void main() {
  
  vec4 light_pos = light_proj * light_view * world_position;
  
  float shadow = 1.0;
  if (recieve_shadows == 1) {
    shadow = shadow_amount_soft_pcf25(light_pos, shadow_map, 0.0005);
  }

  vec4 diffuse_a = texture2D( diffuse_map, uvs );
  
  if (diffuse_a.a < alpha_test) {
    discard;
  }
  
  vec3 albedo = from_gamma(diffuse_a.rgb);
  vec3 spec = texture2D( spec_map, uvs ).rgb;
  
  vec3 bump = texture2D( bump_map, uvs ).rgb;
  bump = swap_red_green_inv(bump);
  bump = mix( bump, vec3( 0.5, 0.5, 1.0 ), bumpiness );
  bump = normalize( ( bump * 2.0 ) - 1.0 );
  
  vec4 world_bump = TBN * vec4(bump,1.0);
  
  vec3 normal = world_bump.xyz / world_bump.w;
  vec3 position = world_position.xyz / world_position.w;
  
  vec3 camera_vector = normalize(camera_position - position);
  
  vec3 diffuse = vec3(0.0,0.0,0.0);
  vec3 ambient = vec3(0.0,0.0,0.0);
  vec3 specular = vec3(0.0,0.0,0.0);
  
  for(int i = 0; i < num_lights; i++) {
    vec3 light_vector = light_position[i] - position;
    float power = light_power[i] / pow(length(light_vector), light_falloff[i]);
    
    vec3 light_dir = normalize(light_position[i] - light_target[i]);
    vec3 half_vector = normalize(light_dir + camera_vector); 
    
    float n_dot_l = max( dot( normal, light_dir) , 0.0);
    float n_dot_h = specular_level * pow(max(dot(normal, half_vector),0.0), glossiness);
    
    if (i == 0) {
      n_dot_l *= shadow;
      n_dot_h *= shadow;
    }
    
    diffuse += power * light_diffuse[i] * albedo * n_dot_l;
    ambient += power * light_ambient[i] * albedo;
    specular += power * light_specular[i] * spec * n_dot_h;
  }
  
  vec3 reflected = normalize(reflect(camera_vector, normal));
  vec3 env = from_gamma(texture2D(env_map, reflected.xy).rgb) * ambient;
  float env_amount = (1.0 - dot(camera_vector, normal)) * spec.r * env_amount;
  diffuse = mix(diffuse, env, env_amount);
  
  vec3 final = diffuse + ambient + specular;
  //final = apply_fog_blue(final, position, camera_position);
  
  gl_FragColor.rgb = final;
  gl_FragColor.a = diffuse_a.a;
}
