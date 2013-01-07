#version 120

/* Headers */

vec3 fxaa_unsharp(sampler2D screen, vec2 pos, int width, int height);
vec3 fxaa(sampler2D screen, vec2 texcoord, int width, int height);

/* End */

/*
  Basic technique of my own invention. Very simple and fast. Odd effects for far away objects.
  
  First applies an unsharp mask. Then looks for areas of high color difference and blurs them.
  Works because it essentially maintains/sharpens the per-pixel detail in low-contrast areas.
  While bluring edges to remove any aliasing too intense.
*/
vec3 fxaa_unsharp(sampler2D screen, vec2 pos, int width, int height) {
  
  const float sharpen = 1.0;
  const float boundry = 0.2;
  
  float kernel = 1.0;
  
  float xoff = kernel / float(width);
  float yoff = kernel / float(height);

  vec3 rgb_ne = texture2D(screen, pos + vec2(-xoff,yoff)).rgb;
  vec3 rgb_n = texture2D(screen, pos + vec2(0.0,yoff)).rgb;
  vec3 rgb_nw = texture2D(screen, pos + vec2(xoff,yoff)).rgb;
  vec3 rgb_w = texture2D(screen, pos + vec2(xoff,0.0)).rgb;
  vec3 rgb_o = texture2D(screen, pos + vec2(0.0,0.0)).rgb;
  vec3 rgb_e = texture2D(screen, pos + vec2(-xoff,0.0)).rgb;
  vec3 rgb_sw = texture2D(screen, pos + vec2(-xoff,-yoff)).rgb;
  vec3 rgb_s = texture2D(screen, pos + vec2(0.0,-yoff)).rgb;
  vec3 rgb_se = texture2D(screen, pos + vec2(xoff,-yoff)).rgb;
  
  vec3 average = (rgb_ne + rgb_n + rgb_nw + rgb_w + rgb_e + rgb_sw + rgb_s + rgb_se) / 8.0;
  vec3 difference = rgb_o - average;
  
  rgb_o = rgb_o + (difference * sharpen);
  difference = rgb_o - average;
  
  float fdiff = abs(dot(vec3(1.0,1.0,1.0), difference));
  
  if (fdiff > boundry) {
    
    float alias_amount = clamp(fdiff * 2.0, 0.0, 0.75);
    //return mix(vec3(0,1,0), vec3(1,0,0), alias_amount);
    return mix(rgb_o, average, alias_amount);
    
  } else {
    return rgb_o;
  }
  
}

vec3 fxaa(sampler2D screen, vec2 texcoord, int width, int height) {
  
  const float span_max = 4.0;
  const float reduce_amount = 1.0 / 4.0;
  const float reduce_min = (1.0 / 64.0);
  
  vec2 pixel = 1.0 / vec2(width, height);

  vec3 rgb_nw = texture2D(screen, texcoord.xy + (vec2(-1.0, -1.0) * pixel)).rgb;
  vec3 rgb_ne = texture2D(screen, texcoord.xy + (vec2( 1.0, -1.0) * pixel)).rgb;
  vec3 rgb_sw = texture2D(screen, texcoord.xy + (vec2(-1.0,  1.0) * pixel)).rgb;
  vec3 rgb_se = texture2D(screen, texcoord.xy + (vec2( 1.0,  1.0) * pixel)).rgb;
  vec3 rgb_m  = texture2D(screen, texcoord.xy).rgb;
	
  vec3 luma = vec3(0.299, 0.587, 0.114);
  float luma_nw = dot(rgb_nw, luma);
  float luma_ne = dot(rgb_ne, luma);
  float luma_sw = dot(rgb_sw, luma);
  float luma_se = dot(rgb_se, luma);
  float luma_m  = dot( rgb_m, luma);
	
  float luma_min = min(luma_m, min(min(luma_nw, luma_ne), min(luma_sw, luma_se)));
  float luma_max = max(luma_m, max(max(luma_nw, luma_ne), max(luma_sw, luma_se)));
	
  vec2 dir;
  dir.x = -((luma_nw + luma_ne) - (luma_sw + luma_se));
  dir.y =  ((luma_nw + luma_sw) - (luma_ne + luma_se));
	
  float dir_reduce = max((luma_nw + luma_ne + luma_sw + luma_se) * (0.25 * reduce_amount), reduce_min);
  float dir_rcp_min = 1.0/(min(abs(dir.x), abs(dir.y)) + dir_reduce);
	
  dir = min(vec2(span_max,  span_max), max(vec2(-span_max, -span_max), dir * dir_rcp_min)) * pixel;

  vec3 rgba0 = texture2D(screen, texcoord.xy + dir * (1.0 / 3.0 - 0.5)).rgb;
  vec3 rgba1 = texture2D(screen, texcoord.xy + dir * (2.0 / 3.0 - 0.5)).rgb;
  vec3 rgba2 = texture2D(screen, texcoord.xy + dir * (0.0 / 3.0 - 0.5)).rgb;
  vec3 rgba3 = texture2D(screen, texcoord.xy + dir * (3.0 / 3.0 - 0.5)).rgb;
  
  vec3 rgb_a = (1.0/ 2.0) * (rgba0 + rgba1);
  vec3 rgb_b = rgb_a * (1.0/ 2.0) + (1.0/ 4.0) * (rgba2 + rgba3);
  
  float luma_b = dot(rgb_b, luma);
  
  if((luma_b < luma_min) || (luma_b > luma_max)){
    return rgb_a;
  } else {
    return rgb_b;
  }
  
}
