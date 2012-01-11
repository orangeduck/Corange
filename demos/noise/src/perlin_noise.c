#include <math.h>
#include <limits.h>
#include <time.h>

#include "perlin_noise.h"

#include "texture.h"
#include "vector.h"

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
  
float perlin_noise2D(vector2 v) {
  
  vector2 amount = v2_fmod(v,1.0);
  
  vector2 p1 = v2_floor(v);
  vector2 p2 = v2_add(p1, v2(1,0) );
  vector2 p3 = v2_add(p1, v2(0,1) );
  vector2 p4 = v2_add(p1, v2(1,1) );
  
  int h1 = v2_mix_hash(p1);
  int h2 = v2_mix_hash(p2);
  int h3 = v2_mix_hash(p3);
  int h4 = v2_mix_hash(p4);
  
  double result = bismootherstep_interpolation(h2, h1, h4, h3, amount.x, amount.y);
  
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
    vector2 seed = v2(rand(),rand());
    
    debug("Octave: %i Wavelength: %f Amplitude: %f", i, wavelength, amplitude);

    for(x = 0; x < x_size; x++)
    for(y = 0; y < y_size; y++) {
      
      /* Four positions are required for tiling behaviour */
      
      vector2 pos, pos_x, pos_y, pos_xy;
      
      pos = v2_div( v2(x, y) , wavelength );
      pos_x = v2_div( v2(x - x_size, y) , wavelength );
      pos_y = v2_div( v2(x, y - y_size) , wavelength );
      pos_xy = v2_div( v2(x - x_size, y - y_size) , wavelength );
      
      pos = v2_add( pos, seed );
      pos_x = v2_add( pos_x, seed );
      pos_y = v2_add( pos_y, seed );
      pos_xy = v2_add( pos_xy, seed );
      
      float val = perlin_noise2D(pos) * amplitude;
      float val_x = perlin_noise2D(pos_x) * amplitude;
      float val_y = perlin_noise2D(pos_y) * amplitude;
      float val_xy = perlin_noise2D(pos_xy) * amplitude;
      
      val = bilinear_interpolation(val_x, val, val_xy, val_y, (float)x/x_size, (float)y/y_size);
      
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
