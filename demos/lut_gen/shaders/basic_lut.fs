varying vec3 normal;
varying vec2 uvs;

uniform sampler2D diffuse;
uniform sampler3D lut;

vec3 color_correction(vec3 color, sampler3D lut, int lut_size) {

  float scale = (float(lut_size) - 1.0) / float(lut_size);
  float offset = 1.0 / (2.0 * float(lut_size));

  return texture3D(lut, clamp(color, 0.0, 1.0) * scale + offset).rgb;
}

void main() {
  vec3 color = texture2D(diffuse, uvs).rgb;
  vec3 corrected = color_correction(color, lut, 64);
  gl_FragColor = vec4(corrected, 1.0);
}