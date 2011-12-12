#include "corange.h"

#include "perlin_noise.h"

#include "noise.h"

int main(int argc, char **argv) {
  
  corange_stop_stdout_redirect();
  
  int x = 512;
  int y = 512;
  int octaves = 8;
  char* filename;
  
  if(argc >= 2) {
    x = atoi(argv[1]);
  }
  if(argc >= 3) {
    y = atoi(argv[2]);
  }
  if(argc >= 4) {
    octaves = atoi(argv[3]);
  }
  if(argc >= 5) {
    filename = malloc(strlen(argv[4]) + 1);
    strcpy(filename, argv[4]);
  } else {
    filename = malloc(strlen("./noise.tga") + 1);
    strcpy(filename, "./noise.tga");
  }
  
  printf("Generating noise texture, resolution %i by %i, with %i octaves\n", x, y, octaves);
  
  image* perlin_noise = perlin_noise_generate(x, y, octaves);
  tga_save_file(perlin_noise, filename);
  
  image_delete(perlin_noise);

  printf("Saved to %s\n", filename);
  
}