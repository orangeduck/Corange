#version 120

uniform sampler2D diffuse;
uniform float alpha_test;

varying vec2 fTexcoord;
varying vec4 fColor;

void main() {
	
	vec4 albedo = texture2D(diffuse, fTexcoord) * fColor;
	if (albedo.a < alpha_test) {
	  discard;
	}
	
	gl_FragData[0].rgb = albedo.rgb;
	gl_FragData[0].a = 0.0;
	
	gl_FragData[1].rgb = vec3(0,0,0);
	gl_FragData[1].a = float(4);
  
  gl_FragDepth = 0.0;
  
}
