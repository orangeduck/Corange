uniform sampler2D diffuse_texture;
uniform sampler2D positions_texture;
uniform sampler2D normals_texture;
uniform sampler2D depth_texture;

uniform sampler2D shadows_texture;

uniform vec3 camera_position;
uniform vec3 light_position;

uniform mat4 light_view;
uniform mat4 light_proj;

void main( void )
{
	vec4 position = texture2D( positions_texture, gl_TexCoord[0].xy );
	
  // SHADOW MAPPING
  
  float shadow = 1.0;
  
  vec4 light_pos = light_proj * light_view * vec4(position.xyz,1.0);
  
  vec4 shadow_coord = (light_pos / light_pos.w) / 2.0 + 0.5;
  
  float shadow_depth = texture2D( shadows_texture, shadow_coord.xy ).r;
  float our_depth = shadow_coord.z;
  
  if (our_depth >= shadow_depth) {
	shadow = 0.1;
  }
  
  // END
	
	vec4 diffuse_a = texture2D( diffuse_texture, gl_TexCoord[0].xy );
	vec3 diffuse = diffuse_a.rgb;
	
	float spec = diffuse_a.a;
	
	vec4 normal_a = texture2D( normals_texture, gl_TexCoord[0].xy );
	vec3 normal = normal_a.rgb;
	float glossiness = normal_a.a;
	
	vec3 lightDir = light_position - position.xyz ;

	normal = normalize(normal);
	lightDir = normalize(lightDir);

	vec3 eyeDir = normalize(camera_position - position.xyz);
	vec3 vHalfVector = normalize(lightDir + eyeDir);
	
	vec3 ambient_amount = 0.3 * diffuse;
	float light_amount = max(dot(normal, lightDir), 0.0);
	float spec_amount = spec * pow(max(dot(normal, vHalfVector),0.0), glossiness);
	
	float difference = our_depth - shadow_depth;
	
	gl_FragColor.rgb = shadow * light_amount * diffuse + ambient_amount + spec_amount;
	//gl_FragColor.rgb = vec3(difference,difference,difference);
	gl_FragColor.a = 1.0;
	
} 