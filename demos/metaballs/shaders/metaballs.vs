attribute vec4 normals;

varying vec3 position;
varying vec3 normal;
varying vec2 uvs;

void main()
{
  uvs = gl_MultiTexCoord0.xy;
  normal = normals.xyz;
  position = gl_Vertex;
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
