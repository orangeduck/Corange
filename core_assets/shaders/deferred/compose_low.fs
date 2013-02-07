#version 120

#define MAX_LIGHTS 32

uniform sampler2D diffuse_texture;
uniform sampler2D positions_texture;
uniform sampler2D normals_texture;
uniform sampler2D random_texture;
uniform sampler2D depth_texture;
uniform sampler2D shadows_texture0;
uniform sampler2D shadows_texture1;
uniform sampler2D shadows_texture2;
uniform sampler2D ssao_texture;
uniform sampler2D env_texture;

uniform vec3 camera_position;

uniform int   lights_num;
uniform int   light_shadows;
uniform float light_power[MAX_LIGHTS];
uniform float light_falloff[MAX_LIGHTS];
uniform vec3  light_position[MAX_LIGHTS];
uniform vec3  light_target[MAX_LIGHTS];
uniform vec3  light_diffuse[MAX_LIGHTS];
uniform vec3  light_ambient[MAX_LIGHTS];
uniform vec3  light_specular[MAX_LIGHTS];

uniform float light_start[3];
uniform mat4  light_view[3];
uniform mat4  light_proj[3];

varying vec2 fTexcoord;

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

void main() {

	vec4 position = texture2D(positions_texture, fTexcoord);
	vec4 color    = texture2D(diffuse_texture,   fTexcoord);
  vec4 normals  = texture2D(normals_texture,   fTexcoord);
  float depth   = texture2D(depth_texture,     fTexcoord).r;
	
  vec3  albedo = color.rgb;
	vec3  normal = normalize(normals.rgb);
  float spec   = color.a;
  float glossiness = normals.a;
  
  int material = int(position.a);
  if (material == 1) { discard; }
  if (material == 4) { gl_FragColor.rgb = albedo; return; }
  
  const float noise_tile = 1.0;
  vec2 random_coords = vec2(position.xz + position.yz + position.yx) * noise_tile;
  vec3 random = normalize( texture2D(random_texture, random_coords).rgb * 2.0 - 1.0 );
  
  vec3 ssao = texture2DLod(ssao_texture, fTexcoord, 1.0).rgb;
  vec3 eye_dir    = normalize(camera_position - position.xyz);
  vec3 light_dir  = normalize(light_position[0] - light_target[0]);
  vec3 light_half = normalize(light_dir + eye_dir);
  
  float n_dot_l = max(dot(normal, light_dir), 0.0);
  float n_dot_h = spec * pow(max(dot(normal, light_half),0.0), glossiness);
  
  vec3 ambient  = light_power[1] * light_ambient[1] * albedo * ssao;
  vec3 diffuse  = light_power[0] * light_diffuse[0] * albedo * n_dot_l;
  vec3 specular = light_power[0] * light_specular[0] * n_dot_h;
  
  gl_FragColor.rgb = diffuse + ambient + specular;

} 
