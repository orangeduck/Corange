#include <math.h>
#include <limits.h>
#include <time.h>

#include "perlin_noise.h"

/*
    Perlin Noise
  
  This is a slight variation on the normal perlin noise which is more expensive, but in my mind slightly more simple and elegant.
  Instead of using a lookup table or something similar to generate random 2D "gradients" at integer positions in a space I instead use a mix hash.
  This hash generates an integer which is indestinguishable from random data.
  This interger is then interpolated between as normal using Ken Perlin's smootherstep polynomial.
  
  So the perlin noise functions output a floating point number between -1 and 1 which is constant on input. Variable or randomness can be achieved using a seeded offset from (0,0)
  Integer values are random noise, while the distances inbetween them are interpolated. Overlaying several octaves gives the standard perlin noise look.
  
  In the 2D generation four samples are taken for each pixel. This is slower but makes the output texture tileable.

*/
  
float perlin_noise2D(vec2 v) {
  
  vec2 amount = vec2_fmod(v,1.0);
  
  vec2 p1 = vec2_floor(v);
  vec2 p2 = vec2_add(p1, vec2_new(1,0) );
  vec2 p3 = vec2_add(p1, vec2_new(0,1) );
  vec2 p4 = vec2_add(p1, vec2_new(1,1) );
  
  int h1 = vec2_mix_hash(p1);
  int h2 = vec2_mix_hash(p2);
  int h3 = vec2_mix_hash(p3);
  int h4 = vec2_mix_hash(p4);
  
  double result = bismootherstep_interp(h2, h1, h4, h3, amount.x, amount.y);
  
  return result / INT_MAX;
  
}


image* perlin_noise_generate(int x_size, int y_size, int octaves) {
  
  unsigned char* data = malloc(sizeof(char) * x_size * y_size);
  
  int i, x, y;
  
  /* Clear data to average */
  for(x = 0; x < x_size; x++)
  for(y = 0; y < y_size; y++) {
    data[x + (y * x_size)] = 128;
  }
  
  srand(time(NULL));
  
  debug("Generating perlin noise.");
  
  for(i = 0; i < octaves; i++ ) {
  
    float wavelength = pow( 2, i);
    float amplitude = pow( 0.5, octaves-i );
    vec2 seed = vec2_new(rand(),rand());
    
    debug("Octave: %i Wavelength: %f Amplitude: %f", i, wavelength, amplitude);

    for(x = 0; x < x_size; x++)
    for(y = 0; y < y_size; y++) {
      
      /* Four positions are required for tiling behaviour */
      
      vec2 pos, pos_x, pos_y, pos_xy;
      
      pos = vec2_div( vec2_new(x, y) , wavelength );
      pos_x = vec2_div( vec2_new(x - x_size, y) , wavelength );
      pos_y = vec2_div( vec2_new(x, y - y_size) , wavelength );
      pos_xy = vec2_div( vec2_new(x - x_size, y - y_size) , wavelength );
      
      pos = vec2_add( pos, seed );
      pos_x = vec2_add( pos_x, seed );
      pos_y = vec2_add( pos_y, seed );
      pos_xy = vec2_add( pos_xy, seed );
      
      float val = perlin_noise2D(pos) * amplitude;
      float val_x = perlin_noise2D(pos_x) * amplitude;
      float val_y = perlin_noise2D(pos_y) * amplitude;
      float val_xy = perlin_noise2D(pos_xy) * amplitude;
      
      val = bilinear_interp(val_x, val, val_xy, val_y, (float)x/x_size, (float)y/y_size);
      
      data[x + (y * x_size)] += val * 128;
    }
  
  }
  
  unsigned char* image_data = malloc(sizeof(char) * 4 * x_size * y_size);
  for(x = 0; x < x_size; x++)
  for(y = 0; y < y_size; y++) {
    int amount = data[x + y * x_size];
    image_data[x * 4 + y * x_size * 4 + 0] = amount;
    image_data[x * 4 + y * x_size * 4 + 1] = amount;
    image_data[x * 4 + y * x_size * 4 + 2] = amount;
    image_data[x * 4 + y * x_size * 4 + 3] = 255;
  }
  
  free(data);
  
  return image_new(x_size, y_size, image_data);
}
