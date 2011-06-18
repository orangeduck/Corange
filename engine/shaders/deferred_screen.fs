uniform sampler2D diffuse_texture;
uniform sampler2D positions_texture;
uniform sampler2D normals_texture;
uniform sampler2D depth_texture;

uniform vec3 cameraPosition;

vec4 color_difference_aa(sampler2D diffuse, vec2 uvs) {

	float width = 800.0;
	float height = 600.0;
	
	float x_offset = 1.0 / width;
	float y_offset = 1.0 / height;
	
	vec4 original = texture2D(diffuse, uvs);
	
	vec4 s1 = texture2D(diffuse, uvs + vec2(x_offset, y_offset) );
	vec4 s2 = texture2D(diffuse, uvs + vec2(-x_offset, y_offset) );
	vec4 s3 = texture2D(diffuse, uvs + vec2(x_offset, -y_offset) );
	vec4 s4 = texture2D(diffuse, uvs + vec2(-x_offset, -y_offset) );

	float difference = dot(abs(s1 - original) + abs(s2 - original) + abs(s3 - original) + abs(s4 - original), vec4(1.0,1.0,1.0,1.0) );
	
	vec4 blurred = (s1 + s2 + s3 + s4) / 4.0;
	
	difference = clamp( difference * difference * 0.1 , 0.0 , 1.0 );
	
	return mix(original, blurred, difference);
}

float ssao(sampler2D positions, sampler2D normals, sampler2D depth, vec2 uvs) {
	
	float size = 0.025;
	float screen_ratio = 1.3333;
	vec2 scope = vec2(size * screen_ratio, size);
	
	// 8-point circle
	vec2 samples[8] = vec2[8](
		scope * vec2(0.0, 1.0),
		scope * vec2(0.707, 0.707),
		scope * vec2(1.0, 0.0),
		scope * vec2(0.707, -0.707),
		scope * vec2(0.0, -1.0),
		scope * vec2(-0.707, -0.707),
		scope * vec2(-1.0, 0.0),
		scope * vec2(-0.707, 0.707));
		
	
	float point_depth = texture2D(depth, uvs).r;
	vec3 point_normal = texture2D(normals, uvs).rgb;
	vec3 point_position = texture2D(positions, uvs).rgb;
	
	float shadow = 0.0;
	
	for(int i=0; i < 8; i++) {
		float sample_depth = texture2D(depth, uvs + samples[i]).r;
		vec3 sample_normal = texture2D(normals, uvs + samples[i]).rgb;
		vec3 sample_position = texture2D(positions, uvs + samples[i]).rgb;
		
		float depth_diff = point_depth - sample_depth;
		float normal_diff = max(dot(sample_normal, point_normal),0.0);
		float position_diff = max(dot(sample_position, point_position),0.0);
		
		if (depth_diff > 0.0) {
			shadow += depth_diff * (normal_diff * 1.0);
		}
	}
	
	return 1.0 - (shadow * 5.0);
}

void main( void )
{
	vec4 diffuse_a = color_difference_aa( diffuse_texture, gl_TexCoord[0].xy );
	vec3 diffuse = diffuse_a.rgb;
	
	float spec = diffuse_a.a;
	
	vec4 position = texture2D( positions_texture, gl_TexCoord[0].xy );
	
	vec4 normal_a = texture2D( normals_texture, gl_TexCoord[0].xy );
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
	
	float shadow_amount = ssao(positions_texture, normals_texture, depth_texture, gl_TexCoord[0].xy);
	
	gl_FragColor.rgb = light_amount * diffuse + ambient_amount + spec_amount;
	gl_FragColor.a = 1.0;
	
	//gl_FragColor.rgb *= vec3(shadow_amount, shadow_amount, shadow_amount);
} 