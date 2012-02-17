uniform vec3 camera_position;
uniform vec3 light_position;

uniform mat4 light_view;
uniform mat4 light_proj;

uniform sampler2D env_map;
uniform sampler2D shadow_map;

varying vec4 position;
varying vec3 normal;
varying vec2 uvs;

/* Headers */

float shadow_amount_pcf25(vec4 light_pos, sampler2D light_depth, float kernel);

/* End */

void main() {
  
  vec4 light_pos = light_proj * light_view * position;
  float shadow = shadow_amount_pcf25(light_pos, shadow_map, 0.005);
  shadow = clamp(shadow, 0.25, 0.75);
  
  vec3 normal = normalize(normal);
  
  vec3 light_dir = normalize(light_position - position.xyz);
  vec3 camera_dir = normalize(camera_position - position.xyz);
  
  vec3 reflected = normalize(reflect(camera_dir, normal));
  vec3 env = texture2D(env_map, reflected.xy).rgb;
  float env_amount = (1.0 - dot(camera_dir, normal)) * 0.75;
  
  vec3 color = mix( (normal + 1)/2, env, env_amount);
  
  vec3 half_vector = normalize(light_dir + camera_dir); 
  float n_dot_h = max( dot( normal, half_vector ) , 0.0);
  vec3 spec = 1.0 * pow( n_dot_h, 75 );
  
  float light = clamp(dot(light_dir, normal) + 1.25, 0.0, 1.0);
  
  gl_FragColor = vec4(shadow * light * color + shadow * spec, 1.0);
}
