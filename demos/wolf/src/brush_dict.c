#include "brush_dict.h"

#include "asset_manager.h"

image* brush_dict_mask;

image* sample_area;
image* brush_sample_area;

int num_brushes_x;
int num_brushes_y;

int num_brushes;

int counter;

void brush_dict_init(texture* brush_tex, int num_x, int num_y, int total) {
  
  texture_set_filtering_nearest(brush_tex);
  
  image* brush_dict_color = texture_get_image(brush_tex);
  image* brush_dict_alpha = image_alpha_channel(brush_dict_color);
  
  brush_dict_mask = image_intensity_mask(brush_dict_alpha, 0.5f);
  
  image_delete(brush_dict_color);
  image_delete(brush_dict_alpha);
  
  brush_sample_area = image_empty(128, 128);
  
  num_brushes_x = num_x;
  num_brushes_y = num_y;

  num_brushes = total;
  
  sample_area = image_blank(128,128);  
  counter = 0;
}

void brush_dict_finish() {
  image_delete(sample_area);
  image_delete(brush_dict_mask);
}

static vector2 position_for_brush(int i) {
  
  int y = (i / num_brushes_x);
  int x = (i % num_brushes_x);
  
  vector2 ret = v2((float)x / num_brushes_x, (float)y / num_brushes_y);
  
  return ret;
  
}

int brush_id_for_position( image* ref, image* stencil, vector2 uvs ) {
  
  char filename[128];
  
  vector2 offset = v2( (float)64 / ref->width, (float)64 / ref->height );
  vector2 top_left = v2_sub( uvs, offset);
  
  image_copy_sub(sample_area, ref, top_left);
  image_write_to_file(sample_area, "sample_area.tga");
  
  image* mask = image_flood_fill_mask(sample_area, 64, 64, 0.02);
  
  int rotated = 0;
  int area_height = image_mask_area_height(mask);
  int area_width = image_mask_area_width(mask);
  
  printf("(%i,%i)", area_height, area_width);
  
  if (area_height > area_width) {
    image_rotate_90_clockwise(mask);
    rotated = 1;
  }
  
  float best_score = 0.0f;
  int best_match = 0;
  
  int i;
  for (i = 0; i < num_brushes; i++){
    image_copy_sub(brush_sample_area, brush_dict_mask, position_for_brush(i) );

    sprintf(filename, "./brush_sample/sample_%i.tga", i);
    image_write_to_file(mask, filename);
    
    sprintf(filename, "./brush_alpha/sample_%i.tga", i);
    image_write_to_file(brush_sample_area, filename);
    
    image_mask_xnor(brush_sample_area, mask);
    
    sprintf(filename, "./brush_xor/sample_%i.tga", i);
    image_write_to_file(brush_sample_area, filename);
    
    float score = image_intensity(brush_sample_area);
    if ( score > best_score ) {
      best_score = score;
      best_match = i;
    }
  }
  
  printf("Best Score: %0.2f Best Match: %i\n", best_score, best_match);
  
  image_delete(mask);
  
  
  return best_match;
}
