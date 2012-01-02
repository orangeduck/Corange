#include <math.h>

#include "level.h"

const float TILE_WIDTH = 32;
const float TILE_HEIGHT = 32;

const int MAX_WIDTH = 512;
const int MAX_HEIGHT = 512;

const int tiletype_none = 0;
const int tiletype_land = 1;
const int tiletype_air = 2;

level* level_load_file(char* filename) {
  
  int num_air_tiles = 0;
  int num_land_tiles = 0;
  
  level* tl = malloc(sizeof(level));
  tl->num_tile_sets = 2;
  tl->tile_sets = malloc(sizeof(tile_set) * 2);
  tl->tile_map = calloc(sizeof(int), MAX_WIDTH * MAX_HEIGHT);
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  char line[MAX_WIDTH];
  int y = 0;
  int x = 0;
  while(SDL_RWreadline(file, line, 1024)) {
    
    for(x = 0; x < strlen(line); x++) {
      char c = line[x];
      if (c == '.') {
        tl->tile_map[x + y * MAX_WIDTH] = tiletype_air;
        num_air_tiles++;
      } else if (c == '#') {
        tl->tile_map[x + y * MAX_WIDTH] = tiletype_land;
        num_land_tiles++;
      } else {
        error("Unknown tile type for symbol '%c'", c);
      }
    }
    
    y++;
  }
  
  SDL_RWclose(file);
  
  int i;
  for(i = 0; i < 2; i++) {
    
    int char_type = 0;
    int num_tiles = 0;
    if (i == 0) {
      char_type = tiletype_air;
      num_tiles = num_air_tiles;
    } else if(i == 1) {
      char_type = tiletype_land;
      num_tiles = num_land_tiles;
    }
    
    float* position_data = malloc(sizeof(float) * 3 * 4 * num_tiles);
    float* uv_data = malloc(sizeof(float) * 2 * 4 * num_tiles);
    
    int pos_i  = 0;
    int uv_i = 0;
    
    for(x = 0; x < MAX_WIDTH; x++)
    for(y = 0; y < MAX_HEIGHT; y++) {
      int type = tl->tile_map[x + y * MAX_WIDTH];
      if( type == char_type ) {
        
        position_data[pos_i] = x * TILE_WIDTH; pos_i++;
        position_data[pos_i] = y * TILE_HEIGHT; pos_i++;
        position_data[pos_i] = 0; pos_i++;
        
        position_data[pos_i] = (x+1) * TILE_WIDTH; pos_i++;
        position_data[pos_i] = y * TILE_HEIGHT; pos_i++;
        position_data[pos_i] = 0; pos_i++;
        
        position_data[pos_i] = (x+1) * TILE_WIDTH; pos_i++;
        position_data[pos_i] = (y+1) * TILE_HEIGHT; pos_i++;
        position_data[pos_i] = 0; pos_i++;
        
        position_data[pos_i] = x * TILE_WIDTH; pos_i++;
        position_data[pos_i] = (y+1) * TILE_HEIGHT; pos_i++;
        position_data[pos_i] = 0; pos_i++;
        
        uv_data[uv_i] = 0; uv_i++;
        uv_data[uv_i] = 0; uv_i++;
        
        uv_data[uv_i] = 1; uv_i++;
        uv_data[uv_i] = 0; uv_i++;
        
        uv_data[uv_i] = 1; uv_i++;
        uv_data[uv_i] = 1; uv_i++;
        
        uv_data[uv_i] = 0; uv_i++;
        uv_data[uv_i] = 1; uv_i++;
        
      }
    }
    
    tl->tile_sets[i].num_tiles = num_tiles;
    
    glGenBuffers(1, &tl->tile_sets[i].positions_buffer);
    glGenBuffers(1, &tl->tile_sets[i].texcoords_buffer);
    
    glBindBuffer(GL_ARRAY_BUFFER, tl->tile_sets[i].positions_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 4 * num_tiles, position_data, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, tl->tile_sets[i].texcoords_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 4 * num_tiles, uv_data, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    free(position_data);
    free(uv_data);
  
  }
    
  return tl;
}

void level_delete(level* tl) {
  
  int i;
  for(i = 0; i < tl->num_tile_sets; i++) {
    glDeleteBuffers(1 , &tl->tile_sets[i].positions_buffer);
    glDeleteBuffers(1 , &tl->tile_sets[i].texcoords_buffer);
  }
  
  free(tl->tile_map);
  free(tl->tile_sets);
  free(tl);
  
}

void level_render(level* tl, vector2 camera_position) {

	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(camera_position.x - viewport_width() / 2, 
          camera_position.x + viewport_width() / 2,
          -camera_position.y + viewport_height() / 2,
          -camera_position.y - viewport_height() / 2
          , -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glEnable(GL_TEXTURE_2D);
  
  int i;
  for(i = 0; i < tl->num_tile_sets; i++) {
    
    if (i == 0) {
      texture* tile_tex = asset_get("./tiles/tile_sky.dds");
      glBindTexture(GL_TEXTURE_2D, *tile_tex);
    } else if (i == 1) {
      texture* tile_tex = asset_get("./tiles/tile_grass.dds");
      glBindTexture(GL_TEXTURE_2D, *tile_tex);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, tl->tile_sets[i].positions_buffer);
    glVertexPointer(3, GL_FLOAT, 0, (void*)0);
    glEnableClientState(GL_VERTEX_ARRAY);
  
    glBindBuffer(GL_ARRAY_BUFFER, tl->tile_sets[i].texcoords_buffer);
    glTexCoordPointer(2, GL_FLOAT, 0, (void*)0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  
    glDrawArrays(GL_QUADS, 0, tl->tile_sets[i].num_tiles * 4);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);  
  
  }
  
  glDisable(GL_TEXTURE_2D);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
	glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

}
