#version 120

uniform sampler2D cloud_texture;
uniform vec3 cloud_color;
uniform vec3 cloud_light;

uniform float time;
uniform float wind;
uniform float opacity;

varying vec2 fTexcoord;

void main() {

  float speed = 0.0025 * wind;

  vec2 coords = vec2(fTexcoord.x + time * speed, 1-fTexcoord.y);
  
	gl_FragColor = vec4(cloud_light * cloud_color, opacity) * texture2D(cloud_texture, coords);
}
