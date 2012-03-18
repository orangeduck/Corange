varying vec3 normal;
varying vec2 uvs;

void main() {
  gl_FragColor = vec4((normal + 1.0)/2.0, 1.0);
}
