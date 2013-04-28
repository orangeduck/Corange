#version 120

#define MAT_DISCARD 1.0
#define MAT_REFLECT_MINOR 2.0
#define MAT_REFLECT_MAJOR 3.0
#define MAT_REFLECT_NONE 4.0
#define MAT_FLAT 5.0
#define MAT_LEAF 6.0
#define MAT_SKIN 7.0
#define MAT_CLOTH 8.0
#define MAT_NONE -1.0

uniform sampler2D diffuse_texture;
uniform sampler2D positions_texture;
uniform sampler2D normals_texture;

uniform vec3 camera_position;

uniform float light_power;
uniform vec3  light_position;
uniform vec3  light_target;
uniform vec3  light_diffuse;
uniform vec3  light_ambient;
uniform vec3  light_specular;

varying vec2 fTexcoord;

void main() {

  vec4 position = texture2D(positions_texture, fTexcoord);
  vec4 color    = texture2D(diffuse_texture,   fTexcoord);
  vec4 normals  = texture2D(normals_texture,   fTexcoord);
	
  vec3  albedo = color.rgb;
  vec3  normal = normalize(normals.rgb);
  float spec   = color.a;
  float glossiness = normals.a;
  
  int material = int(position.a);
  if (material == MAT_DISCARD) { discard; }
  if (material == MAT_FLAT) { gl_FragColor.rgb = albedo; return; }
  
  vec3 eye_dir    = normalize(camera_position - position.xyz);
  vec3 light_dir  = normalize(light_position - light_target);
  vec3 light_half = normalize(light_dir + eye_dir);
  
  float n_dot_l = max(dot(normal, light_dir), 0.0);
  float n_dot_h = spec * pow(max(dot(normal, light_half), 0.0), glossiness);
  
  vec3 ambient  = light_power * light_ambient  * albedo;
  vec3 diffuse  = light_power * light_diffuse  * albedo * n_dot_l;
  vec3 specular = light_power * light_specular * n_dot_h;
  
  gl_FragColor.rgb = diffuse + ambient + specular;

} 
