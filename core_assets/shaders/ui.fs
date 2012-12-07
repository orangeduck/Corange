#version 120

uniform sampler2D diffuse;
uniform sampler2D random;

uniform float time;
uniform float glitch;

varying vec2 texcoords;
varying vec4 color;

void main() {
  vec4 albedo = texture2D(diffuse, texcoords) * color;
  
  vec4 glitch_off = texture2D(random, texcoords + vec2(0, time / 200));
  vec4 glitch_col = texture2D(random, texcoords + glitch_off.yz + vec2(0, time / 200));
  
  vec4 galbedo = texture2D(diffuse, texcoords + glitch_off.xy * vec2(0.05, 0.025)) * color;
  
  vec3  gfinal = mix(galbedo.rgb, glitch_col.rgb, 0.1) * glitch_col.rgb;
  float galpha = mix(galbedo.a, glitch_col.a, 0.1);
  
  gl_FragColor.rgb = mix(albedo.rgb, gfinal, glitch);
  gl_FragColor.a = mix(albedo.a, galpha, glitch);
}
