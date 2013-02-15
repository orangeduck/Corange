#version 120

varying vec3 fNormal;

void main() {
  gl_FragColor = vec4((fNormal + 1.0) / 2.0, 1.0);
}
