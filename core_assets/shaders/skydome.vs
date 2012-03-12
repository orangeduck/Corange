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

uniform vec3 light_direction;

varying vec3 light;
varying vec3 direction;

#define SAMPLES 5

void main() {
  
  /* TODO: Fill in wavelengths */
  vec3 k_ray = (4 * 3.141) / pow(vec3(1,1,1), 4.00);
  vec3 k_mei = (4 * 3.141) / pow(vec3(1,1,1), 0.84);
  
  vec3 ray_direction = normalize(gl_Vertex.xyz);
  direction = -ray_direction;
  
  float sample_len = 1.0 / float(SAMPLES);
  
  vec3 sample_ray = ray_direction * sample_len;
  vec3 sample_point = sample_ray * 0.5;
  
  light = vec3(0,0,0);
  for(int i = 0; i < SAMPLES; i++) {
    
    float height = i * sample_len;
    //float depth = exp(-height/0.25);
    
    float light_angle = dot(light_direction, sample_point) / height;
    float camera_angle = dot(ray_direction, sample_point) / height;
    
    //float scatter = depth * (scale(light_angle) * scale(camera_angle));
    //float attentuate = exp(-scatter * (k_ray + k_mei));
    
    //light += attentuate * depth * sample_len;
    
    sample_point += sample_ray;
  }
  
  gl_Position = proj_matrix * view_matrix * world_matrix * gl_Vertex;
}
