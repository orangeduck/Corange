#version 120

/* Prototypes */

float ssao_depth(vec2 texcoords, sampler2D depth_texture, sampler2D random_texture);

/* End */


vec3 normal_from_depth(sampler2D depth_texture, vec2 texcoords) {

  float depth = texture2D(depth_texture, texcoords).r;

  vec2 offset1 = vec2(0.0,0.001);
  vec2 offset2 = vec2(0.001,0.0);
  
  float depth1 = texture2D(depth_texture, texcoords + offset1).r;
  float depth2 = texture2D(depth_texture, texcoords + offset2).r;
  
  vec3 p1 = vec3(offset1, depth1 - depth);
  vec3 p2 = vec3(offset2, depth2 - depth);
  
  vec3 normal = cross(p1, p2);
  normal.z = -normal.z;
  
  return normalize(normal);
  
}

vec3 sample_sphere[16] = vec3[16](vec3(0.53812504, 0.18565957, -0.43192),vec3(0.13790712, 0.24864247, 0.44301823),vec3(0.33715037, 0.56794053, -0.005789503),vec3(-0.6999805, -0.04511441, -0.0019965635),
                          vec3(0.06896307, -0.15983082, -0.85477847),vec3(0.056099437, 0.006954967, -0.1843352),vec3(-0.014653638, 0.14027752, 0.0762037),vec3(0.010019933, -0.1924225, -0.034443386),
                          vec3(-0.35775623, -0.5301969, -0.43581226),vec3(-0.3169221, 0.106360726, 0.015860917),vec3(0.010350345, -0.58698344, 0.0046293875),vec3(-0.08972908, -0.49408212, 0.3287904),
                          vec3(0.7119986, -0.0154690035, -0.09183723),vec3(-0.053382345, 0.059675813, -0.5411899),vec3(0.035267662, -0.063188605, 0.54602677),vec3(-0.47761092, 0.2847911, -0.0271716));

float ssao_depth(vec2 texcoords, sampler2D depth_texture, sampler2D random_texture) {
  
  const float total_strength = 1.0;
  const float base = 0.2;
  
  const float area = 0.0075;
  const float falloff = 0.000001;
  
  const float radius = 0.05;
  
  const int samples = 16;
  
  vec3 random = normalize( texture2D(random_texture, texcoords * 4.0).rgb * 2.0 - 1.0 );
  
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