
uniform vec3 diffuse_light;
uniform vec3 ambient_light;
uniform vec3 specular_light;
 
uniform float glossiness;
uniform float bumpiness;

uniform float specular_level;

uniform sampler2D diffuse_map;
uniform sampler2D bump_map;
uniform sampler2D spec_map;

varying vec4 light_vector; 
varying vec4 half_angle;
varying vec2 uvs;

void main() {

  vec4 diffuse_a = texture2D( diffuse_map, uvs );
  vec3 diffuse = diffuse_a.rgb;
  
  vec3 bump = texture2D( bump_map, uvs ).rgb;
  vec3 spec = texture2D( spec_map, uvs ).rgb;
  
  bump = mix( vec3( 0.5, 0.5, 1.0 ), bump, bumpiness );
  bump = normalize( ( bump * 2.0 ) - 1.0 );
  
  vec3 normalized_light_vector = normalize(light_vector.xyz / half_angle.w);
  vec3 normalized_half_angle = normalize(half_angle.xyz / half_angle.w);
  
  float n_dot_l = max(dot( bump, normalized_light_vector ) , 0.0);
  float n_dot_h = max(dot( bump, normalized_half_angle ) , 0.0);
  
  vec3 final_diffuse = (diffuse * diffuse_light * n_dot_l);
  vec3 final_ambient = (diffuse * ambient_light);
  vec3 final_spec = (spec * specular_light * pow( n_dot_h, glossiness ) * specular_level );
  
  vec3 final = final_diffuse + final_ambient + final_spec;
  
  gl_FragColor = vec4(final , diffuse_a.a);

}