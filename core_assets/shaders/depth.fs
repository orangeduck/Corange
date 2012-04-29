#version 120

uniform sampler2D diffuse;
uniform float alpha_test;

void main() {
 
  float alpha = texture2D(diffuse, gl_TexCoord[0].xy).a;
  if (alpha < alpha_test) {
    discard;
  }

  gl_FragColor = vec4(gl_FragCoord.zzz, 1.0);
}