#version 120

uniform vec4 start;
uniform vec4 end;

varying vec4 position;

void main() {
	float blend = (position.y + 1.0) / 2.0;
	
	gl_FragData[0].rgb = mix(end.rgb, start.rgb, blend);
	gl_FragData[0].a = 0.0;
	
	gl_FragData[1].rgb = vec3(0,0,0);
	gl_FragData[1].a = float(1);
	
	gl_FragData[2].rgb = vec3(0,0,0);
	gl_FragData[2].a = 0.0;
}