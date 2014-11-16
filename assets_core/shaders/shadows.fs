#version 120

/* Prototypes */

float shadow_amount(vec3 position, mat4 light_view, mat4 light_proj, sampler2D light_depth, float kernel, vec2 seed);
float shadow_amount_soft_pcf25(vec4 light_pos, sampler2D light_depth, float hardness);

/* End */

float linear_depth(float depth, float near, float far){
  return (2.0 * near) / (far + near - depth * (far - near));
}

float shadow_amount(vec3 position, mat4 light_view, mat4 light_proj, sampler2D light_depth, float kernel, vec2 seed) {
 
  vec4 light_pos = light_proj * light_view * vec4(position, 1.0);
  light_pos = light_pos / light_pos.w;
  
  float pixel_depth = light_pos.z / 2 + 0.5;
  vec2  pixel_coords = vec2(light_pos.x, light_pos.y) / 2.0 + 0.5;
  
  float shade = 1.0;
  
  vec2 offset0 = reflect(vec2(-0.00,  0.02), seed);
  vec2 offset1 = reflect(vec2( 0.35, -0.04), seed);
  vec2 offset2 = reflect(vec2( 0.66, -0.32), seed);
  vec2 offset3 = reflect(vec2(-0.04, -0.04), seed);
  vec2 offset4 = reflect(vec2( 0.24, -0.22), seed);
  vec2 offset5 = reflect(vec2(-0.09,  0.10), seed);
  vec2 offset6 = reflect(vec2( 0.24,  0.04), seed);
  vec2 offset7 = reflect(vec2( 0.37,  0.88), seed);
  
  shade -= sign(pixel_depth - texture2D(light_depth, pixel_coords + offset0 * kernel).r - 0.001) * (1.0 / 8.0);
  shade -= sign(pixel_depth - texture2D(light_depth, pixel_coords + offset1 * kernel).r - 0.001) * (1.0 / 8.0);
  shade -= sign(pixel_depth - texture2D(light_depth, pixel_coords + offset2 * kernel).r - 0.001) * (1.0 / 8.0);
  shade -= sign(pixel_depth - texture2D(light_depth, pixel_coords + offset3 * kernel).r - 0.001) * (1.0 / 8.0);
  shade -= sign(pixel_depth - texture2D(light_depth, pixel_coords + offset4 * kernel).r - 0.001) * (1.0 / 8.0);
  shade -= sign(pixel_depth - texture2D(light_depth, pixel_coords + offset5 * kernel).r - 0.001) * (1.0 / 8.0);
  shade -= sign(pixel_depth - texture2D(light_depth, pixel_coords + offset6 * kernel).r - 0.001) * (1.0 / 8.0);
  shade -= sign(pixel_depth - texture2D(light_depth, pixel_coords + offset7 * kernel).r - 0.001) * (1.0 / 8.0);
  
  return shade;
  
}

float shadow_amount_soft_pcf25(vec4 light_pos, sampler2D light_depth, float hardness) {
  
  light_pos = vec4(light_pos.xyz / light_pos.w, 1);
  
  if ((abs(light_pos.x) > 1) || (abs(light_pos.y) > 1) || (abs(light_pos.z) > 1)) {
    return 1.0;
  }
  
  vec4 shadow_coord = light_pos / 2.0 + 0.5;
  float our_depth = shadow_coord.z;

  float pixel_depth = texture2D( light_depth, shadow_coord.xy ).r;
  
  float dkernel = 0.01;
  vec2 disc_samples[25] = vec2[25]( vec2(-2.0*dkernel, -2.0*dkernel),
                       vec2(-2.0*dkernel,  -dkernel),
                       vec2(-2.0*dkernel,  dkernel),
                       vec2(-2.0*dkernel,  2.0*dkernel),
                       vec2(-2.0*dkernel,  0.0),
                      
                       vec2(-dkernel, -2.0*dkernel),
                       vec2(-dkernel,  -dkernel),
                       vec2(-dkernel,  dkernel),
                       vec2(-dkernel,  2.0*dkernel),
                       vec2(-dkernel,  0.0),

                       vec2( 0.0,  -2.0*dkernel),
                       vec2( 0.0,  -dkernel),
                       vec2( 0.0,  dkernel),
                       vec2( 0.0,  2.0*dkernel),
                       vec2( 0.0,  0.0),
                       
                       vec2( dkernel,  -2.0*dkernel),
                       vec2( dkernel,  -dkernel),
                       vec2( dkernel,  dkernel),
                       vec2( dkernel,  2.0*dkernel),
                       vec2( dkernel,  0.0),
                      
                       vec2( 2.0*dkernel,  -2.0*dkernel),
                       vec2( 2.0*dkernel,  -dkernel),
                       vec2( 2.0*dkernel,  dkernel),
                       vec2( 2.0*dkernel,   2.0*dkernel),
                       vec2( 2.0*dkernel,   0.0)
                      );

  float blocked_depth = 0.0;
  for(int i = 0; i < 25; i++) {
    vec2 offset = disc_samples[i];
    float sample_depth = texture2D( light_depth, shadow_coord.xy + offset ).r;
    blocked_depth += min(sample_depth, our_depth);
  }
  blocked_depth = blocked_depth / 25;
  
  float prenumbra = max((our_depth - blocked_depth) / blocked_depth, 0.0);
  
  float kernel = prenumbra * hardness * 3000.0 + 0.00025;
  
  vec2 samples[25] = vec2[25]( vec2(-2.0*kernel, -2.0*kernel),
                       vec2(-2.0*kernel,  -kernel),
                       vec2(-2.0*kernel,  kernel),
                       vec2(-2.0*kernel,  2.0*kernel),
                       vec2(-2.0*kernel,  0.0),
                      
                       vec2(-kernel, -2.0*kernel),
                       vec2(-kernel,  -kernel),
                       vec2(-kernel,  kernel),
                       vec2(-kernel,  2.0*kernel),
                       vec2(-kernel,  0.0),

                       vec2( 0.0,  -2.0*kernel),
                       vec2( 0.0,  -kernel),
                       vec2( 0.0,  kernel),
                       vec2( 0.0,  2.0*kernel),
                       vec2( 0.0,  0.0),
                       
                       vec2( kernel,  -2.0*kernel),
                       vec2( kernel,  -kernel),
                       vec2( kernel,  kernel),
                       vec2( kernel,  2.0*kernel),
                       vec2( kernel,  0.0),
                      
                       vec2( 2.0*kernel,  -2.0*kernel),
                       vec2( 2.0*kernel,  -kernel),
                       vec2( 2.0*kernel,  kernel),
                       vec2( 2.0*kernel,   2.0*kernel),
                       vec2( 2.0*kernel,   0.0)
                      );
  
  float shadow = 1.0;
  for(int i = 0; i < 25; i++) {
    vec2 offset = samples[i];
    float shadow_depth = texture2D( light_depth, shadow_coord.xy + offset ).r;
    
    if (our_depth >= shadow_depth + 0.001) {
      shadow = shadow - (1.0 / 25.0);
    }
  }
  
  return shadow;
}