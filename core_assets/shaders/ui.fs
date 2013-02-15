#version 120

uniform sampler2D diffuse;
uniform sampler2D random;

uniform float time;
uniform float glitch;

varying vec2 fTexcoord;
varying vec4 fColor;

void main() {
  vec4 albedo = texture2D(diffuse, fTexcoord) * fColor;
  
  vec4 glitch_off = texture2D(random, fTexcoord + albedo.rg * vec2(0, time / 200));
  vec4 glitch_col = texture2D(random, fTexcoord + glitch_off.yz + vec2(0, time / 200));
  
  vec4 galbedo = texture2D(diffuse, fTexcoord + glitch_off.xy * vec2(0.05, 0.025)) * fColor;
  
  vec3  gfinal = mix(galbedo.rgb, glitch_col.rgb, 0.30) * glitch_col.rgb * glitch_col.rgb;
  float galpha = mix(galbedo.a, glitch_col.a, 0.15);
  
  gl_FragColor.rgb = mix(albedo.rgb, gfinal, glitch);
  gl_FragColor.a = mix(albedo.a, galpha, glitch);
}
