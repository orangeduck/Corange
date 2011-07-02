uniform sampler2D image1;
uniform sampler2D image2;

varying vec2 uvs;

void main()
{
  vec4 color1 = texture2D(image1, uvs);
  vec4 color2 = texture2D(image2, uvs);
  
  gl_FragColor = color1 * color2;
}