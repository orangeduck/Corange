varying vec4 screen_position;

void main()
{
  screen_position = gl_ModelViewProjectionMatrix * gl_Vertex;
  gl_Position = screen_position;
}