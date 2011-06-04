uniform sampler2D tImage0;
uniform sampler2D tImage1;
uniform sampler2D tImage2;

uniform vec3 cameraPosition;

void main( void )
{
	vec4 diffuse_a = texture2D( tImage0, gl_TexCoord[0].xy );
	vec3 diffuse = diffuse_a.rgb;
	float spec = diffuse_a.a;
	
	vec4 position = texture2D( tImage1, gl_TexCoord[0].xy );
	
	vec4 normal_a = texture2D( tImage2, gl_TexCoord[0].xy );
	vec3 normal = normal_a.rgb;
	float glossiness = normal_a.a;
	
	vec3 light = vec3(150.0,250.0,0.0);
	vec3 lightDir = light - position.xyz ;

	normal = normalize(normal);
	lightDir = normalize(lightDir);

	vec3 eyeDir = normalize(cameraPosition - position.xyz);
	vec3 vHalfVector = normalize(lightDir + eyeDir);
	
	vec3 ambient_amount = 0.3 * diffuse;
	float light_amount = max(dot(normal, lightDir), 0.0);
	float spec_amount = pow(max(dot(normal, vHalfVector),0.0), glossiness) * spec;
	
	gl_FragColor.rgb = light_amount * diffuse + ambient_amount + spec_amount;
	gl_FragColor.a = 1.0;
} 