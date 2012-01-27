varying vec3 normal;
varying vec2 uvs;

void main()
{
  gl_FragColor = vec4((normal + 1)/2, 1.0);
}
