#version 120

uniform samplerCube cubemap;
uniform vec3 camera_position;
varying vec3 fPosition;
varying vec3 fNormal;

void main( void )
{
    vec3 view = fPosition - camera_position;
    vec3 R = reflect( view,normalize(fNormal));
    gl_FragColor = textureCube(cubemap,R);
}

