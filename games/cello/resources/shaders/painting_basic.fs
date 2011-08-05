uniform mat4 world_matrix;

uniform mat4 light_view;
uniform mat4 light_proj;

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
uniform sampler2D spec_map;
uniform sampler2D env_map;
uniform sampler2D shadow_map;
uniform sampler2D paint_map;

varying vec2 uvs;
varying vec4 world_position;
varying vec4 world_normal;

void main() {

  // SHADOW MAPPING
  
  float shadow = 1.0;
  
  vec4 light_pos = light_proj * light_view * world_position;
  
  vec4 shadow_coord = (light_pos / light_pos.w) / 2.0 + 0.5;
  
  float shadow_depth = texture2D( shadow_map, shadow_coord.xy ).r;
  float our_depth = shadow_coord.z;
  
  if (our_depth >= shadow_depth) {
	shadow = 0.1;
  }
  
  // END

  vec4 diffuse_a = texture2D( diffuse_map, uvs );
  vec3 diffuse = diffuse_a.rgb;
  
  vec3 spec = texture2D( spec_map, uvs ).rgb;
    
  vec3 normal = world_normal.xyz / world_normal.w;
  vec3 position = world_position.xyz / world_position.w;
  
  vec3 light_vector = normalize(position - light_position);
  vec3 eye_vector = normalize(position - eye_position);
  vec3 half_vector = normalize(light_vector + eye_vector); 
  
  float n_dot_l = max( -dot( normal, light_vector ) , 0.0f);
  float n_dot_h = max( -dot( normal, half_vector ) , 0.0f);
  
  vec3 reflected = normalize(reflect(eye_vector, normal));
  vec3 env = texture2D(env_map, reflected.xy).rgb;
  float env_amount = (1 - dot(eye_vector, normal)) * spec.r * env_amount;
  
  vec3 final_diffuse = (diffuse * diffuse_light * shadow * n_dot_l);
  final_diffuse = mix(final_diffuse, env, env_amount);
  vec3 final_ambient = ambient_light * diffuse;
  vec3 final_spec = (spec * specular_light * shadow * pow( n_dot_h, glossiness ) * specular_level );
  
  vec3 final = final_diffuse + final_ambient + final_spec;
  
  gl_FragData[0] = vec4(final , diffuse_a.a);

  vec4 paint = texture2D(paint_map, uvs);
  
  gl_FragData[1] = paint;
  
}