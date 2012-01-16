#include <string.h>

#include "SDL/SDL_rwops.h"
#include "SDL/SDL_local.h"

#include "error.h"
#include "asset_manager.h"

#include "font.h"

font* font_load_file(char* filename) {
  
  font* f = malloc(sizeof(font));
  
  /* Encodes ASCII, not unicode */
  f->locations = malloc( sizeof(vector2) * 256 );
  f->sizes = malloc( sizeof(vector2) * 256 );
  f->offsets = malloc( sizeof(vector2) * 256 );

  f->texture_map = 0;
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  
  if(file == NULL) {
    error("Could not load file %s", filename);
  }
  
  char line[1024];
  while(SDL_RWreadline(file, line, 1024)) {
    
    int tex_id;
    char texture_filename[1024];
    if (sscanf(line, "page id=%i file=\%s", &tex_id, texture_filename) > 0) {
      
      char* root = asset_name_location(filename);
      
      char* full = malloc(strlen(root) + strlen(texture_filename) + 1 );
      strcpy(full, root);
      
      strcat(full, texture_filename+1); /* +1 to remove beginning quotation */
      full[strlen(full)-1] = '\0'; /* remove ending quotation */
      
      if(asset_loaded(full)) {
        f->texture_map = asset_get(full);
      } else {
        load_file(full);
        f->texture_map = asset_get(full);
      }
      
      free(root);
      free(full);
    }
    
    int lineheight, base, scalew, scaleh, pages, packed, a_chan, r_chan, g_chan, b_chan;
    if (sscanf(line, "common lineHeight=%i base=%i scaleW=%i scaleH=%i pages=%i packed=%i alphaChnl=%i redChnl=%i greenChnl=%i blueChnl=%i", &lineheight, &base, &scalew, &scaleh, &pages, &packed, &a_chan, &r_chan, &g_chan, &b_chan) > 0) {
      
      f->width = scalew;
      f->height = scaleh;
      
    }
    
    int id, x, y, w, h, x_off, y_off, x_adv, page, chnl;
    if (sscanf(line, "char id=%i x=%i y=%i width=%i height=%i xoffset=%i yoffset=%i xadvance=%i page=%i chnl=%i", &id, &x, &y, &w, &h, &x_off, &y_off, &x_adv, &page, &chnl) > 0) {
      
      f->locations[id] = v2((float)x / f->width, (float)y / f->height);
      f->sizes[id] = v2((float)w / f->width, (float)h / f->height);
      f->offsets[id] = v2((float)x_off / f->width, (float)y_off / f->height);
    
    }
    
  }
  
  SDL_RWclose(file);
  
  return f;
  
}

void font_delete(font* f) {
  
  free(f->locations);
  free(f->sizes);
  free(f->offsets);
  
  free(f);
}