#version 120

/* Prototypes */

float ssao_depth(vec2 texcoords, sampler2D depth_texture, sampler2D random_texture, float seed);

/* End */


vec3 normal_from_depth(sampler2D depth_texture, vec2 texcoords) {

  float depth = texture2D(depth_texture, texcoords).r;

  vec2 offset1 = vec2(0.0,0.01);
  vec2 offset2 = vec2(0.01,0.0);
  
  float depth1 = texture2D(depth_texture, texcoords + offset1).r;
  float depth2 = texture2D(depth_texture, texcoords + offset2).r;
  
  vec3 p1 = vec3(offset1, depth1 - depth);
  vec3 p2 = vec3(offset2, depth2 - depth);
  
  vec3 normal = cross(p1, p2);
  normal.z = -normal.z;
  
  return normalize(normal);
  
}

vec3 sample_sphere[32] = vec3[32](vec3(-0.00, 0.02, -0.03), vec3(0.35, 0.04, 0.31), vec3(0.66, 0.32, 0.53), vec3(-0.04, 0.04, 0.01), vec3(0.24, 0.22, 0.89), vec3(-0.09, 0.10, -0.54), vec3(0.24, 0.04, 0.01), vec3(0.37, 0.88, 0.05), vec3(0.02, 0.11, -0.19), vec3(-0.04, 0.83, -0.01), vec3(0.33, 0.11, -0.44), vec3(0.21, 0.17, 0.28), vec3(0.48, 0.30, 0.34), vec3(0.39, 0.72, 0.43), vec3(0.19, 0.20, 0.03), vec3(0.35, 0.04, -0.01), vec3(-0.00, 0.02, -0.25), vec3(-0.07, 0.12, -0.04), vec3(0.00, 0.01, -0.40), vec3(-0.27, 0.41, -0.44), vec3(0.13, 0.26, -0.14), vec3(0.15, 0.19, -0.26), vec3(-0.32, 0.29, 0.56), vec3(-0.00, 0.00, 0.13), vec3(-0.36, 0.18, 0.07), vec3(0.70, 0.21, 0.39), vec3(-0.36, 0.17, 0.91), vec3(-0.11, 0.12, 0.26), vec3(-0.59, 0.67, 0.14), vec3(-0.24, 0.75, 0.27), vec3(0.18, 0.04, -0.58), vec3(-0.16, 0.11, -0.26));

                          
float ssao_depth(vec2 texcoords, sampler2D depth_texture, sampler2D random_texture, float seed) {
  
  const float total_strength = 0.75;
  const float base = 0.0;
  
  /* Area basically refers to how "detailed" the SSAO is. Higher the less detailed */
  const float area = 0.0005;
  
  /* Falloff must be less than area and erm, doesn't seem to do much */
  const float falloff = 0.00009;
  
  const float radius = 0.025;
  
  const int samples = 8;
  
  /* A Higher value will reduce noise but increase banding */
  const float noise_reduce = 100;
  
  vec3 random = normalize( texture2D(random_texture, noise_reduce * (texcoords + vec2(seed*1.21, -seed*1.311))).rgb * 2.0 - 1.0 );
  
  float depth = texture2D(depth_texture, texcoords).r;
 
  vec3 position = vec3(texcoords, depth);
  vec3 normal = normal_from_depth(depth_texture, texcoords);
  
  float radius_depth = radius/depth;
  float occlusion = 0.0;
  for(int i=0; i < samples; i++) {
  
    vec3 ray = radius_depth * reflect(sample_sphere[i], random);
    vec3 projected = position + sign(dot(ray,normal)) * ray;
    
    float occ_depth = texture2D(depth_texture, clamp(projected.xy,0.0,1.0)).r;
    float difference = depth - occ_depth;
    
    float step;
    if (difference >= falloff) { step = 1.0; } else { step = 0.0; }
    occlusion += step * smoothstep(falloff, area, difference);
  }
  
  float ao = 1.0 - total_strength * occlusion * (1.0 / float(samples));
  return clamp(ao + base, 0.0, 1.0);

}