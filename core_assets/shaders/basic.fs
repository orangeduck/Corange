#version 120

varying vec3 normal;

void main() {
  gl_FragColor = vec4((normal + 1.0) / 2.0, 1.0);
}
