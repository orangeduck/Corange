uniform sampler2D diffuse;

varying float alpha_test;
varying vec3 color;
varying vec2 uvs;

vec3 from_gamma(vec3 color);
vec3 to_gamma(vec3 color);

void main() {
  vec4 diffuse_tex = texture2D(diffuse, uvs);
  
  if (diffuse_tex.a < alpha_test) { discard; }
  
  vec3 albedo = from_gamma(diffuse_tex.rgb);
  
  gl_FragColor = vec4(to_gamma(albedo * color), 1.0);
}
