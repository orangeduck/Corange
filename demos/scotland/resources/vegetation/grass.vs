uniform mat4 world_matrix;
uniform mat4 proj_matrix;
uniform mat4 view_matrix;

uniform vec3 camera_position;

uniform sampler2D terrain_color;
uniform sampler2D terrain_normals;

#define MAX_LIGHTS 32

uniform int num_lights;
uniform float light_power[MAX_LIGHTS];
uniform float light_falloff[MAX_LIGHTS];
uniform vec3 light_position[MAX_LIGHTS];
uniform vec3 light_target[MAX_LIGHTS];
uniform vec3 light_diffuse[MAX_LIGHTS];
uniform vec3 light_ambient[MAX_LIGHTS];
uniform vec3 light_specular[MAX_LIGHTS];

varying float alpha_test;
varying vec3 color;
varying vec2 uvs;

vec3 to_gamma(vec3 color) {
  vec3 ret;
  ret.r = pow(color.r, 2.2);
  ret.g = pow(color.g, 2.2);
  ret.b = pow(color.b, 2.2);
	return ret;
}

vec3 from_gamma(vec3 color) {
  vec3 ret;
  ret.r = pow(color.r, 1.0/2.2);
  ret.g = pow(color.g, 1.0/2.2);
  ret.b = pow(color.b, 1.0/2.2);
	return ret;
}

void main() {
  
  uvs = vec2(gl_MultiTexCoord0.x, 1-gl_MultiTexCoord0.y);
  
  vec4 world_position = world_matrix * gl_Vertex;
  vec3 position = world_position.xyz / world_position.w;
  gl_Position = proj_matrix * view_matrix * world_position;
  
  alpha_test = clamp(distance(camera_position, position) / 45, 0.1, 1.1);
  
  vec2 global_uvs = position.xz / 1024;
  vec3 albedo = from_gamma(texture2D(terrain_color, global_uvs));
  vec3 normal = texture2D(terrain_normals, global_uvs);
  
  float temp = normal.g;
  normal.g = 1-normal.b;
  normal.b = temp;
  normal.r = 1-normal.r;
  normal = (normal * 2.0) - 1.0;
  
  vec3 diffuse = vec3(0,0,0);
  vec3 ambient = vec3(0,0,0);
  
  for(int i = 0; i < num_lights; i++) {
    vec3 light_vector = light_position[i] - position;
    float power = light_power[i] / pow(length(light_vector), light_falloff[i]);
    
    vec3 light_dir = normalize(light_target[i] - light_position[i]);
    
    float n_dot_l = max( dot( normal, light_dir) , 0.0);
    
    diffuse += power * light_diffuse[i] * albedo * n_dot_l;
    ambient += power * light_ambient[i] * albedo;
  }
  
  color = diffuse + ambient;
  
}
