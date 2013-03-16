#version 120

attribute vec3 vPosition;
attribute vec3 vNormal;
attribute vec3 vTangent;
attribute vec3 vBinormal;
attribute vec2 vTexcoord;
attribute vec4 vColor;

uniform mat4 world;
uniform mat4 view;
uniform mat4 proj;

varying vec2 fTexcoord;
varying vec4 fScreen;
varying vec4 fColor;
varying mat4 fTBN;

void main() {

  fTexcoord = vTexcoord;
  fColor = vColor;
  
  fTBN = mat4(
    vTangent.x,  vTangent.y,  vTangent.z,  0,
    vBinormal.x, vBinormal.y, vBinormal.z, 0,
    vNormal.x,   vNormal.y,   vNormal.z,   0,
    0, 0, 0, 1);
  
  fScreen = proj * view * world * vec4(vPosition, 1);
  gl_Position = fScreen;
}

