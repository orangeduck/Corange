#version 120

uniform vec4 start;
uniform vec4 end;

varying vec4 position;

void main() {
	float blend = (position.y + 1.0) / 2.0;
	gl_FragColor = mix(end, start, blend);
}
