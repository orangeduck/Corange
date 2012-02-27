uniform sampler2D diffuse;

varying vec2 uvs;

void main() {
  uvs.y = -uvs.y;
  vec3 color = texture2D(diffuse, uvs).rgb;
  
  float brightness = 1.5;
  
  gl_FragColor = vec4(brightness * color, 1.0);
}
