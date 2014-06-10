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
  const float falloff = 5.0;
  
  float dist = (1.0/target) * abs(difference - target);
  if (difference > target) {
    dist *= (1.0/falloff);
  }
  
  dist = clamp(dist, 0.0, 1.0);
  
  return smoothstep_map(1.0-dist);
}

void main() {
	
  const float total_strength = 1.0;
  const float base = 0.0;
  const float radius = 0.000001;
  const int samples = 6;
  const float noise_tile = 10.0;
  
  const vec3 ao_sample_sphere[32] = vec3[32](
    vec3(-0.00,  0.02, -0.03), vec3( 0.35, -0.04,  0.31), vec3( 0.66, -0.32,  0.53), 
    vec3(-0.04, -0.04,  0.01), vec3( 0.24, -0.22,  0.89), vec3(-0.09,  0.10, -0.54), 
    vec3( 0.24,  0.04,  0.01), vec3( 0.37,  0.88,  0.05), vec3( 0.02,  0.11, -0.19), 
    vec3(-0.04,  0.83, -0.01), vec3( 0.33,  0.11, -0.44), vec3( 0.21, -0.17,  0.28), 
    vec3( 0.48, -0.30,  0.34), vec3( 0.39, -0.72,  0.43), vec3( 0.19, -0.20,  0.03), 
    vec3( 0.35, -0.04, -0.01), vec3(-0.00, -0.02, -0.25), vec3(-0.07,  0.12, -0.04), 
    vec3( 0.00,  0.01, -0.40), vec3(-0.27,  0.41, -0.44), vec3( 0.13,  0.26, -0.14), 
    vec3( 0.15,  0.19, -0.26), vec3(-0.32,  0.29,  0.56), vec3(-0.00, -0.00,  0.13), 
    vec3(-0.36, -0.18,  0.07), vec3( 0.70,  0.21,  0.39), vec3(-0.36,  0.17,  0.91), 
    vec3(-0.11, -0.12,  0.26), vec3(-0.59, -0.67,  0.14), vec3(-0.24, -0.75,  0.27), 
    vec3( 0.18,  0.04, -0.58), vec3(-0.16, -0.11, -0.26));
  
  vec3  pixel  = texture2D(positions_texture, fTexcoord).rgb;
  float depth  = texture2D(depth_texture, fTexcoord).r;
  vec3  normal = texture2D(normals_texture, fTexcoord).rgb;
  
  vec3 random = 
    abs(normal.x) * texture2D(random_texture, pixel.yz * noise_tile).rgb +
    abs(normal.y) * texture2D(random_texture, pixel.xz * noise_tile).rgb +
    abs(normal.z) * texture2D(random_texture, pixel.xy * noise_tile).rgb;
  random = normalize(random * 2.0 - 1.0);

  vec3 position = vec3(fTexcoord, depth);
  
  float radius_depth = radius/depth;
  float occlusion = 0.0;
  
  for(int i = 0; i < samples; i++) {
  
    vec3 ray = radius_depth * reflect(ao_sample_sphere[i], random);
    vec3 projected = position + sign(dot(ray,normal)) * ray * vec3(width, height, 0);
    
    float occ_depth = texture2D(depth_texture, projected.xy).r;
    float difference = depth - occ_depth;
    
    occlusion += difference_occlusion(difference, clip_near, clip_far);
  }
  
  float ao = total_strength * occlusion * (1.0 / float(samples));
  float total = 1.0 - clamp(ao + base, 0.0, 1.0);
  
  gl_FragColor = vec4(total, total, total, 1);
	  
} 