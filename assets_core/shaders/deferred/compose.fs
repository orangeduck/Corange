#version 120

#define MAX_LIGHTS 32

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
uniform sampler2D random_texture;
uniform sampler2D depth_texture;

uniform sampler2D shadows_texture0;
uniform sampler2D shadows_texture1;
uniform sampler2D shadows_texture2;
uniform sampler2D ssao_texture;
uniform samplerCube env_texture;
uniform sampler2D skin_lookup;

uniform mat4 inv_view;
uniform mat4 inv_proj;
uniform float clip_near;
uniform float clip_far;

uniform vec3 camera_position;

uniform int lights_num;
uniform float light_power[MAX_LIGHTS];
uniform float light_falloff[MAX_LIGHTS];
uniform vec3 light_position[MAX_LIGHTS];
uniform vec3 light_target[MAX_LIGHTS];
uniform vec3 light_diffuse[MAX_LIGHTS];
uniform vec3 light_ambient[MAX_LIGHTS];
uniform vec3 light_specular[MAX_LIGHTS];

uniform float light_start[3];
uniform mat4 light_view[3];
uniform mat4 light_proj[3];

varying vec2 fTexcoord;

#define SHADOW_BIAS 0.001
#define SHADOW_SAMPLE_SPHERE vec3[32]( \
    vec3(-0.00,  0.02, -0.03), vec3( 0.35, -0.04,  0.31), vec3( 0.66, -0.32,  0.53), \
    vec3(-0.04, -0.04,  0.01), vec3( 0.24, -0.22,  0.89), vec3(-0.09,  0.10, -0.54), \
    vec3( 0.24,  0.04,  0.01), vec3( 0.37,  0.88,  0.05), vec3( 0.02,  0.11, -0.19), \
    vec3(-0.04,  0.83, -0.01), vec3( 0.33,  0.11, -0.44), vec3( 0.21, -0.17,  0.28), \
    vec3( 0.48, -0.30,  0.34), vec3( 0.39, -0.72,  0.43), vec3( 0.19,  0.20,  0.03), \
    vec3( 0.35, -0.04, -0.01), vec3(-0.00, -0.02, -0.25), vec3(-0.07,  0.12, -0.04), \
    vec3( 0.00,  0.01, -0.40), vec3(-0.27,  0.41, -0.44), vec3( 0.13,  0.26, -0.14), \
    vec3( 0.15,  0.19, -0.26), vec3(-0.32,  0.29,  0.56), vec3(-0.00, -0.00,  0.13), \
    vec3(-0.36, -0.18,  0.07), vec3( 0.70,  0.21,  0.39), vec3(-0.36,  0.17,  0.91), \
    vec3(-0.11, -0.12,  0.26), vec3(-0.59, -0.67,  0.14), vec3(-0.24, -0.75,  0.27), \
    vec3( 0.18,  0.04, -0.58), vec3(-0.16,  0.11, -0.26))


float shadow_amount(vec3 position, mat4 light_view, mat4 light_proj, sampler2D light_depth, float kernel, vec2 seed) {
 
  vec4 light_pos = light_proj * light_view * vec4(position, 1.0);
  light_pos = light_pos / light_pos.w;
  
  float pixel_depth = light_pos.z / 2 + 0.5;
  vec2  pixel_coords = vec2(light_pos.x, light_pos.y) / 2.0 + 0.5;
  
  float shade = 1.0;  
  
  vec2 offset0 = reflect(SHADOW_SAMPLE_SPHERE[0].xy, seed);
  vec2 offset1 = reflect(SHADOW_SAMPLE_SPHERE[1].xy, seed);
  vec2 offset2 = reflect(SHADOW_SAMPLE_SPHERE[2].xy, seed);
  vec2 offset3 = reflect(SHADOW_SAMPLE_SPHERE[3].xy, seed);
  
  float shadow_depth0 = texture2D( light_depth, pixel_coords + offset0 * kernel ).r;
  float shadow_depth1 = texture2D( light_depth, pixel_coords + offset1 * kernel ).r;
  float shadow_depth2 = texture2D( light_depth, pixel_coords + offset2 * kernel ).r;
  float shadow_depth3 = texture2D( light_depth, pixel_coords + offset3 * kernel ).r;
  
  shade = shade - sign(pixel_depth - shadow_depth0 - SHADOW_BIAS) * (float(1) / float(4));
  shade = shade - sign(pixel_depth - shadow_depth1 - SHADOW_BIAS) * (float(1) / float(4));
  shade = shade - sign(pixel_depth - shadow_depth2 - SHADOW_BIAS) * (float(1) / float(4));
  shade = shade - sign(pixel_depth - shadow_depth3 - SHADOW_BIAS) * (float(1) / float(4));
  
  return shade;
  
}

float when_eq(float x, float y) {
  return 1.0 - abs(sign(x - y));
}

vec4 when_eq(vec4 x, vec4 y) {
  return 1.0 - abs(sign(x - y));
}

float when_neq(float x, float y) {
  return abs(sign(x - y));
}

vec3 from_gamma(vec3 color) {
  return vec3(
    pow(color.r, 1.0 / 2.2),
    pow(color.g, 1.0 / 2.2),
    pow(color.b, 1.0 / 2.2));
}

float perspective_depth(float depth, float near, float far) {
  return (((2.0 * near) / depth) - far - near) / (near - far);
}

void main() {

  float depth = texture2D(depth_texture, fTexcoord).r;
	vec3 position_clip = vec3(fTexcoord.xy, perspective_depth(depth, clip_near, clip_far)) * 2.0 - 1.0;
  vec4 position = inv_view * inv_proj * vec4(position_clip, 1);
  position = position / position.w;

	vec4 diffuse_a = texture2D(diffuse_texture, fTexcoord );
	vec3 diffuse_amount = diffuse_a.rgb;
  float spec_amount = diffuse_a.a;
  
  vec4 normals = texture2D(normals_texture, fTexcoord);
  
  vec3  normal = normalize(normals.rgb);
  float glossiness = mod(normals.a, 1.0) * 1000;
  int material = int(normals.a);
  
  float curvature = glossiness;
  
  if (material == MAT_DISCARD) { discard; }
  if (material == MAT_FLAT) { gl_FragColor.rgb = diffuse_amount; return; }
  
  float noise_tile = 1.0;
  vec3 random = 
    abs(normal.x) * texture2D(random_texture, position.yz * noise_tile).rgb +
    abs(normal.y) * texture2D(random_texture, position.xz * noise_tile).rgb +
    abs(normal.z) * texture2D(random_texture, position.xy * noise_tile).rgb;
  random = normalize(random * 2.0 - 1.0);
  
  float shadow0 = shadow_amount(position.xyz, light_view[0], light_proj[0], shadows_texture0, 0.00075, random.xy);
  float shadow1 = shadow_amount(position.xyz, light_view[1], light_proj[1], shadows_texture1, 0.00075, random.xy);
  float shadow2 = shadow_amount(position.xyz, light_view[2], light_proj[2], shadows_texture2, 0.00100, random.xy);
	float shadow = depth > light_start[2] ? shadow2 : (depth > light_start[1] ? shadow1 : shadow0);
  
  vec3 eye_dir = normalize(camera_position - position.xyz);
  float n_dot_c = dot(normal, eye_dir);
  
  vec3 ssao = texture2D(ssao_texture, fTexcoord).rgb;
  vec3 env = textureCube(env_texture, reflect(-eye_dir, normal)).rgb;
  
  vec4 materialsv = vec4(material, material, material, material);
  vec4 materials0 = vec4(MAT_REFLECT_MINOR, MAT_REFLECT_MAJOR, MAT_REFLECT_NONE, MAT_FLAT);
  vec4 materials1 = vec4(MAT_LEAF, MAT_SKIN, MAT_CLOTH, MAT_NONE);
  
  float reflect_glossiness = 1.0;

  float reflect_amount =
    dot(when_eq(materials0, materialsv), vec4(0.5, 1.5, 0.0, 0.0)) +
    dot(when_eq(materials1, materialsv), vec4(0.5, 0.5, 1.25, 0.0));
  
  float inner_rim_amount = 
    dot(when_eq(materials0, materialsv), vec4(0.1, 0.05, 0.0, 0.0)) +
    dot(when_eq(materials1, materialsv), vec4(0.1, 0.1, 0.25, 0.0));
  
  float outer_rim_amount = 
    dot(when_eq(materials0, materialsv), vec4(0.1, 0.25, 0.0, 0.0)) +
    dot(when_eq(materials1, materialsv), vec4(0.5, 0.1, 0.5, 0.0));
  
  float inner_rim_exp = 
    dot(when_eq(materials0, materialsv), vec4(20.0, 30.0, 1.0, 1.0)) +
    dot(when_eq(materials1, materialsv), vec4(40.0, 5.0, 30.0, 1.0));
  
  float outer_rim_exp =
    dot(when_eq(materials0, materialsv), vec4(10.0, 5.0, 1.0, 1.0)) +
    dot(when_eq(materials1, materialsv), vec4(20.0, 1.0, 1.0, 1.0));
  
  float is_skin = when_eq(material, MAT_SKIN);
  glossiness = is_skin * 15.0 + (1-is_skin) * glossiness;
  
  vec3 diffuse    = vec3(0.0, 0.0, 0.0);
  vec3 ambient    = vec3(0.0, 0.0, 0.0);
  vec3 specular   = vec3(0.0, 0.0, 0.0);
  vec3 reflection = vec3(0.0, 0.0, 0.0);
  vec3 inner_rim  = vec3(0.0, 0.0, 0.0);
  vec3 outer_rim  = vec3(0.0, 0.0, 0.0);
  
  for(int i = 0; i < lights_num; i++) {
    
    vec3 light_vector = light_position[i] - position.xyz;
    float power = light_power[i] / pow(length(light_vector), light_falloff[i]);
    
    vec3 light_dir = normalize(light_position[i] - light_target[i]);
    vec3 light_half = normalize(light_dir + eye_dir);
    
    float n_dot_l = dot(normal, light_dir);
    float n_dot_h = dot(normal, light_half);
    float n_dot_v = dot(normal, eye_dir);
    float v_dot_h = dot(eye_dir, light_half);
    
    vec3  light_diff = max(vec3(n_dot_l, n_dot_l, n_dot_l), 0.0);
    float light_spec = ((glossiness+2) / (8 * 3.141)) * max(pow(n_dot_h, glossiness), 0.0);
    float light_refl = ((reflect_glossiness+2) / (2 * 3.141)) * max(pow((1.0-n_dot_c), reflect_glossiness), 0.0);
    
    float light_inrim = pow(clamp(n_dot_v, 0, 1), inner_rim_exp);
    float light_outrim = pow(clamp(1-n_dot_v, 0, 1), outer_rim_exp);
   
    if (material == MAT_SKIN) {
      light_diff = texture2D( skin_lookup, clamp(vec2(n_dot_l * 0.5 + 0.5, curvature), 0.1, 0.9)).rgb;
    }
   
    light_diff *= clamp(shadow + i, 0, 1);
    light_spec *= clamp(shadow + i, 0, 1);
    light_inrim *= clamp(shadow + i, 0, 1);
    light_outrim *= clamp(shadow + i, 0, 1);
    
    ambient    += power * light_ambient[i];
    diffuse    += power * light_diffuse[i]  * light_diff;
    specular   += power * light_specular[i] * light_spec;
    reflection += power * light_ambient[i]  * light_refl;
    inner_rim  += power * light_specular[i] * light_inrim;
    outer_rim  += power * light_specular[i] * light_outrim;
    
  }
  
  ambient    = ambient    * diffuse_amount * ssao;
  diffuse    = diffuse    * diffuse_amount;
  specular   = specular   * spec_amount;
  reflection = reflection * spec_amount * reflect_amount * env;
  inner_rim  = inner_rim  * spec_amount * inner_rim_amount;
  outer_rim  = outer_rim  * spec_amount * outer_rim_amount;
  
  gl_FragColor.rgb = ambient + diffuse + specular + reflection + inner_rim + outer_rim; 

} 
