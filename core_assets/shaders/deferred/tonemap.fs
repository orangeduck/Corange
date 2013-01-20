#version 120

uniform sampler2D hdr_texture;
uniform float exposure;

varying vec2 fTexcoord;

vec3 filmic_map(vec3 color) {

  const float A = 0.15;
  const float B = 0.50;
  const float C = 0.10;
  const float D = 0.20;
  const float E = 0.02;
  const float F = 0.30;

  return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

vec3 filmic_tonemap(vec3 color, float exposure) { 

  const float W = 11.2;

  vec3 curr = filmic_map( exposure * color );
  vec3 white_scale = 1.0 / 1.0 / filmic_map( vec3(W,W,W) );
  return curr * white_scale;
}

void main() {

  vec4 color = texture2D(hdr_texture, fTexcoord);
  
  color.rgb = filmic_tonemap(color.rgb, exposure);
  
  float intensity = (color.r + color.g + color.b) / 3.0;
  color.a = max(intensity - 1.0, 0.0);
  
  gl_FragColor = color;
}