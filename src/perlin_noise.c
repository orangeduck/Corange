#include <math.h>
#include <limits.h>

#include "perlin_noise.h"

#include "texture.h"
#include "vector.h"

float lerp(float p1, float p2, float amount) {
  return (p1 * amount) + (p2 * (1-amount));
}

float smoothstep(float p1, float p2, float amount) {
  float scaled_amount = amount*amount*(3 - 2*amount);
  return lerp( p1, p2, scaled_amount );
}

float smootherstep(float p1, float p2, float amount) {
  float scaled_amount = amount*amount*amount*(amount*(amount*6 - 15) + 10);
  return lerp( p1, p2, scaled_amount );
}

float cosine_interpolation(float p1, float p2, float amount) {
   float mu2 = (1-cos(amount*3.1415926))/2;
   return(p2*(1-mu2)+p1*mu2);
}

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
  
  double side1 = smootherstep(h2, h1, amount.x);
  double side2 = smootherstep(h4, h3, amount.x);
  
  double final = smootherstep(side2, side1, amount.y);
    
  float ret = final / INT_MAX;
  
  //printf("%f,",ret);
  
  return ret;
  
}


texture* perlin_noise_generate_texture2D(int x_size, int y_size, int octaves) {
  
  unsigned char* data = malloc(sizeof(char) * x_size * y_size);
  
  int i, x, y;
  
  /* Clear data to average */
  for(x = 0; x < x_size; x++)
  for(y = 0; y < y_size; y++) {
    data[x + (y * x_size)] = 128;
  }
  
  srand(time(NULL));
  
  for(i = 0; i < octaves; i++ ) {
  
    float wavelength = pow( 2, i);
    float amplitude = pow( 0.5, octaves-i );
    vector2 seed = v2(rand(),rand());
    
    printf("Octave: %i Wavelength: %f Amplitude: %f\n", i, wavelength, amplitude);

    for(x = 0; x < x_size; x++)
    for(y = 0; y < y_size; y++) {
      
      /* Four positions are required for tiling behaviour */
      
      vector2 pos, pos_x, pos_y, pos_xy;
      
      pos = v2_div( v2(x,y) , wavelength );
      pos_x = v2_div( v2(x - x_size,y) , wavelength );
      pos_y = v2_div( v2(x,y - y_size) , wavelength );
      pos_xy = v2_div( v2(x - x_size,y - y_size) , wavelength );
      
      pos = v2_add( pos, seed );
      pos_x = v2_add( pos_x, seed );
      pos_y = v2_add( pos_y, seed );
      pos_xy = v2_add( pos_xy, seed );
      
      float val = perlin_noise2D(pos) * amplitude;
      float val_x = perlin_noise2D(pos_x) * amplitude;
      float val_y = perlin_noise2D(pos_y) * amplitude;
      float val_xy = perlin_noise2D(pos_xy) * amplitude;
      
      val = lerp( val_x, val, (float)x / x_size);
      val_y = lerp( val_xy, val_y, (float)x / x_size );
      
      val = lerp( val_y, val, (float)y / y_size);
      
      int byte_val = val * 128;
      
      data[x + (y * x_size)] += byte_val;
    }
  
  }
  
  texture* t = texture_new();
  glBindTexture(GL_TEXTURE_2D, *t);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x_size, y_size, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data );
  
  free(data);
  
  return t;
}
