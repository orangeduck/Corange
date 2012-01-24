uniform vec3 camera_position;
uniform vec3 light_position;

uniform sampler2D env_map;

varying vec3 position;
varying vec3 normal;
varying vec2 uvs;

void main() {
  
  normal = normalize(normal);
  
  vec3 light_dir = normalize(light_position - position);
  vec3 camera_dir = normalize(camera_position - position);
  
  vec3 reflected = normalize(reflect(camera_dir, normal));
  vec3 env = texture2D(env_map, reflected.xy).rgb;
  float env_amount = (1.0 - dot(camera_dir, normal)) * 0.75;
  
  vec3 color = mix( (normal + 1)/2, env, env_amount);
  
  vec3 half_vector = normalize(light_dir + camera_dir); 
  float n_dot_h = max( dot( normal, half_vector ) , 0.0);
  vec3 spec = 1.0 * pow( n_dot_h, 75 );
  
  float light = clamp(dot(light_dir, normal) + 1.25, 0.0, 1.0);
  
  gl_FragColor = vec4(color * light + spec, 1.0);
}
