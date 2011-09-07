uniform sampler2D hdr_texture;

// Control exposure with this value
uniform float exposure;
uniform float brightMax;


vec4 gamma(vec4 color, float amount) {
	vec4 ret;
	ret.r = pow(color.r, amount);
	ret.g = pow(color.g, amount);
	ret.b = pow(color.b, amount);
	ret.a = color.a;
	return ret;
}

vec4 gamma_space(vec4 color) {
	vec4 ret;
	ret.r = pow(color.r, 2.2);
	ret.g = pow(color.g, 2.2);
	ret.b = pow(color.b, 2.2);
	ret.a = color.a;
	return ret;
}

vec4 linear_space(vec4 color) {
	vec4 ret;
	ret.r = pow(color.r, 1.0/2.2);
	ret.g = pow(color.g, 1.0/2.2);
	ret.b = pow(color.b, 1.0/2.2);
	ret.a = color.a;
	return ret;
}

float A = 0.15;
float B = 0.50;
float C = 0.10;
float D = 0.20;
float E = 0.02;
float F = 0.30;
float W = 11.2;

vec3 tonemap(vec3 x) {
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 stepped_color(vec3 color, float num_steps) {
	color *= num_steps;
	color = floor(color);
	color /= num_steps;
	return color;
}

void main()
{
  vec4 color = texture2D(hdr_texture, gl_TexCoord[0].xy);
  
  //color = color / (1+color);
  //gl_FragColor = linear_space(color);
  
  float exposure = 1.0f;
  vec3 curr = tonemap( exposure * color.rgb );
  
  vec3 white_scale = 1.0 / 1.0 / tonemap( vec3(W,W,W) );
  vec3 retcolor = curr * white_scale;
  
  //vec3 x = max(0, color - 0.004);
  //vec3 ret = (x*(6.2*x+.5))/(x*(6.2*x+1.7)+0.06);
  
  retcolor = linear_space( vec4(retcolor,1) ).rgb;
  
  retcolor = stepped_color(retcolor, 1.618 * 20.0);
  
  gl_FragColor = vec4(retcolor,1);
}