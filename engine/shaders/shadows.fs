#version 120

/* Prototypes */

float shadow_amount(vec4 light_pos, sampler2D light_depth);
float shadow_amount_pcf4(vec4 light_pos, sampler2D light_depth, float kernel);
float shadow_amount_pcf9(vec4 light_pos, sampler2D light_depth, float kernel);
float shadow_amount_pcf16(vec4 light_pos, sampler2D light_depth, float kernel);
float shadow_amount_pcf25(vec4 light_pos, sampler2D light_depth, float kernel);
float shadow_amount_soft_pcf25(vec4 light_pos, sampler2D light_depth, float hardness);

/* End */


float shadow_amount(vec4 light_pos, sampler2D light_depth) {

  float shadow = 1.0;
  
  light_pos = vec4(light_pos.xyz / light_pos.w, 1);
  
  if ((abs(light_pos.x) > 1) || (abs(light_pos.y) > 1) || (abs(light_pos.z) > 1)) {
    return 1.0;
  }
  
  vec4 shadow_coord = light_pos / 2.0 + 0.5;
  float our_depth = shadow_coord.z;
  float shadow_depth = texture2D( light_depth, shadow_coord.xy ).r;
  
  if (our_depth >= shadow_depth) {
    shadow = 0.1;
  }
  
  return shadow;

}

float shadow_amount_pcf4(vec4 light_pos, sampler2D light_depth, float kernel) {

  float shadow = 1.0;
  
  light_pos = vec4(light_pos.xyz / light_pos.w, 1);
  
  if ((abs(light_pos.x) > 1) || (abs(light_pos.y) > 1) || (abs(light_pos.z) > 1)) {
    return 1.0;
  }
  
  vec4 shadow_coord = light_pos / 2.0 + 0.5;
  float our_depth = shadow_coord.z;
  
  vec2 samples[4] = vec2[4]( vec2(kernel, kernel),
                      vec2(kernel, -kernel),
                      vec2(-kernel, kernel),
                      vec2(-kernel, -kernel) );
  
  for(int i = 0; i < 4; i++) {
    vec2 offset = samples[i];
    float shadow_depth = texture2D( light_depth, shadow_coord.xy + offset ).r;
    
    if (our_depth >= shadow_depth) {
      shadow = shadow - 0.20;
    }
  }
  
  return shadow;
}

float shadow_amount_pcf9(vec4 light_pos, sampler2D light_depth, float kernel) {

  float shadow = 1.0;
  
  light_pos = vec4(light_pos.xyz / light_pos.w, 1);
  
  if ((abs(light_pos.x) > 1) || (abs(light_pos.y) > 1) || (abs(light_pos.z) > 1)) {
    return 1.0;
  }
  
  vec4 shadow_coord = light_pos / 2.0 + 0.5;
  float our_depth = shadow_coord.z;
  
  vec2 samples[9] = vec2[9]( vec2(-kernel, -kernel),
                      vec2(-kernel,  0.0),
                      vec2(-kernel,  kernel),
                      vec2( kernel, -kernel),
                      vec2( kernel,  0.0),
                      vec2( kernel,  kernel),
                      vec2( 0.0   , -kernel),
                      vec2( 0.0   ,  0.0),
                      vec2( 0.0   ,  kernel) );
  
  for(int i = 0; i < 9; i++) {
    vec2 offset = samples[i];
    float shadow_depth = texture2D( light_depth, shadow_coord.xy + offset ).r;
    
    if (our_depth >= shadow_depth) {
      shadow = shadow - 0.1;
    }
  }
  
  return shadow;
}

float shadow_amount_pcf16(vec4 light_pos, sampler2D light_depth, float kernel) {

  float shadow = 1.0;
  
  light_pos = vec4(light_pos.xyz / light_pos.w, 1);
  
  if ((abs(light_pos.x) > 1) || (abs(light_pos.y) > 1) || (abs(light_pos.z) > 1)) {
    return 1.0;
  }
  
  vec4 shadow_coord = light_pos / 2.0 + 0.5;
  float our_depth = shadow_coord.z;
  
  vec2 samples[16] = vec2[16]( vec2(-2.0*kernel, -2.0*kernel),
                       vec2(-2.0*kernel,  -kernel),
                       vec2(-2.0*kernel,  kernel),
                       vec2(-2.0*kernel,  2.0*kernel),
                      
                       vec2(-kernel, -2.0*kernel),
                       vec2(-kernel,  -kernel),
                       vec2(-kernel,  kernel),
                       vec2(-kernel,  2.0*kernel),
                      
                       vec2( kernel,  -2.0*kernel),
                       vec2( kernel,  -kernel),
                       vec2( kernel,  kernel),
                       vec2( kernel,  2.0*kernel),
                      
                       vec2( 2.0*kernel,  -2.0*kernel),
                       vec2( 2.0*kernel,  -kernel),
                       vec2( 2.0*kernel,  kernel),
                       vec2( 2.0*kernel,   2.0*kernel)
                      );
  
  for(int i = 0; i < 16; i++) {
    vec2 offset = samples[i];
    float shadow_depth = texture2D( light_depth, shadow_coord.xy + offset ).r;
    
    if (our_depth >= shadow_depth) {
      shadow = shadow - (1.0 / 16.0);
    }
  }
  
  return shadow;
}

float shadow_amount_pcf25(vec4 light_pos, sampler2D light_depth, float kernel) {

  float shadow = 1.0;
  
  light_pos = vec4(light_pos.xyz / light_pos.w, 1);
  
  if ((abs(light_pos.x) > 1) || (abs(light_pos.y) > 1) || (abs(light_pos.z) > 1)) {
    return 1.0;
  }
  
  vec4 shadow_coord = light_pos / 2.0 + 0.5;
  float our_depth = shadow_coord.z;
  
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
  
  for(int i = 0; i < 25; i++) {
    vec2 offset = samples[i];
    float shadow_depth = texture2D( light_depth, shadow_coord.xy + offset ).r;
    
    if (our_depth >= shadow_depth) {
      shadow = shadow - (1.0 / 25.0);
    }
  }
  
  return shadow;
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
  
  float prenumbra = max((our_depth - blocked_depth) * hardness * 0.25,0) / blocked_depth;
  
  float kernel = prenumbra * 1000 + 0.00025;
  
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
    
    if (our_depth >= shadow_depth) {
      shadow = shadow - (1.0 / 25.0);
    }
  }
  
  //return kernel * 100;
  return shadow;
}