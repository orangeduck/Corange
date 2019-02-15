#version 120

uniform sampler2D diffuse_map;
uniform sampler2D normal_map;
uniform sampler2D specular_map;
uniform sampler2D parallax_map;

uniform vec3  camera_position;
uniform float parallax_level;
uniform float glossiness_level;
uniform float specular_level;

uniform float clip_near;
uniform float clip_far;

varying vec2 fTexcoord;
varying vec3 fColor;
varying vec3 fPosition;

varying vec3 Tpos;
varying vec3 Tcam;


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

vec3 swap_red_green_inv(vec3 color) {
  float temp = 1.0-color.r;
  color.r = 1.0-color.g;
  color.g = temp;
  return color;
}


float linear_depth(float depth, float near, float far){
  return (2.0 * near) / (far + near - depth * (far - near));
}

vec2 parallax_uv(float type,vec2 uv, vec3 view_dir)
{
  if(type == 0)
  {
    //Simple Parallax mapping
    float dep = texture2D(parallax_map,uv).z;
    vec2  uvn = view_dir.xy / view_dir.z * (dep * parallax_level);
    return uv - uvn ;
  };

  float num_layers = 128;
  float layer_depth = 1.0 / num_layers;
  float cur_layer_depth = 0.0;
  vec2 delta_uv = view_dir.xy / view_dir.z * parallax_level / num_layers;
  vec2 cur_uv = uv;
  float depth_from_tex = texture2D(parallax_map, cur_uv).r;
  for (int i = 0; i < num_layers; i++) {
      cur_layer_depth += layer_depth;
      cur_uv -= delta_uv;
      depth_from_tex = texture2D(parallax_map, cur_uv).r;
      if (depth_from_tex < cur_layer_depth){
          break;
      }
  }
  if(type == 1)
  {
    //Step Parallax mapping
    return cur_uv;
  }

  if(type == 2)
  {
    // Parallax occlusion mapping
    vec2 prev_uv = cur_uv + delta_uv;
    float next = depth_from_tex - cur_layer_depth;
    float prev = texture2D(parallax_map, prev_uv).r - cur_layer_depth
               + layer_depth;
    float weight = next / (next - prev);
    return mix(cur_uv, prev_uv, weight);
  }
}

void main( void ) 
{
vec2 uvs = vec2(fTexcoord.x, -fTexcoord.y);
vec3 viewDir   = normalize(Tcam - Tpos);
//replace camera x y position
vec3 viewDirInv = vec3(viewDir.y,viewDir.x,viewDir.z);
//invert camera y bacouse coord y is nimus
viewDirInv.y = -viewDirInv.y;
//offset texture coordinates with Parallax Mapping
uvs = parallax_uv(2,uvs,  viewDirInv);
//get textures
vec4 diffuse  = texture2D(diffuse_map, uvs);
vec4 specular = texture2D(specular_map, uvs) * specular_level;
vec4 normal   = texture2D(normal_map, uvs);
diffuse.rgb = from_gamma(diffuse.rgb) * fColor * specular.r;
//set very simple light and normal mapping
vec3  norm = normalize(normal.rgb * 2.0 - 1.0)* glossiness_level;
vec3  light_dir = normalize(camera_position + vec3(10) - fPosition);
float light_val = max(dot(light_dir, norm),0.0);
vec3  ambient   = 0.3 * diffuse.rgb;

gl_FragColor = vec4(light_val * diffuse.rgb + ambient, 1.0);
gl_FragDepth = linear_depth(gl_FragCoord.z, clip_near, clip_far);
















}

