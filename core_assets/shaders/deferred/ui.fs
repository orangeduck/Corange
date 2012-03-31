uniform sampler2D diffuse;

uniform float alpha_test;

void main() {
	
	vec4 color = texture2D(diffuse, gl_TexCoord[0].xy);
	if (color.a < alpha_test) {
	  discard;
	}
	
	gl_FragData[0].rgb = color.rgb;
	gl_FragData[0].a = 0.0;
	
	gl_FragData[1].rgb = vec3(0,0,0);
	gl_FragData[1].a = float(1);
	
	gl_FragData[2].rgb = vec3(0,0,0);
	gl_FragData[2].a = 0.0;
}
