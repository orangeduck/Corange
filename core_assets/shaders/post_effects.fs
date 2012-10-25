#version 120

/* Headers */

vec3 unsharp_mask(sampler2D screen, vec2 coords, float strength, int width, int height);
vec3 chromatic_aberration(sampler2D screen, vec2 coords, float offset);
vec3 basic_dof(int width, int height, sampler2D screen, sampler2D screendepth, sampler2D random, vec2 coords);
vec3 bokeh_dof(int width, int height, float near, float far, sampler2D screen, sampler2D screendepth, sampler2D random, vec2 coords, float focalDepth);
float linear_depth(float depth, float near, float far);

/* End */

vec3 unsharp_mask(sampler2D screen, vec2 coords, float strength, int width, int height) {
  
  float off_x = 1.0 / float(width);
  float off_y = 1.0 / float(height);
  
  vec3 s0 = texture2D(screen, coords + vec2(-off_x, off_y)).rgb;
  vec3 s1 = texture2D(screen, coords + vec2(0.0, off_y)).rgb;
  vec3 s2 = texture2D(screen, coords + vec2(off_x, off_y)).rgb;
  vec3 s3 = texture2D(screen, coords + vec2(-off_x, 0.0)).rgb;
  vec3 s4 = texture2D(screen, coords + vec2(0.0, 0.0)).rgb;
  vec3 s5 = texture2D(screen, coords + vec2(off_x, 0.0)).rgb;
  vec3 s6 = texture2D(screen, coords + vec2(-off_x, -off_y)).rgb;
  vec3 s7 = texture2D(screen, coords + vec2(0.0, 0.0)).rgb;
  vec3 s8 = texture2D(screen, coords + vec2(off_x, -off_y)).rgb;
  
  vec3 avg = (s0 + s1 + s2 + s3 + s5 + s6 + s7 + s8) / 8.0;
  
  vec3 diff = s4 - avg;
  return s4 + (diff * strength);
}

vec3 chromatic_aberration(sampler2D screen, vec2 coords, float offset) {

  offset = offset / 1000.0;
  vec3 result;
	result.r = texture2D(screen, coords + vec2(offset,0.0)).r;
	result.g = texture2D(screen, coords - vec2(offset,0.0)).g;
	result.b = texture2D(screen, coords - vec2(offset,0.0)).b;
  
  return result;
}

float linear_depth(float depth, float near, float far) {
  return (2.0 * near) / (far + near - depth * (far - near));
}

// Depth of field

vec2 sample_circle[32] = vec2[32](vec2(-0.00, 0.02), vec2(0.35, 0.04), vec2(0.66, 0.32), vec2(-0.04, 0.04), vec2(0.24, 0.22), vec2(-0.09, 0.10), vec2(0.24, 0.04), vec2(0.37, 0.88), vec2(0.02, 0.11), vec2(-0.04, 0.83), vec2(0.33, 0.11), vec2(0.21, 0.17), vec2(0.48, 0.30), vec2(0.39, 0.72), vec2(0.19, 0.20), vec2(0.35, 0.04), vec2(-0.00, 0.02), vec2(-0.07, 0.12), vec2(0.00, 0.01), vec2(-0.27, 0.41), vec2(0.13, 0.26), vec2(0.15, 0.19), vec2(-0.32, 0.29), vec2(-0.00, 0.00), vec2(-0.36, 0.18), vec2(0.70, 0.21), vec2(-0.36, 0.17), vec2(-0.11, 0.12), vec2(-0.59, 0.67), vec2(-0.24, 0.75), vec2(0.18, 0.04), vec2(-0.16, 0.11));

vec3 pow_3(vec3 x, float f) {
  vec3 y;
  y.r = pow(x.r, f);
  y.g = pow(x.g, f);
  y.b = pow(x.b, f);
  return y;
}

vec3 circle_blur(int width, int height, sampler2D screen, sampler2D random, vec2 coords, float amount, float size) {
  
  vec2 texel = vec2(1.0/float(width), 1.0/float(height));
  vec3 pixel = texture2D(screen, coords).rgb;
  
  const float randscale = 50;
  vec2 rand = texture2D(random, coords * randscale).xy;
  
  vec3 accum = vec3(0.0, 0.0, 0.0);
  
  const int samples = 16;
  for(int i = 0; i < samples; i++) {
    vec2 reflected = reflect(sample_circle[i], rand);
    vec3 sample = texture2D(screen, coords + reflected * texel * size).rgb;
    accum += sample;
  }
  accum /= samples;
  
  return mix(pixel, accum, clamp(amount, 0.0, 1.0));
  
}

vec3 basic_dof(int width, int height, sampler2D screen, sampler2D screendepth, sampler2D random, vec2 coords) {
  
  vec3 pixel = texture2D(screen, coords).rgb;
  float depth = texture2D(screendepth, coords).r;
  float center_depth = texture2D(screendepth, vec2(0.5, 0.5)).r;
  
  float amount = pow( abs(depth-center_depth), 2.0);
  float brightness = pow( dot(pixel, vec3(1.0, 1.0, 1.0)), 2.0);
  
  amount = abs(amount) * 1.0;
  
  return circle_blur(width, height, screen, random, coords, amount, 50.0);
  
}

vec3 bokeh_color(sampler2D screen, vec2 coords, vec2 texel, float blur) {
	
  const float threshold = 0.5;
  const float gain = 5.0;
  const float fringe = 0.75;
  
  vec3 col;
	col.r = texture2D(screen, coords + vec2(0.0,1.0)*texel*fringe*blur).r;
	col.g = texture2D(screen, coords + vec2(-0.866,-0.5)*texel*fringe*blur).g;
	col.b = texture2D(screen, coords + vec2(0.866,-0.5)*texel*fringe*blur).b;
	
	vec3 lumcoeff = vec3(0.299, 0.587, 0.114);
	float lum = dot(col.rgb, lumcoeff);
	float thresh = max((lum-threshold)*gain, 0.0);
  
	return col + mix(vec3(0.0, 0.0, 0.0), col, thresh * blur);
}

vec3 bokeh_dof(int width, int height, sampler2D screen, sampler2D screendepth, sampler2D random, vec2 coords, float focal_depth) {
  
  vec2 texel = vec2(1.0/width,1.0/height);
  
  const float focal_length = 0.5; // Lower is wider range of sharp focus
  const float fstop = 1000.0; // Lower increases over exposure
  const float CoC = 0.003; // Just multiplied by fstop
  const float fblur = 30.0;
  
	float depth = texture2D(screendepth, coords).x;
  float center_depth = texture2D(screendepth, vec2(0.5, 0.5)).x;
	float focus_depth = focal_depth > 0.0 ? focal_depth : center_depth;
  
  float f = focal_length; //focal length in mm
  float d = focus_depth * 1000.0; //focal plane in mm
  float o = depth * 1000.0; //depth in mm
  
  float a = (o*f)/(o-f); 
  float b = (d*f)/(d-f); 
  float c = (d-f)/(d * fstop * CoC); 
  
  float blur = abs(a-b)*c;
	blur = clamp(blur, 0.0, 1.0);
  
	vec2 noise = texture2D(random, coords * 100 * blur).rg;
	
	// getting blur x and y step factor
  vec2 offset = noise * texel * blur * fblur;
  
	// calculation of final color
  const int samples = 3; //samples on the first ring
  const int rings = 3; //ring count
  const float bias = 0.5; //bokeh edge bias

	vec3 col = vec3(0.0);
  
	if(blur < 0.05) {
	
    col = texture2D(screen, coords).rgb;
	
  } else {
  
		col = texture2D(screen, coords).rgb;
		float s = 1.0;
		for (int i = 1; i <= rings; i += 1) {   
			
      int ringsamples = i * samples;
			for (int j = 0 ; j < ringsamples ; j += 1) {
				float step = 3.141*2.0 / float(ringsamples);
				float pw = (cos(float(j)*step)*float(i));
				float ph = (sin(float(j)*step)*float(i));
        float amount = mix(1.0,(float(i))/(float(rings)),bias);
				col += bokeh_color(screen, coords + vec2(pw,ph) * offset, texel, blur) * amount;  
				s += 1.0 * mix(1.0,(float(i))/(float(rings)),bias);   
			}
		}
		col /= s;
    
	}
  
  return col;
}
	
