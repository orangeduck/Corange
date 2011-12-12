uniform sampler2D hdr_texture;

// Control exposure with this value
uniform float exposure;
uniform float brightMax;

void main()
{
  vec4 color = texture2D(hdr_texture, gl_TexCoord[0].xy);
  
  float exposure = 0.1;
  float brightmax = 50.0;
  
  float y = dot( vec4(0.30, 0.59, 0.11, 0.0) , color );
  float yd = 1.0 * ( exposure / brightmax + 1.0 ) / (exposure + 1.0);
  
  color *= yd;
  
  gl_FragColor = color;
}