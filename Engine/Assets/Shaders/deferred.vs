
attribute vec3 vNormal, vTangent, vBiNormal;

uniform mat4 ModelMatrix;
uniform mat4 TransformMatrixInvese;
uniform mat4 RotationMatrixInverse;

varying vec4 normals;
varying vec4 position;
varying mat4 TBN;

void main( void )
{
   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
   gl_TexCoord[0] = gl_MultiTexCoord0;
   normals = RotationMatrixInverse * vec4(gl_Normal,1);
   position = ModelMatrix * gl_Vertex;


   TBN = mat4( vTangent.x, vBiNormal.x, vNormal.x, 0,
               vTangent.y, vBiNormal.y, vNormal.y, 0,
               vTangent.z, vBiNormal.z, vNormal.z, 0,
               0, 0, 0, 1 );

   gl_FrontColor = vec4(1.0, 1.0, 1.0, 1.0);
} 