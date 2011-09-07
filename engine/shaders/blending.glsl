// Photoshop based blending modes

vec4 darken(a,b) {
	return min(a, b);
}

vec4 multiply(a,b) {
	return a * b;
}

vec4 color_burn(a,b) {
	return 1 - (1-a) / b;
}
vec4 linear_burn(a,b) {
	return a + b - 1;
}


vec4 lighten(a,b) {
	return max(a,b);
}

vec4 screen(a,b) {
	return 1 - (1-a) * (1-b);
}

vec4 color_dodge(a,b) {
	return a / (1-b);
}

vec4 linear_dodge(a,b) {
	return a + b;
}


vec4 overlay(a,b) {
	vec4 res;
	if ( a > 0.5 ) { res = 1 - (1 - 2 * (a-0.5)) * (1-b); }
	if ( a <= 0.5) { res = a * (b + 0.5); }
	return res;
}

vec4 soft_light(a,b) {
	vec4 res;
	if ( a > 0.5 ) { res = 1 - (1 - a) * (1 - (b - 0.5)); }
	if ( a <= 0.5) { res = a * (b + 0.5); }
	return res;	
}

vec4 hard_light(a,b) {
	vec4 res;
	if ( a > 0.5 ) { res = 1 - (1 - a) * (1 - 2 * (b - 0.5)); }
	if ( a <= 0.5) { res = a * (2 * b); }
	return res;	
}

vec4 vivid_light(a,b) {
	vec4 res;
	if ( a > 0.5 ) { res = 1 - (1 - a) / (2 * (b - 0.5)); }
	if ( a <= 0.5) { res = a / (1 - 2 * b); }
	return res;		
}

vec4 linear_light(a,b) {
	vec4 res;
	if ( a > 0.5 ) { res = a + (2 * (b - 0.5)); }
	if ( a <= 0.5) { res = a + 2 * b - 1.0; }
	return res;			
}

vec4 pin_light(a,b) {
	vec4 res;
	if ( a > 0.5 ) { res = max(a , 2 * (b - 0.5)); }
	if ( a <= 0.5) { res = min(a , 2 * b); }
	return res;	
}


vec4 difference(a,b) {
	return abs(a-b);
}

vec4 exclusion(a,b) {
	return 0.5 - 2 * (base - 0.5) * (b - 0.5);
}