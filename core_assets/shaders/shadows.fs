#version 120

/* Prototypes */

float shadow_amount(vec3 position, mat4 light_view, mat4 light_proj, sampler2D light_depth, int samples, float kernel, vec2 seed);
float shadow_amount_soft_pcf25(vec4 light_pos, sampler2D light_depth, float hardness);

/* End */

float linear_depth(float depth, float near, float far){
  return (2.0 * near) / (far + near - depth * (far - near));
}

const vec3 shadow_sample_sphere[32] = vec3[32](
  vec3(-0.00,  0.02, -0.03), vec3( 0.35, -0.04,  0.31), vec3( 0.66, -0.32,  0.53), 
  vec3(-0.04, -0.04,  0.01), vec3( 0.24, -0.22,  0.89), vec3(-0.09,  0.10, -0.54), 
  vec3( 0.24,  0.04,  0.01), vec3( 0.37,  0.88,  0.05), vec3( 0.02,  0.11, -0.19), 
  vec3(-0.04,  0.83, -0.01), vec3( 0.33,  0.11, -0.44), vec3( 0.21, -0.17,  0.28), 
  vec3( 0.48, -0.30,  0.34), vec3( 0.39, -0.72,  0.43), vec3( 0.19,  0.20,  0.03), 
  vec3( 0.35, -0.04, -0.01), vec3(-0.00, -0.02, -0.25), vec3(-0.07,  0.12, -0.04), 
  vec3( 0.00,  0.01, -0.40), vec3(-0.27,  0.41, -0.44), vec3( 0.13,  0.26, -0.14), 
  vec3( 0.15,  0.19, -0.26), vec3(-0.32,  0.29,  0.56), vec3(-0.00, -0.00,  0.13), 
  vec3(-0.36, -0.18,  0.07), vec3( 0.70,  0.21,  0.39), vec3(-0.36,  0.17,  0.91), 
  vec3(-0.11, -0.12,  0.26), vec3(-0.59, -0.67,  0.14), vec3(-0.24, -0.75,  0.27), 
  vec3( 0.18,  0.04, -0.58), vec3(-0.16,  0.11, -0.26));

const float shadow_bias = 0.001;

float shadow_amount(vec3 position, mat4 light_view, mat4 light_proj, sampler2D light_depth, int samples, float kernel, vec2 seed) {
 
  vec4 light_pos = light_proj * light_view * vec4(position, 1.0);
  light_pos = light_pos / light_pos.w;
  
  float pixel_depth = light_pos.z / 2 + 0.5;
  vec2  pixel_coords = vec2(light_pos.x, light_pos.y) / 2.0 + 0.5;
  
  float shade = 1.0;
  float k = 0.0025;
  
  for (int i = 0; i < 8; i++) {
    float shadow_depth = texture2D( light_depth, pixel_coords + shadow_sample_sphere[i].xy * k ).r;
    shade = shade - sign(pixel_depth - shadow_depth - shadow_bias) * 0.12;
  }
  
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
    
    if (our_depth >= shadow_depth + shadow_bias) {
      shadow = shadow - (1.0 / 25.0);
    }
  }
  
  return shadow;
}