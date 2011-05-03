#ifndef font_h
#define font_h

#include "vector.h"
#include "texture.h"

typedef struct {

  texture* texture_map;
  int width;
  int height;
  
  vector2* locations;
  vector2* sizes;
  vector2* offsets;

} font;

font* font_load_file(char* filename);

void parse_char_line(font* f, char* line);

void font_delete(font* font);


#endif