#version 120

uniform sampler2D depth;
uniform sampler2D bump0;
uniform sampler2D bump1;
uniform sampler2D bump2;
uniform sampler2D bump3;
uniform samplerCube cube_sea;

uniform vec4 bump_factor;

uniform float light_power;
uniform vec3 light_direction;
uniform vec3 light_diffuse;
uniform vec3 light_ambient;
uniform vec3 light_specular;

uniform float clip_near;
uniform float clip_far;

uniform vec3 camera_direction;

varying vec2 fTexcoord;
varying vec3 fNormal;
varying vec4 fScreen;

float linear_depth(float depth, float near, float far) {
  return (2.0 * near) / (far + near - depth * (far - near));
}

void main() {
  
  vec3 screenpos = (fScreen.xyz / fScreen.w) / 2.0 + 0.5;
  
  float depth_pix = linear_depth(screenpos.z, clip_near, clip_far);
  float depth_cur = texture2D(depth, screenpos.xy).r;
  
  const float thickness = 300;
  float transparency = clamp( thickness * (depth_cur - depth_pix), 0, 1);
  
  if (transparency == 0) { discard; }
  
  vec4 bump0_norm = texture2D(bump0, fTexcoord);
  vec4 bump1_norm = texture2D(bump1, fTexcoord);
  vec4 bump2_norm = texture2D(bump2, fTexcoord);
  vec4 bump3_norm = texture2D(bump3, fTexcoord);
  
  vec3 bump = 
    (bump0_norm.xyz * bump_factor.r +
     bump1_norm.xyz * bump_factor.g +
     bump2_norm.xyz * bump_factor.b +
     bump3_norm.xyz * bump_factor.a);
  
  vec3 normal = normalize(fNormal + 0.1 * vec3(bump.x-0.5, 0, bump.y-0.5));
  
  vec3 light_half = normalize(light_direction + camera_direction);
  
  float n_dot_l =       max(dot(normal, light_direction), 0.0);
  float n_dot_h =   pow(max(dot(normal, light_half), 0.0), 1500);
	float n_dot_c = 1 - clamp(dot(normal, camera_direction), 0, 1);
  
  float fresnel = 0.0204 + 0.9796 * (n_dot_c * n_dot_c * n_dot_c * n_dot_c * n_dot_c);
  
	const vec3 albedo_sky  = 0.1 * vec3(1.0, 4.0, 06.0);
	const vec3 albedo_down = 0.1 * vec3(1.0, 4.0, 06.0);
	const vec3 albedo_up   = 0.1 * vec3(1.0, 5.0, 7.00);
	vec3 albedo = (0.1 * fresnel * albedo_sky) + mix( albedo_down, albedo_up, n_dot_c);
  
  vec3 env      = 1.0 * textureCube(cube_sea, reflect(camera_direction, normal)).rgb;
  vec3 ambient  = 2.0 * albedo * light_power * light_ambient;
  vec3 specular = 100 * light_power * light_specular * n_dot_h; 
  vec3 diffuse  = 0.5 * albedo * light_power * light_diffuse * n_dot_l;
  
  diffuse = mix(diffuse, env, clamp(n_dot_c, 1.0, 1.0));
  
	gl_FragColor.rgb = diffuse + ambient + specular;
	gl_FragColor.a = transparency;
  
}
