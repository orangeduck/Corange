uniform vec4 start;
uniform vec4 end;

varying vec4 position;

void main() {
	float blend = (position.y + 1) / 2;
	gl_FragColor = mix(end, start, blend);
}
