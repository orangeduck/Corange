#include "tile_level.h"

tile_level* level_load_file(char* filename) {
  
  int MAX_WIDTH = 512;
  int MAX_HEIGHT = 512;
  
  int tiletype_land = 1;
  int tiletype_air = 2;
  
  int* tilemap = calloc(sizeof(int), MAX_WIDTH * MAX_HEIGHT);
  
  
  
  free(tilemap);
  
    
  
}

void tile_level_delete(tile_level* tl) {

}
