#version 120

uniform sampler2D sun_texture;
uniform float sun_brightness;
uniform vec4 sun_color;

varying vec2 fTexcoord;

void main() {
	gl_FragColor = sun_brightness * sun_color * texture2D(sun_texture, fTexcoord);
}
