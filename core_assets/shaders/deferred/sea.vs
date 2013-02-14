#version 120

attribute vec3 vPosition;
attribute vec2 vTexcoord;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;
uniform float time;

varying vec2 fTexcoord;
varying vec3 fNormal;
varying vec4 fScreen;

vec4 spike_wave( vec2 direction, vec2 position, float wavelength, float spikiness ) {

  vec4 result = vec4(0,0,0,0);
  result.y = sin( dot(direction, position / wavelength) ) + 1.0;
  result.xz = 0.5 * spikiness * direction * cos( dot(direction, position / wavelength) );

  return result;
}

void main() {
  
  vec4 world_pos = world * vec4(vPosition, 1);
  world_pos = world_pos / world_pos.w;
  
  fTexcoord = world_pos.xz * 0.3;
  
  vec4 wave = vec4(0,0,0,0);
  wave += 0.40 * spike_wave( normalize(vec2(1.00, 0.00)) , world_pos.xz * 0.912 + time * 3.211 , 3.30, 3 );
  wave += 0.15 * spike_wave( normalize(vec2(1.00, 0.62)) , world_pos.xz * 0.932 + time * 2.971 , 2.85, 3 );
  wave += 0.05 * spike_wave( normalize(vec2(0.41, 1.00)) , world_pos.xz * 1.112 + time * 3.314 , 3.21, 3 );
  wave += 0.40 * spike_wave( normalize(vec2(0.10, 0.51)) , world_pos.xz * 1.321 + time * 2.823 , 4.12, 3 );
  
  fNormal = vec3( 1 * wave.x, -1.0, 1 * wave.z);
  fScreen = proj * view * vec4(world_pos + wave);
  gl_Position = fScreen;
}

