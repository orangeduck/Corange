#include "ui/ui_style.h"

ui_style* ui_style_current = &ui_style_corange;

ui_style ui_style_corange = {
  
  /* Box */
  .box_back_image = {"$CORANGE/ui/pale_orange.dds"},
  .box_back_width  = 32,
  .box_back_height = 32,
  .box_back_tile = true,
  .box_back_border_size = 1.0,
  .box_back_border_color = {1, 1, 1, 0.9},
  .box_glitch = 0.0,
  .box_text_color  = {1.0, 1.0, 1.0, 0.9},
  .box_label_color = {0.5, 0.5, 0.5, 0.9},
  .box_text_halign = TEXT_ALIGN_CENTER,
  .box_text_valign = TEXT_ALIGN_CENTER,
  .box_up_color    = {1.00, 0.75, 0.75, 0.75},
  .box_down_color  = {1.00, 0.50, 0.50, 0.75},
  .box_inset_color = {0.75, 0.33, 0.33, 0.75},
  
  /* Text */
  .text_font = {"$CORANGE/fonts/console_font.fnt"},
  .text_color = {0,0,0,0},
  .text_scale = {1.0, 1.0},
  
  /* Spinner */
  .spinner_image = {"$CORANGE/ui/spinner.dds"},
  .spinner_speed = 5.0
  
};

ui_style ui_style_hunt = {
  
  /* Box */
  .box_back_image = {"$CORANGE/ui/back_wood.dds"},
  .box_back_width  = 128,
  .box_back_height = 128,
  .box_back_tile = true,
  .box_back_border_size = 1,
  .box_back_border_color = {0,0,0,1},
  .box_glitch = 1.0,
  .box_text_color  = {0.75,0.75,0.75, 1},
  .box_label_color = {0.50, 0.50, 0.50, 1},
  .box_text_halign = TEXT_ALIGN_CENTER,
  .box_text_valign = TEXT_ALIGN_CENTER,
  .box_up_color = {0.1, 0.1, 0.1, 1},
  .box_down_color = {1,1,1,1},
  .box_inset_color = {0.1, 0.1, 0.1, 1.0},

  /* Text */
  .text_font = {"$CORANGE/fonts/console_font.fnt"},
  .text_color = {0,0,0,0},
  .text_scale = {1.0, 1.0},
  
  /* Spinner */
  .spinner_image = {"$CORANGE/ui/spinner.dds"},
  .spinner_speed = 5.0

};