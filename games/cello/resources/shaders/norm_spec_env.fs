uniform mat4 world_matrix;

uniform vec3 light_position;
uniform vec3 eye_position;

uniform vec3 diffuse_light;
uniform vec3 ambient_light;
uniform vec3 specular_light;
 
uniform float glossiness;
uniform float bumpiness;
uniform float env_amount;

uniform float specular_level;

uniform sampler2D diffuse_map;
uniform sampler2D bump_map;
uniform sampler2D spec_map;
uniform sampler2D env_map;

varying vec2 uvs;
varying vec3 world_position;
varying mat4 TBN;

void main() {

  vec4 diffuse_a = texture2D( diffuse_map, uvs );
  vec3 diffuse = diffuse_a.rgb;
  
  vec3 bump = texture2D( bump_map, uvs ).rgb;
  vec3 spec = texture2D( spec_map, uvs ).rgb;
  
  bump = mix( vec3( 0.5, 0.5, 1.0 ), bump, bumpiness );
  bump = normalize( ( bump * 2.0 ) - 1.0 );
  
  vec3 world_bump = (vec4(bump,1.0) * TBN * world_matrix).xyz;
  
  vec3 light_vector = -normalize(light_position - world_position);
  vec3 eye_vector = normalize(eye_position - world_position);
  vec3 half_vector = normalize(light_vector + eye_vector); 
  
  float n_dot_l = max(dot( world_bump, light_vector ) , 0.0f);
  float n_dot_h = max(dot( world_bump, half_vector ) , 0.0f);
  
  vec3 reflected = normalize(reflect(eye_vector, world_bump));
  vec3 env = texture2D(env_map, reflected.xy).rgb;
  float env_amount = (1 - dot(eye_vector, world_bump)) * spec.r * env_amount;
  
  vec3 final_diffuse = (diffuse * diffuse_light * n_dot_l);
  final_diffuse = mix(final_diffuse, env, env_amount);
  vec3 final_ambient = ambient_light * diffuse;
  vec3 final_spec = (spec * specular_light * pow( n_dot_h, glossiness ) * specular_level );
  
  vec3 final = final_diffuse + final_ambient + final_spec;
  
  gl_FragColor = vec4(final , diffuse_a.a);

}