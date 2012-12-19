#version 120

attribute vec3 vPosition;
attribute vec2 vTexcoord;
attribute vec3 vBone_indicies;
attribute vec3 vBone_weights;

uniform mat4 world_bones[32];
uniform mat4 world;
uniform mat4 proj;
uniform mat4 view;

varying vec2 fTexcoord;
varying float fDepth;

void main() {

  fTexcoord = vTexcoord;

  vec4 blendpos = vec4(0,0,0,0);
  for (int i = 0; i < 3; i++) {
    blendpos += vec4((world_bones[int(vBone_indicies[i])] * vec4(vPosition, 1)).xyz, 1.0) * vBone_weights[i];
  }
  
  vec4 screen_position = proj * view * world * blendpos;
  fDepth = screen_position.z / screen_position.w;
  gl_Position = screen_position;
}