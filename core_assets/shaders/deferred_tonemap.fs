uniform sampler2D hdr_texture;

uniform float exposure;

vec3 filmic_tonemap(vec3 color, float exposure);

void main()
{
  vec4 color = texture2D(hdr_texture, gl_TexCoord[0].xy);
  
  color.rgb = filmic_tonemap(color.rgb, exposure);
  color.a = length(color.rgb - vec3(1,1,1));
  
  gl_FragColor = color;
}