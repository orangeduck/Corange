#version 120

uniform int width;
uniform int height;

/* Prototypes */

float ssao_depth(vec2 texcoords, sampler2D depth_texture, sampler2D random_texture, float seed);
float ssao(vec2 texcoords, sampler2D depth_texture, sampler2D normals_texture, sampler2D random_texture, float seed);

/* End */

vec3 normal_from_depth(sampler2D depth_texture, vec2 texcoords) {

  float depth = texture2D(depth_texture, texcoords).r;

  vec2 offset1 = vec2(0.0, 2.0 / height);
  vec2 offset2 = vec2(2.0 / width, 0.0);
  
  float depth1 = texture2D(depth_texture, texcoords + offset1).r;
  float depth2 = texture2D(depth_texture, texcoords + offset2).r;
  
  vec3 p1 = vec3(offset1, depth1 - depth);
  vec3 p2 = vec3(offset2, depth2 - depth);
  
  vec3 normal = cross(p1, p2);
  normal.z = -normal.z;
  
  return normalize(normal);
  
}

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


float smoothstep_map(float x) {
  return x*x*(3.0 - 2.0*x);
}

float difference_occlusion(float difference) {
  
  difference = max(difference, 0.0);
  
  /* This is the depth difference at which the maximum occlusion happens */
  const float target = 0.00025;
  
  /* This is the length of the falloff after maximum depth difference is reached */
  const float falloff = 5.0;
  
  float dist = (1.0/target) * abs(difference - target);
  if (difference > target) {
    dist *= (1.0/falloff);
  }
  
  dist = clamp(dist, 0.0, 1.0);
  
  return smoothstep_map(1.0-dist);
}

float ssao_depth(vec2 texcoords, sampler2D depth_texture, sampler2D random_texture, float seed) {
  
  const float total_strength = 1.0;
  const float base = 0.0;
  const float radius = 0.0000001;
  const int samples = 8;
  const float noise_tile = 0.05;
  
  vec2 random_coords = (texcoords + vec2(1.12 * seed, -1.09 * seed)) * vec2(width, height) * noise_tile;
  vec3 random = normalize( texture2D(random_texture, random_coords).rgb * 2.0 - 1.0 );
  
  float depth = texture2D(depth_texture, texcoords).r;
 
  vec3 position = vec3(texcoords, depth);
  vec3 normal = normal_from_depth(depth_texture, texcoords);
  
  float radius_depth = radius/depth;
  float occlusion = 0.0;
  for(int i = 0; i < samples; i++) {
  
    vec3 ray = radius_depth * reflect(ao_sample_sphere[i], random);
    vec2 projected = position.xy + sign(dot(ray,normal)) * ray.xy * vec2(width, height);
    
    float occ_depth = texture2D(depth_texture, projected).r;
    float difference = depth - occ_depth;
    
    occlusion += difference_occlusion(difference);
  }
  
  float ao = total_strength * occlusion * (1.0 / float(samples));
  return 1.0 - clamp(ao + base, 0.0, 1.0);
  
}

float ssao(vec2 texcoords, sampler2D depth_texture, sampler2D normals_texture, sampler2D random_texture, float seed) {
  
  const float total_strength = 1.0;
  const float base = 0.0;
  const float radius = 0.0000001;
  const int samples = 8;
  const float noise_tile = 0.1;
  
  vec2 random_coords = (texcoords + vec2(1.12 * seed, -1.09 * seed)) * vec2(width, height) * noise_tile;
  vec3 random = normalize( texture2D(random_texture, random_coords).rgb * 2.0 - 1.0 );
  
  float depth = texture2D(depth_texture, texcoords).r;
  vec3 normal = texture2D(normals_texture, texcoords).rgb;
  vec3 position = vec3(texcoords, depth);
  
  float radius_depth = radius/depth;
  float occlusion = 0.0;
  for(int i = 0; i < samples; i++) {
  
    vec3 ray = radius_depth * reflect(ao_sample_sphere[i], random);
    vec2 projected = position.xy + sign(dot(ray,normal)) * ray.xy * vec2(width, height);
    
    float occ_depth = texture2D(depth_texture, projected).r;
    float difference = depth - occ_depth;
    
    occlusion += difference_occlusion(difference);
  }
  
  float ao = total_strength * occlusion * (1.0 / float(samples));
  return 1.0 - clamp(ao + base, 0.0, 1.0);
  
}