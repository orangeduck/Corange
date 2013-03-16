#version 120

uniform sampler2D hdr_texture;
uniform float exposure;

/* Headers */

vec3 filmic_tonemap(vec3 color, float exposure);

/* End */

void main() {

  vec4 color = texture2D(hdr_texture, gl_TexCoord[0].xy);
  
  color.rgb = filmic_tonemap(color.rgb, exposure);
  
  float intensity = (color.r + color.g + color.b) / 3.0;
  color.a = max(intensity - 1.0, 0.0);
  
  gl_FragColor = color;
}