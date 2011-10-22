#ifndef brush_dict_h
#define brush_dict_h

#include "image.h"
#include "vector.h"
#include "texture.h"

void brush_dict_init(texture* brush_tex, int num_x, int num_y, int total) ;
void brush_dict_finish();

int brush_id_for_position( image* ref, image* stencil, vector2 uvs );


#endif