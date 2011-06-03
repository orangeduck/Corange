#include <string.h>

#include "asset_manager.h"

#include "font.h"

font* font_load_file(char* filename) {
  
  font* f = malloc(sizeof(font));
  
  /* Encodes ASCII, not unicode */
  f->locations = malloc( sizeof(vector2) * 256 );
  f->sizes = malloc( sizeof(vector2) * 256 );
  f->offsets = malloc( sizeof(vector2) * 256 );

  f->texture_map = 0;
  
  char* c = asset_load_file(filename);
  
  /* Begin parsing font data */
  
  char* line = malloc(1024);
  
  int i = 0;
  int j = 0;
  
  while(1) {
  
    /* If end of string then exit. */
    if( c[i] == '\0') { break; }
    
    /* End of line reached */
    if( c[i] == '\n') {
    
      /* Null terminate line buffer */
      line[j-1] = '\0';
      
      //printf("LINE: %s \n",line);
      
      /* Might be nice to change this later */
      if (line[0] == 'p' && line[1] == 'a' && line[2] == 'g' && line[3] == 'e') {
      
        char* tex = strstr(line, "file=") + 6;
        int k = 0;
        while(tex[k] != '"') k++;
        
        char* root = asset_file_location(filename);
        
        char* filename = malloc(k+1);
        memcpy(filename, tex, k);
        filename[k] = '\0';
        
        char* full = malloc(strlen(root) + strlen(filename) + 1 );
        strcpy(full, root);
        strcat(full, filename);
        
        if(asset_loaded(full)) {
          f->texture_map = (texture*)asset_get(full);
        } else {
          load_file(full);
          f->texture_map = (texture*)asset_get(full);
        }
        
        
        free(root);
        free(filename);
        free(full);
        
      } else if (line[0] == 'c' && line[1] == 'o' && line[2] == 'm') {
      
        char* sizeline;
        sizeline = strstr(line, "scaleW=") + 7;
        f->width = strtoul(sizeline, NULL, 0);
        
        sizeline = strstr(line, "scaleH=") + 7;
        f->height = strtoul(sizeline, NULL, 0);
        
      } else if (line[0] == 'c' && line[1] == 'h' && line[2] == 'a' && line[3] == 'r' && line[4] == ' ') {
        
        parse_char_line(f, line);
      
      }
      
      /* Reset line buffer index */
      j = 0;
      
    } else {
    
      /* Otherwise add character to line buffer */
      line[j] = c[i];
      j++;
    }
    i++;
  }
  
  free(line);
  
  /* End parsing font data */
  
  free(c);
  
  return f;
  
};

void parse_char_line(font* f, char* c) {
  
  int i = 0;
  char* end;
  
  while (*c != '=' ) c++; c++;
  int id = strtoul(c, &end, 0);
  c = end;
  
  while (*c != '=' ) c++; c++;
  int x_loc = strtoul(c, &end, 0);
  c = end;
  
  while (*c != '=' ) c++; c++;
  int y_loc = strtoul(c, &end, 0);
  c = end;

  while (*c != '=' ) c++; c++;
  int width = strtoul(c, &end, 0);
  c = end;
  
  while (*c != '=' ) c++; c++;
  int height = strtoul(c, &end, 0);
  c = end;
  
  while (*c != '=' ) c++; c++;
  int x_off = strtoul(c, &end, 0);
  c = end;
  
  while (*c != '=' ) c++; c++;
  int y_off = strtoul(c, &end, 0);
  c = end;
  
  //printf("Line Details: %i %i %i %i %i %i %i \n", id, x_loc, y_loc, width, height, x_off, y_off);
  
  f->locations[id] = v2((float)x_loc / f->width, (float)y_loc / f->height);
  f->sizes[id] = v2((float)width / f->width, (float)height / f->height);
  f->offsets[id] = v2((float)x_off / f->width, (float)y_off / f->height);
  
}

void font_delete(font* f) {
  
  texture_delete(f->texture_map);

  free(f->locations);
  free(f->sizes);
  free(f->offsets);
  
  free(f);
};