#include "assets/font.h"

font* font_load_file(char* filename) {
  
  font* f = malloc(sizeof(font));
  f->width = 0;
  f->height = 0;
  
  /* Encodes ASCII */
  f->locations = malloc( sizeof(vec2) * 256 );
  f->sizes = malloc( sizeof(vec2) * 256 );
  f->offsets = malloc( sizeof(vec2) * 256 );
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  
  if(file == NULL) {
    error("Could not load file %s", filename);
  }
  
  char line[1024];
  while(SDL_RWreadline(file, line, 1024)) {
    
    int tex_id;
    char tex_file[MAX_PATH];
    if (sscanf(line, "page id=%i file=%s", &tex_id, tex_file) > 0) {
      
      fpath location;
      SDL_PathFileLocation(location.ptr, filename);
      
      /* +1 to remove beginning quotation */
      strcat(location.ptr, tex_file+1); 
      
      /* remove ending quotation */
      location.ptr[strlen(location.ptr)-1] = '\0';
      
      f->texture_map = asset_hndl_new(location);
    }
    
    int lineheight, base, scalew, scaleh;
    int pages, packed, a_chan, r_chan, g_chan, b_chan;
    if (sscanf(line, "common lineHeight=%i base=%i scaleW=%i scaleH=%i "
                     "pages=%i packed=%i alphaChnl=%i "
                     "redChnl=%i greenChnl=%i blueChnl=%i", 
                     &lineheight, &base, &scalew, &scaleh, 
                     &pages, &packed, &a_chan, 
                     &r_chan, &g_chan, &b_chan) > 0) {
                     
      f->width = scalew;
      f->height = scaleh;
    }
    
    int id, x, y, w, h, x_off, y_off, x_adv, page, chnl;
    if (sscanf(line, "char id=%i x=%i y=%i width=%i height=%i "
                     "xoffset=%i yoffset=%i xadvance=%i page=%i chnl=%i", 
                     &id, &x, &y, &w, &h, &x_off, 
                     &y_off, &x_adv, &page, &chnl) > 0) {
      
      f->locations[id] = vec2_new((float)x / f->width, (float)y / f->height);
      f->sizes[id] = vec2_new((float)w / f->width, (float)h / f->height);
      f->offsets[id] = vec2_new((float)x_off / f->width, (float)y_off / f->height);
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
