#include "tile_level.h"

tile_level* level_load_file(char* filename) {
  
  int MAX_WIDTH = 512;
  int MAX_HEIGHT = 512;
  
  int tiletype_none = 0;
  int tiletype_land = 1;
  int tiletype_air = 2;
  
  int* tilemap = calloc(sizeof(int), MAX_WIDTH * MAX_HEIGHT);
  int num_tiles = 0;
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  char line[MAX_WIDTH];
  int y = 0;
  int x = 0;
  while(SDL_RWreadline(file, line, 1024)) {
    
    for(x = 0; x < strlen(line); x++) {
      char c = line[x];
      if (c == '.') {
        tilemap[x + y * MAX_WIDTH] = tiletype_air;
        num_tiles++;
      } else if (c == '#') {
        tilemap[x + y * MAX_WIDTH] = tiletype_land;
        num_tiles++;
      } else {
        error("Unknown tile type for symbol '%c'", c);
      }
    }
    
    y++;
  }
  
  SDL_RWclose(file);
  
  float* position_data = malloc(sizeof(float) * 3 * 4 * num_tiles);
  float* uv_data = malloc(sizeof(float) * 2 * 4 * num_tiles);
  
  int pos_i  = 0;
  int uv_i = 0;
  
  for(x = 0; x < MAX_WIDTH; x++)
  for(y = 0; y < MAX_HEIGHT; y++) {
    int type = tilemap[x + y * MAX_WIDTH];
    if( type != tiletype_none ) {
      
      position_data[pos_i] = ; pos_i++;
      position_data[pos_i] = ; pos_i++;
      position_data[pos_i] = ; pos_i++;
      
      position_data[pos_i] = ; pos_i++;
      position_data[pos_i] = ; pos_i++;
      position_data[pos_i] = ; pos_i++;
      
      position_data[pos_i] = ; pos_i++;
      position_data[pos_i] = ; pos_i++;
      position_data[pos_i] = ; pos_i++;
      
      position_data[pos_i] = ; pos_i++;
      position_data[pos_i] = ; pos_i++;
      position_data[pos_i] = ; pos_i++;
      
      uv_data[uv_i] = ; uv_i++;
      uv_data[uv_i] = ; uv_i++;
      
      uv_data[uv_i] = ; uv_i++;
      uv_data[uv_i] = ; uv_i++;
      
      uv_data[uv_i] = ; uv_i++;
      uv_data[uv_i] = ; uv_i++;
      
      uv_data[uv_i] = ; uv_i++;
      uv_data[uv_i] = ; uv_i++;
      
    }
  }
  
  free(tilemap);
  
  glGenBuffers(1, &s->vertex_vbo);
  glGenBuffers(1, &s->triangle_vbo);
  
  free(position_data);
  free(uv_data);
    
  
}

void tile_level_delete(tile_level* tl) {

}
