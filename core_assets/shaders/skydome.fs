uniform sampler2D diffuse;
varying vec2 uvs;

void main() {
  /*
  uvs.y = -uvs.y;
  vec3 color = texture2D(diffuse, uvs).rgb;
  
  float brightness = 1.0;
  float contrast = 1.0;
  
  color = brightness * pow(color, contrast);
  
  gl_FragColor = vec4(color , 1.0);
  */
  
  vec3 bottom_color = mix(vec3(0.066,0.2,0.450), vec3(0.5098,0.6501,0.7529), 0.5);
  vec3 top_color = mix(vec3(0.066,0.2,0.450), vec3(0.5098,0.6501,0.7529), 0.1);
  
  vec3 color = mix(bottom_color, top_color, uvs.y);
  
  gl_FragColor = vec4(color , 1.0);
}
