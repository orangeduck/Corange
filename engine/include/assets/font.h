/**
*** :: Font ::
***
***   Fonts created with Bitmap Font Generator
***
***   http://www.angelcode.com/products/bmfont/
***
**/

#ifndef font_h
#define font_h

#include "cengine.h"
#include "casset.h"

typedef struct {
  asset_hndl texture_map;
  int width;
  int height;
  
  vec2* locations;
  vec2* sizes;
  vec2* offsets;
} font;

font* font_load_file(char* filename);
void font_delete(font* font);


#endif