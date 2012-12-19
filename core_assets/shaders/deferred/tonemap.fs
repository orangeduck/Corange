#version 120

uniform sampler2D hdr_texture;
uniform float exposure;

varying vec2 fTexcoord;

/* Headers */

vec3 filmic_tonemap(vec3 color, float exposure);

/* End */

void main() {

  vec4 color = texture2D(hdr_texture, fTexcoord);
  
  color.rgb = filmic_tonemap(color.rgb, exposure);
  
  float intensity = (color.r + color.g + color.b) / 3.0;
  color.a = max(intensity - 1.0, 0.0);
  
  gl_FragColor = color;
}