#version 120

/* Photoshop based blending modes */

/* Prototypes */

vec4 darken(vec4 a, vec4 b);
vec4 multiply(vec4 a, vec4 b);
vec4 color_burn(vec4 a, vec4 b);
vec4 linear_burn(vec4 a, vec4 b);
vec4 lighten(vec4 a, vec4 b);
vec4 screen(vec4 a, vec4 b);
vec4 color_dodge(vec4 a, vec4 b);
vec4 linear_dodge(vec4 a, vec4 b);
vec4 overlay(vec4 a, vec4 b);
vec4 soft_light(vec4 a, vec4 b);
vec4 hard_light(vec4 a, vec4 b);
vec4 vivid_light(vec4 a, vec4 b);
vec4 linear_light(vec4 a, vec4 b);
vec4 pin_light(vec4 a, vec4 b);
vec4 difference(vec4 a, vec4 b);
vec4 exclusion(vec4 a, vec4 b);

/* End */

vec4 darken(vec4 a, vec4 b) {
	return min(a, b);
}

vec4 multiply(vec4 a, vec4 b) {
	return a * b;
}

vec4 color_burn(vec4 a, vec4 b) {
	return 1.0 - (1.0-a) / b;
}
vec4 linear_burn(vec4 a, vec4 b) {
	return a + b - 1.0;
}


vec4 lighten(vec4 a, vec4 b) {
	return max(a,b);
}

vec4 screen(vec4 a, vec4 b) {
	return 1.0 - (1.0-a) * (1.0-b);
}

vec4 color_dodge(vec4 a, vec4 b) {
	return a / (1.0-b);
}

vec4 linear_dodge(vec4 a, vec4 b) {
	return a + b;
}


vec4 overlay(vec4 a, vec4 b) {
	vec4 res;
	if ( length(a.rgb) > 0.5 ) { res = 1.0 - (1.0 - 2.0 * (a-0.5)) * (1.0-b); }
	if ( length(a.rgb) <= 0.5) { res = a * (b + 0.5); }
	return res;
}

vec4 soft_light(vec4 a, vec4 b) {
	vec4 res;
	if ( length(a.rgb) > 0.5 ) { res = 1.0 - (1.0 - a) * (1.0 - (b - 0.5)); }
	if ( length(a.rgb) <= 0.5) { res = a * (b + 0.5); }
	return res;	
}

vec4 hard_light(vec4 a, vec4 b) {
	vec4 res;
	if ( length(a.rgb) > 0.5 ) { res = 1.0 - (1.0 - a) * (1.0 - 2.0 * (b - 0.5)); }
	if ( length(a.rgb) <= 0.5) { res = a * (2.0 * b); }
	return res;	
}

vec4 vivid_light(vec4 a, vec4 b) {
	vec4 res;
	if ( length(a.rgb) > 0.5 ) { res = 1.0 - (1.0 - a) / (2.0 * (b - 0.5)); }
	if ( length(a.rgb) <= 0.5) { res = a / (1.0 - 2.0 * b); }
	return res;		
}

vec4 linear_light(vec4 a, vec4 b) {
	vec4 res;
	if ( length(a.rgb) > 0.5 ) { res = a + (2.0 * (b - 0.5)); }
	if ( length(a.rgb) <= 0.5) { res = a + 2.0 * b - 1.0; }
	return res;			
}

vec4 pin_light(vec4 a, vec4 b) {
	vec4 res;
	if ( length(a.rgb) > 0.5 ) { res = max(a , 2.0 * (b - 0.5)); }
	if ( length(a.rgb) <= 0.5) { res = min(a , 2.0 * b); }
	return res;	
}


vec4 difference(vec4 a, vec4 b) {
	return abs(a-b);
}

vec4 exclusion(vec4 a, vec4 b) {
	return 0.5 - 2.0 * (a - 0.5) * (b - 0.5);
}