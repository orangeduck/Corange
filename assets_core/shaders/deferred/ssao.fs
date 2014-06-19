#version 120

uniform sampler2D depth_texture;
uniform sampler2D normals_texture;
uniform sampler2D positions_texture;
uniform sampler2D random_texture;

uniform float seed;
uniform float clip_near;
uniform float clip_far;

uniform int width;
uniform int height;

varying vec2 fTexcoord;

float smoothstep_map(float x) {
  return x*x*(3.0 - 2.0*x);
}

float difference_occlusion(float difference, float clip_near, float clip_far) {
  
  difference = max(difference, 0.0);
  
  /* This is the depth difference at which the maximum occlusion happens */
  float target = 0.000005 * (clip_far - clip_near); 
  
  /* This is the length of the falloff after maximum depth difference is reached */
  float falloff = 5.0;
  
  float dist = (1.0/target) * abs(difference - target);
  if (difference > target) {
    dist *= (1.0/falloff);
  }
  
  dist = clamp(dist, 0.0, 1.0);
  
  return smoothstep_map(1.0-dist);
}

#define STRENGTH 1.0
#define BASE 0.0
#define RADIUS 0.000001
#define SAMPLES 6
#define TILE 10.0

void main() {
  
  vec3  pixel  = texture2D(positions_texture, fTexcoord).rgb;
  float depth  = texture2D(depth_texture, fTexcoord).r;
  vec3  normal = texture2D(normals_texture, fTexcoord).rgb;
  
  vec3 random = 
    abs(normal.x) * texture2D(random_texture, pixel.yz * TILE).rgb +
    abs(normal.y) * texture2D(random_texture, pixel.xz * TILE).rgb +
    abs(normal.z) * texture2D(random_texture, pixel.xy * TILE).rgb;
  random = normalize(random * 2.0 - 1.0);

  vec3 position = vec3(fTexcoord, depth);  
  float radius_depth = RADIUS / depth;
  
  vec3 ray0 = radius_depth * reflect(vec3(-0.00,  0.02, -0.03), random);
  vec3 ray1 = radius_depth * reflect(vec3( 0.35, -0.04,  0.31), random);
  vec3 ray2 = radius_depth * reflect(vec3( 0.66, -0.32,  0.53), random);
  vec3 ray3 = radius_depth * reflect(vec3(-0.04, -0.04,  0.01), random);
  vec3 ray4 = radius_depth * reflect(vec3( 0.24, -0.22,  0.89), random);
  vec3 ray5 = radius_depth * reflect(vec3(-0.09,  0.10, -0.54), random);
  
  vec3 projected0 = position + sign(dot(ray0, normal)) * ray0 * vec3(width, height, 0);
  vec3 projected1 = position + sign(dot(ray1, normal)) * ray1 * vec3(width, height, 0);
  vec3 projected2 = position + sign(dot(ray2, normal)) * ray2 * vec3(width, height, 0);
  vec3 projected3 = position + sign(dot(ray3, normal)) * ray3 * vec3(width, height, 0);
  vec3 projected4 = position + sign(dot(ray4, normal)) * ray4 * vec3(width, height, 0);
  vec3 projected5 = position + sign(dot(ray5, normal)) * ray5 * vec3(width, height, 0);

  float occlusion = 0.0;
  occlusion += difference_occlusion(depth - texture2D(depth_texture, projected0.xy).r, clip_near, clip_far);
  occlusion += difference_occlusion(depth - texture2D(depth_texture, projected1.xy).r, clip_near, clip_far);
  occlusion += difference_occlusion(depth - texture2D(depth_texture, projected2.xy).r, clip_near, clip_far);
  occlusion += difference_occlusion(depth - texture2D(depth_texture, projected3.xy).r, clip_near, clip_far);
  occlusion += difference_occlusion(depth - texture2D(depth_texture, projected4.xy).r, clip_near, clip_far);
  occlusion += difference_occlusion(depth - texture2D(depth_texture, projected5.xy).r, clip_near, clip_far);
  
  float ao = STRENGTH * occlusion * (1.0 / float(SAMPLES));
  float total = 1.0 - clamp(ao + BASE, 0.0, 1.0);
  
  gl_FragColor = vec4(total, total, total, 1);
	  
} 