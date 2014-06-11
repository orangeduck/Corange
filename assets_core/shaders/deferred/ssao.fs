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

#define SAMPLE_SPHERE vec3[6]( \
    vec3(-0.00,  0.02, -0.03), vec3( 0.35, -0.04,  0.31), vec3( 0.66, -0.32,  0.53), \
    vec3(-0.04, -0.04,  0.01), vec3( 0.24, -0.22,  0.89), vec3(-0.09,  0.10, -0.54))

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
  
  float radius_depth = RADIUS/depth;
  float occlusion = 0.0;
  
  for (int i = 0; i < SAMPLES; i++) {
  
    vec3 ray = radius_depth * reflect(SAMPLE_SPHERE[i], random);
    vec3 projected = position + sign(dot(ray,normal)) * ray * vec3(width, height, 0);
    
    float occ_depth = texture2D(depth_texture, projected.xy).r;
    float difference = depth - occ_depth;
    
    occlusion += difference_occlusion(difference, clip_near, clip_far);
    
  }
  
  float ao = STRENGTH * occlusion * (1.0 / float(SAMPLES));
  float total = 1.0 - clamp(ao + BASE, 0.0, 1.0);
  
  gl_FragColor = vec4(total, total, total, 1);
	  
} 