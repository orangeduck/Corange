varying vec4         position;
varying mat4         TBN;

uniform sampler2D    tDiffuse, tBumpMap;

void main( void )
{
   gl_FragData[0] = texture2D(tDiffuse,gl_TexCoord[0].st);
   gl_FragData[0].a = 1;
   gl_FragData[1] = vec4(position.xyz,1);
   gl_FragData[2] = (texture2D(tBumpMap,gl_TexCoord[0].st) * 2 -
                     vec4(1,1,1,0)) * TBN;
   gl_FragData[2].a = 1;
}