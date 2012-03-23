uniform vec3 camera_position;
uniform vec3 light_position;

uniform sampler2D stencil_texture;
uniform sampler2D depth_texture;
uniform sampler2D positions_texture;
uniform sampler2D normals_texture;
uniform sampler2D env_texture;

void main() {
  
  float depth = texture2D(depth_texture, gl_TexCoord[0].xy).a;
  
  if (depth == 1.0) {
	gl_FragColor = vec4(0.25, 0.25, 0.25, 0.25);
	return;
  }
  
  vec3 position = texture2D(positions_texture, gl_TexCoord[0].xy).xyz;
  vec3 normal = texture2D(normals_texture, gl_TexCoord[0].xy).xyz;
  
  vec3 light_dir = normalize(light_position - position);
  vec3 camera_dir = normalize(camera_position - position);
  
  vec3 reflected = normalize(reflect(camera_dir, normal));
  vec3 env = texture2D(env_texture, reflected.xy).rgb;
  float env_amount = (1.0 - dot(camera_dir, normal)) * 0.75;
  
  vec3 color = mix( (normal + 1)/2, env, env_amount);
  
  vec3 half_vector = normalize(light_dir + camera_dir); 
  float n_dot_h = max( dot( normal, half_vector ) , 0.0);
  vec3 spec = 5.0 * pow( n_dot_h, 75 );
  
  float light = clamp(dot(light_dir, normal) + 1.25, 0.0, 1.0);
  
  gl_FragColor = vec4(light * color + spec, 1.0);
  
}
