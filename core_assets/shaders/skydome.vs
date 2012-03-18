/*

  Draw vector from camera to atmosphere
  Sample N points along ray.
  Get light intensity at point.
  
  Multiply intensity by ray length.
  
  To get intensity there are two types of scattering Rayleigh and Mei
  These rely on a phase function which takes:
    An angle to the light from ray point.
    A constant which effects how scattering works.
  
  Phase function: 
    
      F(a, g) =  3*(1-g^2)               1 + cos^2(a)
                -----------  *  ----------------------------
                 2*(2+g^2)      (1 + g^2 - 2*g*cos(a))^(3/2)

    For rayleigh scattering you can set g to 0 so...
    
      RayLeighF(a) = (3/4) * (1 + cos^2(a))
    
    For Mie you can set g to between -0.75 and -0.999
    
      MieF(a) = (1.3125 / 5.125) * (1 + cos^2(a) / (1.5625 + 1.4*cos(a))^(3/2))
    
    And remember that cos(a) = dot(norm(L), norm(D)) and cos^2(a) = cos(a) * cos(a)
    
    The phase function can be done per-pixel fairly easily.
    The phase function can also be separated by wavelengths and intensities for rayleigh.
    
    So at each pixel we have
      P = I(ray) * K(ray) * RayLeighF(a)
        + I(mie) * K(mie) * MieF(a)
    
    Where K(ray) and K(mie) are the scattering constants independant for colors.
      K(ray) = 1 / w^4
      K(mie) = 1 / w^0.84
    
    Now we just need to work out I(ray), and I(mie) in the vertex shader.
    
    
  The out scattering equation:
  
    This equation says how much light is scattered away from a section of the ray.
    
    t(Pa, Pb, l) = 4pi * K(l) * Integrate_Pa^Pb( exp(-h/H0) ) ds
    
    A good value for H0 which is the "scale height" or average height for atmospheric density is 0.25 (where the total height is 1). So we get
    
    t(Pa, Pb) = 4pi * K(l) * Integrate_Pa^Pb(exp(-h / 0.25)) ds
      
      where h is the height of the sample point.
  
  
  The in scattering equation:
    
    So this is the full thing. It gives the amount of light scattered into the camera across a full ray Pa->Pb. This is what needs to be done per vertex
    
      I(l) = Is(l) * K(l) * F(a, g) * Integral_Pa^Pb( exp(-h/H0) * exp(-t(PPc, l)-t(PPa, l) )) ds
    
    But again there are a number of simplications. To think about. First let us forget about the constants as we are doing them in the pixel shader, for example Is is sunlight function. Secondly we have a simplified t function. So it looks more like this:
    
      I = Integral_Pa^Pb( exp(-h/0.25) * exp(-t(PPc)-t(PPa)) )
    
    Where PPc is from point to sun and PPa is point to camera.
    
    
  The surface scattering equation:
  
    There is another minor equation for surface scattering. And can be Calculated per vertex for the whole ray.
    
      Isur = Ie * exp(-t(PaPb))
    
    Where Ie is the amount of surface reflectance. And PaPb is the full ray.
    
  
  So what is clear from these results is that basically what needs to be accelerated is the out scattering equation. If the out scattering equation could be solved easily then the rest would fall into place.
  
  So the approach people have taken for this function is to either use a lookup table or to analyse the lookup table and create some appropriate functions. Most of these lookups or functions require an input of some height and some angle.
  
  See: http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter16.html
  For how it was done.
  
  
  --------------------
  
  So, there are two things that need to be solved:
    
    t(Pa, Pb) = 4pi * K(l) * Integrate_Pa^Pb(exp(-h / 0.25)) ds
    
    For when PaPb is the vector from Point to Camera
    For when PaPb is the vector from Point to Sun
  
    And for these vectors all we require is the height of the point along the integral.
  
  
*/

uniform mat4 world_matrix;
uniform mat4 proj_matrix;
uniform mat4 view_matrix;

#define MAX_LIGHTS 32

uniform int num_lights;
uniform float light_power[MAX_LIGHTS];
uniform float light_falloff[MAX_LIGHTS];
uniform vec3 light_position[MAX_LIGHTS];
uniform vec3 light_target[MAX_LIGHTS];
uniform vec3 light_diffuse[MAX_LIGHTS];
uniform vec3 light_ambient[MAX_LIGHTS];
uniform vec3 light_specular[MAX_LIGHTS];

uniform vec3 camera_position;

varying vec3 direction;
varying vec3 m_color;
varying vec3 r_color;
varying vec3 debug;

const int nsamples = 2;
const float fsamples = 2.0;

const vec3 inv_wavelength = 1 / pow(vec3(0.620,0.495,0.475), 4);

const float outer_radius = 10.25;
const float inner_radius = 10.00;

const vec3 esun = vec3(100.0, 100.0, 100.0);
const float kr = 0.0025;
const float km = 0.0010;

const vec3 kresun = kr * esun;
const vec3 kmesun = km * esun;

const float kr4pi = kr * 4 * 3.141;
const float km4pi = km * 4 * 3.141;

const float scale = 1 / (outer_radius - inner_radius);
const float scale_depth = 0.25;
const float scale_over_depth = scale / scale_depth;

/* Analytical scale function */
float scalefunc(float angle) {
  float x = 1-angle;
	return scale_depth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
}

void main() {
  
  vec3 light_direction = -normalize(light_target[0] - light_position[0]);
  
  camera_position.y = camera_position.y / 2048 + inner_radius;
  camera_position.xz = (camera_position.xz - 512) / 2048;
  
	vec3 position;
  position.x = gl_Vertex.x * 4.0 * (outer_radius - inner_radius);
  position.z = gl_Vertex.z * 4.0 * (outer_radius - inner_radius);
  position.y = inner_radius + (gl_Vertex.y * (outer_radius - inner_radius));
  
	vec3 ray = position - camera_position;
	float far = length(ray);
	ray /= far;

	vec3 start = camera_position;
	float height = length(start);
	float depth = exp(scale_over_depth * (inner_radius - height));
	float start_angle = dot(ray, start) / height;
	float start_offset = depth * scalefunc(start_angle);

	float sample_length = far / fsamples;
	float scaled_length = sample_length * scale;
	vec3 sample_ray = ray * sample_length;
	vec3 sample_point = start + sample_ray * 0.5;

	vec3 front_color = vec3(0.0, 0.0, 0.0);
	for(int i=0; i<nsamples; i++) {
		float height = length(sample_point);
		float depth = exp(scale_over_depth * (inner_radius - height));
		float light_angle = dot(light_direction, sample_point) / height;
		float camera_angle = dot(ray, sample_point) / height;
		float scatter = (start_offset + depth*(scalefunc(light_angle) - scalefunc(camera_angle)));
		vec3 attenuate = exp(-scatter * (inv_wavelength * kr4pi + km4pi));
		front_color += attenuate * (depth * scaled_length);
		sample_point += sample_ray;
	}

	m_color = front_color * kmesun;
	r_color = front_color * (inv_wavelength * kresun);
  
  direction = camera_position - position;
  
	gl_Position = proj_matrix * view_matrix * world_matrix * gl_Vertex;
}
