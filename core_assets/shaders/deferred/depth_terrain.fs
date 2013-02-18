#version 120

uniform float clip_near;
uniform float clip_far;

varying float fDepth;

float linear_depth(float depth, float near, float far){
  return (2.0 * near) / (far + near - depth * (far - near));
}

void main() {
  float depth = linear_depth(fDepth, clip_near, clip_far);
  
  //gl_FragDepth = 0;
  gl_FragColor = vec4(depth, depth, depth, depth);
}