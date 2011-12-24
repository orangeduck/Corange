#version 120

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
uniform sampler2D bump_map;
uniform sampler2D spec_map;
uniform sampler2D env_map;
uniform sampler2D shadow_map;

uniform sampler3D lut;

varying vec2 uvs;
varying vec4 world_position;
varying mat4 TBN;
varying float depth;

const float shadow_bias = 0.0002;

float shadow_amount_soft_pcf25(vec4 light_pos, sampler2D light_depth, float hardness) {
  return 1.0;
}

vec3 to_gamma(vec3 color) {
  vec3 ret;
  ret.r = pow(color.r, 2.2);
  ret.g = pow(color.g, 2.2);
  ret.b = pow(color.b, 2.2);
	return ret;
}

vec3 from_gamma(vec3 color) {
  vec3 ret;
  ret.r = pow(color.r, 1.0/2.2);
  ret.g = pow(color.g, 1.0/2.2);
  ret.b = pow(color.b, 1.0/2.2);
	return ret;
}

vec3 swap_red_green(vec3 color) {
  float temp = color.r;
  color.r = color.g;
  color.g = temp;
  return color;
}

vec3 color_correction(vec3 color, sampler3D lut, int lut_size) {

  float scale = (float(lut_size) - 1.0) / float(lut_size);
  float offset = 1.0 / (2.0 * float(lut_size));

  return texture3D(lut, clamp(color, 0.0, 1.0) * scale + offset).rgb;
}

void main() {
  
  vec4 light_pos = light_proj * light_view * world_position;
  float shadow = shadow_amount_soft_pcf25(light_pos, shadow_map, 0.0005);

  vec4 diffuse_a = texture2D( diffuse_map, uvs );
  vec3 diffuse = from_gamma(diffuse_a.rgb);
  
  vec3 bump = texture2D( bump_map, uvs ).rgb;
  vec3 spec = from_gamma(texture2D( spec_map, uvs ).rgb);
  
  bump = mix( bump, vec3( 0.5, 0.5, 1.0 ), bumpiness );
  bump = normalize( ( bump * 2.0 ) - 1.0 );
  
  bump = swap_red_green(bump);
  
  vec4 world_bump = TBN * vec4(bump,1.0);
  
  vec3 normal = world_bump.xyz / world_bump.w;
  vec3 position = world_position.xyz / world_position.w;
  
  vec3 light_vector = normalize(light_position - position);
  vec3 eye_vector = normalize(eye_position - position);
  vec3 half_vector = normalize(light_vector + eye_vector); 
  
  float n_dot_l = max( dot( normal, light_vector) , 0.0);
  float n_dot_h = max( dot( normal, half_vector ) , 0.0);
  
  vec3 reflected = normalize(reflect(eye_vector, normal));
  vec3 env = from_gamma(texture2D(env_map, reflected.xy).rgb * 0.5);
  float env_amount = (1.0 - dot(eye_vector, normal)) * spec.r * env_amount;
  
  vec3 final_diffuse = (diffuse * diffuse_light * shadow * n_dot_l);
  final_diffuse = mix(final_diffuse, env, env_amount);
  vec3 final_ambient = ambient_light * diffuse;
  vec3 final_spec = (spec * specular_light * shadow * pow( n_dot_h, glossiness ) * specular_level );
  
  vec3 final = to_gamma(final_diffuse + final_ambient + final_spec);
  
  gl_FragColor.rgb = color_correction(final, lut, 64);
  gl_FragColor.a = diffuse_a.a;

}