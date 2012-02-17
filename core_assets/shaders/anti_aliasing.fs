/* Headers */

vec3 fxaa_unsharp(sampler2D screen, vec2 pos, int width, int height);
vec3 fxaa(sampler2D screen, vec2 pos, int width, int height);

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
  vec3 rgb_n = texture2D(screen, pos + vec2(0,yoff)).rgb;
  vec3 rgb_nw = texture2D(screen, pos + vec2(xoff,yoff)).rgb;
  vec3 rgb_w = texture2D(screen, pos + vec2(xoff,0)).rgb;
  vec3 rgb_o = texture2D(screen, pos + vec2(0,0)).rgb;
  vec3 rgb_e = texture2D(screen, pos + vec2(-xoff,0)).rgb;
  vec3 rgb_sw = texture2D(screen, pos + vec2(-xoff,-yoff)).rgb;
  vec3 rgb_s = texture2D(screen, pos + vec2(0,-yoff)).rgb;
  vec3 rgb_se = texture2D(screen, pos + vec2(xoff,-yoff)).rgb;
  
  vec3 average = (rgb_ne + rgb_n + rgb_nw + rgb_w + rgb_e + rgb_sw + rgb_s + rgb_se) / 8;
  vec3 difference = rgb_o - average;
  
  rgb_o = rgb_o + (difference * sharpen);
  difference = rgb_o - average;
  
  float fdiff = abs(dot(vec3(1,1,1), difference));
  
  if (fdiff > boundry) {
    
    float alias_amount = clamp(fdiff * 2, 0.0, 0.75);
    //return mix(vec3(0,1,0), vec3(1,0,0), alias_amount);
    return mix(rgb_o, average, alias_amount);
    
  } else {
    return rgb_o;
  }
  
}



/* Following technique unfinished! */

float fxaa_lum(vec3 rgb) {
  return rgb.g * (0.587/0.299) + rgb.r;
}

#define THRESHOLD_MIN (1.0 / 16.0)
#define THRESHOLD_EDGE (1.0 / 8.0)

#define SUBPIX_TRIM (1.0 / 4.0)
#define SUBPIX_TRIM_SCALE 1.0
#define SUBPIX_CAP (3.0 / 4.0)

#define SEARCH_STEPS 5

vec3 fxaa(sampler2D screen, vec2 pos, int width, int height) {
  
  float xoff = 1.0 / float(width);
  float yoff = 1.0 / float(height);
  
  vec3 rgb_n = texture2D(screen, pos + vec2(0,yoff)).rgb;
  vec3 rgb_w = texture2D(screen, pos + vec2(xoff,0)).rgb;
  vec3 rgb_o = texture2D(screen, pos + vec2(0,0)).rgb;
  vec3 rgb_e = texture2D(screen, pos + vec2(-xoff,0)).rgb;
  vec3 rgb_s = texture2D(screen, pos + vec2(0,-yoff)).rgb;

  float lum_n = fxaa_lum(rgb_n);
  float lum_w = fxaa_lum(rgb_w);
  float lum_o = fxaa_lum(rgb_o);
  float lum_e = fxaa_lum(rgb_e);
  float lum_s = fxaa_lum(rgb_s);
  
  float range_min = min(lum_o, min(min(lum_n, lum_w), min(lum_s, lum_e)));
  float range_max = max(lum_o, max(max(lum_n, lum_w), max(lum_s, lum_e)));
  
  float range = range_max - range_min;
  
  if (range > max(THRESHOLD_MIN, range_max * THRESHOLD_EDGE)) {
    
    float lum_l = (lum_n + lum_w + lum_e + lum_s) * 0.25;
    float range_l = abs(lum_l - lum_o);
    float blend_l = max(0.0, (range_l / range) - SUBPIX_TRIM) * SUBPIX_TRIM_SCALE;
    
    blend_l = min(SUBPIX_CAP, blend_l);
    
    vec3 rgb_l = rgb_n + rgb_w + rgb_o + rgb_e + rgb_s;
    
    vec3 rgb_nw = texture2D(screen, pos + vec2(xoff,yoff)).rgb;
    vec3 rgb_ne = texture2D(screen, pos + vec2(-xoff,yoff)).rgb;
    vec3 rgb_sw = texture2D(screen, pos + vec2(xoff,-yoff)).rgb;
    vec3 rgb_se = texture2D(screen, pos + vec2(-xoff,-yoff)).rgb;
    
    float lum_nw = fxaa_lum(rgb_nw);
    float lum_ne = fxaa_lum(rgb_ne);
    float lum_sw = fxaa_lum(rgb_sw);
    float lum_se = fxaa_lum(rgb_se);
    
    rgb_l += (rgb_nw + rgb_ne + rgb_sw + rgb_se);
    rgb_l = rgb_l * (1.0 / 9.0);
    
    float edge_v = abs((0.25 * lum_nw) + (-0.5 * lum_n) + (0.25 * lum_ne)) +
                   abs((0.50 * lum_w ) + (-1.0 * lum_o) + (0.50 * lum_e )) +
                   abs((0.25 * lum_sw) + (-0.5 * lum_s) + (0.25 * lum_se));
    
    float edge_h = abs((0.25 * lum_nw) + (-0.5 * lum_w) + (0.25 * lum_sw)) + 
                   abs((0.50 * lum_w ) + (-1.0 * lum_o) + (0.50 * lum_s )) +
                   abs((0.25 * lum_ne) + (-0.5 * lum_e) + (0.25 * lum_se));
    
    vec2 off_np;
    if (edge_h >= edge_v) {
      off_np = vec2(1,0);
    } else {
      off_np = vec2(0,1);
    }
    
    if (edge_h >= edge_v) {
      return vec3(1,0,0);
    } else {
      return vec3(0,1,0);
    }
    
    /*
    
    int done_n = 0;
    int done_p = 0;
    float lum_end_n = 0;
    float lum_end_p = 0;
    vec2 pos_n = pos;
    vec2 pos_p = pos;
    float gradient_n = 0.0;
    
    for(int i = 0; i < SEARCH_STEPS; i++) {
      if (!done_n) lum_end_n = fxaa_lum(texture2DGrad(screen, pos_n, off_np).rgb);
      if (!done_p) lum_end_p = fxaa_lum(texture2DGrad(screen, pos_p, off_np).rgb);
      
      done_n = done_n || (abs(lum_end_n - lum_n) >= gradient_n;
      done_p = done_p || (abs(lum_end_p - lum_p) >= gradient_n;
      if (done_n && done_p) break;
      
      if (!done_n) pos_n -= off_np;
      if (!done_n) pos_n += off_np;
    }
    
    */
    
  }
  
  return rgb_o;
  
}
