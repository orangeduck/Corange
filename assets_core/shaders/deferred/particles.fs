#version 120

uniform sampler2D depth;
uniform sampler2D particle_diffuse;
uniform sampler2D particle_normals;

uniform float particle_depth;
uniform float particle_thickness;
uniform float particle_bumpiness;
uniform float particle_scattering;

uniform float clip_near;
uniform float clip_far;

uniform float light_power;
uniform vec3 light_direction;
uniform vec3 light_diffuse;
uniform vec3 light_ambient;

varying vec2 fTexcoord;
varying vec4 fScreen;
varying vec4 fColor;
varying mat4 fTBN;

float linear_depth(float depth, float near, float far){
  return (2.0 * near) / (far + near - depth * (far - near));
}

vec3 swap_red_green_inv(vec3 color) {
  float temp = 1.0-color.r;
  color.r = 1.0-color.g;
  color.g = temp;
  return color;
}

void main() {
  
  vec4 albedo  = texture2D(particle_diffuse, fTexcoord) * fColor;
  vec4 normals = texture2D(particle_normals, fTexcoord);
  
  vec3 screenpos = (fScreen.xyz / fScreen.w) / 2.0 + 0.5;
  
  float depth_pix = linear_depth(screenpos.z, clip_near, clip_far);
  float depth_cur = texture2D(depth, screenpos.xy).r;
  float depth_add = particle_depth * 0.01 * (normals.a - 0.5);
  
  albedo.a = albedo.a * clamp( particle_thickness * 500 * (depth_cur - (depth_pix + depth_add)), 0, 1);
  
  normals.xyz = swap_red_green_inv(normals.xyz);
  normals.xyz = mix(normals.xyz, vec3(0.5, 0.5, 1.0), particle_bumpiness);
  
  vec4 normal = vec4(normals.xyz * 2.0 - 1.0, 1);
  normal = normal * fTBN;
  normal = normal / normal.w;
  
  float n_dot_l = max(dot(normal.xyz, -light_direction), particle_scattering);
  
  vec3 ambient = 3 * light_power * light_ambient * albedo.rgb;
  vec3 diffuse = 4 * light_power * light_diffuse * albedo.rgb * n_dot_l;
  
	gl_FragColor.rgb = ambient + diffuse;
	gl_FragColor.a = albedo.a;
  
}
