varying vec3 normal;

void main()
{
  vec3 color = vec3(0.5f, 0.5f, 0.5f);
  float light = dot(normalize(normal) , vec3(1.0f, 1.0f, 1.0f) );
  
  vec3 ambient = vec3(0.2f, 0.2f, 0.2f);
  
  vec3 final = (color * light) + ambient;
  
  gl_FragColor = vec4(final, 1.0f);
}