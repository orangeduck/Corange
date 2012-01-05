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
  
  level* l = malloc(sizeof(level));
  l->num_tile_sets = 2;
  l->tile_sets = malloc(sizeof(tile_set) * 2);
  l->tile_map = calloc(sizeof(int), MAX_WIDTH * MAX_HEIGHT);
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  char line[MAX_WIDTH];
  int y = 0;
  int x = 0;
  while(SDL_RWreadline(file, line, 1024)) {
    
    for(x = 0; x < strlen(line); x++) {
      char c = line[x];
      if (c == '.') {
        l->tile_map[x + y * MAX_WIDTH] = tiletype_air;
        num_air_tiles++;
      } else if (c == '#') {
        l->tile_map[x + y * MAX_WIDTH] = tiletype_land;
        num_land_tiles++;
      } else if ((c == '\r') || (c == '\n')) {
      
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
      int type = l->tile_map[x + y * MAX_WIDTH];
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
    
    l->tile_sets[i].num_tiles = num_tiles;
    
    glGenBuffers(1, &l->tile_sets[i].positions_buffer);
    glGenBuffers(1, &l->tile_sets[i].texcoords_buffer);
    
    glBindBuffer(GL_ARRAY_BUFFER, l->tile_sets[i].positions_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 4 * num_tiles, position_data, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, l->tile_sets[i].texcoords_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 4 * num_tiles, uv_data, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    free(position_data);
    free(uv_data);
  
  }
    
  return l;
}

void level_delete(level* l) {
  
  int i;
  for(i = 0; i < l->num_tile_sets; i++) {
    glDeleteBuffers(1 , &l->tile_sets[i].positions_buffer);
    glDeleteBuffers(1 , &l->tile_sets[i].texcoords_buffer);
  }
  
  free(l->tile_map);
  free(l->tile_sets);
  free(l);
  
}

void level_render(level* l, vector2 camera_position) {
  
  /* Render Background */
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(0, viewport_width(), 0, viewport_height(), -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  glEnable(GL_TEXTURE_2D);
  
  texture* background = asset_get("./backgrounds/bluesky.dds");
  glBindTexture(GL_TEXTURE_2D, *background);
  
  glBegin(GL_QUADS);
    
    glVertex3f(0, viewport_height(), 0);
    glTexCoord2f(1, 0);
    glVertex3f(viewport_width(), viewport_height(), 0);
    glTexCoord2f(1, 1);
    glVertex3f(viewport_width(), 0, 0);
    glTexCoord2f(0, 1);
    glVertex3f(0, 0, 0);
    glTexCoord2f(0, 0);
    
  glEnd();
  
  glDisable(GL_TEXTURE_2D);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
	glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
  /* Render Tiles */
  
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
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  int i;
  for(i = 0; i < l->num_tile_sets; i++) {
    
    if (i == 0) {
      texture* tile_tex = asset_get("./tiles/tile_sky.dds");
      glBindTexture(GL_TEXTURE_2D, *tile_tex);
    } else if (i == 1) {
      texture* tile_tex = asset_get("./tiles/tile_grass.dds");
      glBindTexture(GL_TEXTURE_2D, *tile_tex);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, l->tile_sets[i].positions_buffer);
    glVertexPointer(3, GL_FLOAT, 0, (void*)0);
    glEnableClientState(GL_VERTEX_ARRAY);
  
    glBindBuffer(GL_ARRAY_BUFFER, l->tile_sets[i].texcoords_buffer);
    glTexCoordPointer(2, GL_FLOAT, 0, (void*)0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  
    glDrawArrays(GL_QUADS, 0, l->tile_sets[i].num_tiles * 4);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);  
    
  }
  
  glDisable(GL_TEXTURE_2D);
  
  glDisable(GL_BLEND);
  
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  
	glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

}

int level_tile_at(level* l, vector2 position) {
  
  int x = floor( position.x / 32 );
  int y = floor( -position.y / 32 );
  
  assert(x >= 0, "Invalid Position, (%0.2f,%0.2f)", position.x, position.y);
  assert(y >= 0, "Invalid Position, (%0.2f,%0.2f)", position.x, position.y);
  assert(x < MAX_WIDTH, "Invalid Position, (%0.2f,%0.2f)", position.x, position.y);
  assert(y < MAX_HEIGHT, "Invalid Position, (%0.2f,%0.2f)", position.x, position.y);
  
  return l->tile_map[x + y * MAX_WIDTH];
}

vector2 level_tile_position(level* l, int x, int y) {
  
  return v2(x * 32, y * 32);
  
}
