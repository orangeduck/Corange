#version 120

uniform sampler2D diffuse_map;
uniform float alpha_test;

uniform float clip_near;
uniform float clip_far;

varying vec2 fTexcoord;
varying float fDepth;

float linear_depth(float depth, float near, float far){
  return (2.0 * near) / (far + near - depth * (far - near));
}

void main() {
  
  vec2 uvs = vec2(fTexcoord.x, 1-fTexcoord.y);
  
  float alpha = texture2D(diffuse_map, uvs).a;
  if (alpha < alpha_test) {
    discard;
  }
  
  float depth = linear_depth(fDepth, clip_near, clip_far);
  
  //gl_FragDepth = 0;
  gl_FragColor = vec4(depth, depth, depth, depth);
}