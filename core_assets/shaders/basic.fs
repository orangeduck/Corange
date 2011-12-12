varying vec3 normal;

void main()
{
  vec3 color = vec3(0.5, 0.5, 0.5);
  float light = dot(normalize(normal) , vec3(1.0, 1.0, 1.0) );
  
  vec3 ambient = vec3(0.2, 0.2, 0.2);
  
  vec3 final = (color * light) + ambient;
  
  gl_FragColor = vec4(final, 1.0);
}