attribute vec3 normal;
attribute vec3 tangent;
attribute vec3 binormal;

uniform mat4 world_matrix;
uniform mat4 view_matrix;
uniform mat4 proj_matrix;

varying vec4 position;
varying mat4 TBN;

void main( void )
{
   gl_Position = proj_matrix * view_matrix * world_matrix * gl_Vertex;
   gl_TexCoord[0] = gl_MultiTexCoord0;
   position = world_matrix * gl_Vertex;

   TBN = mat4( tangent.x, binormal.x, normal.x, 0.0,
               tangent.y, binormal.y, normal.y, 0.0,
               tangent.z, binormal.z, normal.z, 0.0,
               0.0, 0.0, 0.0, 1.0 );
} 