#include "ui/ui_textbox.h"
#include "ui/ui_style.h"

ui_textbox* ui_textbox_new() {
  ui_textbox* tb = malloc(sizeof(ui_textbox));

  tb->inner = ui_rectangle_new();
  ui_rectangle_set_texture(tb->inner, 
    asset_hndl_new_load(ui_style_current->box_back_image), 
    ui_style_current->box_back_width,
    ui_style_current->box_back_height,
    ui_style_current->box_back_tile);
  ui_rectangle_set_border(tb->inner,
    ui_style_current->box_back_border_size,
    ui_style_current->box_back_border_color);
  ui_rectangle_set_glitch(tb->inner, ui_style_current->box_glitch);
  ui_rectangle_set_color(tb->inner, ui_style_current->box_inset_color);
  
  tb->outer = ui_rectangle_new();
  ui_rectangle_set_texture(tb->outer, 
    asset_hndl_new_load(ui_style_current->box_back_image), 
    ui_style_current->box_back_width,
    ui_style_current->box_back_height,
    ui_style_current->box_back_tile);
  ui_rectangle_set_border(tb->outer,
    ui_style_current->box_back_border_size,
    ui_style_current->box_back_border_color);
  ui_rectangle_set_glitch(tb->outer, ui_style_current->box_glitch);
  ui_rectangle_set_color(tb->outer, ui_style_current->box_up_color);
  
  tb->contents = ui_text_new();
  ui_text_set_color(tb->contents, ui_style_current->box_text_color);
  ui_text_align(tb->contents, TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
  ui_text_move(tb->contents, ui_rectangle_center(tb->inner));
  ui_text_draw_string(tb->contents, "");
  
  tb->label = ui_text_new();
  ui_text_set_color(tb->label, ui_style_current->box_label_color);
  ui_text_align(tb->label, TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
  ui_text_move(tb->label, ui_rectangle_center(tb->inner));
  ui_text_draw_string(tb->label, "");
  
  tb->password = false;
  tb->max_chars = 256;
  
  tb->selected = false;
  tb->active = true;
  tb->enabled = true;
  
  return tb;
}

void ui_textbox_set_password(ui_textbox* tb, bool password) {
  tb->password = password;
}

void ui_textbox_set_max_chars(ui_textbox* tb, int l) {
  tb->max_chars = l;
}

void ui_textbox_addchar(ui_textbox* tb, char c) {
  
  char* curr = tb->contents->string;
  if (strlen(curr) >= tb->max_chars) return;
  
  char* temp = malloc(strlen(curr) + 2);
  strcpy(temp, curr);
  strcat(temp, (char[]){c, 0});
  
  ui_textbox_set_contents(tb, temp);
  
  free(temp);
}

void ui_textbox_rmchar(ui_textbox* tb) {
  
  char* curr = tb->contents->string;
  if (strlen(curr) <= 0) return;
  
  curr[strlen(curr)-1] = '\0';
  
  ui_text_draw(tb->contents);
  
}

void ui_textbox_delete(ui_textbox* tb) {
  
  ui_rectangle_delete(tb->inner);
  ui_rectangle_delete(tb->outer);
  
  ui_text_delete(tb->contents);
  ui_text_delete(tb->label);
  
  free(tb);
  
}

void ui_textbox_move(ui_textbox* tb, vec2 pos) {
  ui_rectangle_move(tb->inner, pos);
  ui_rectangle_move(tb->outer, vec2_sub(pos, vec2_new(5,5)));
  ui_text_move(tb->contents, ui_rectangle_center(tb->inner));
  ui_text_move(tb->label, ui_rectangle_center(tb->inner));
}

void ui_textbox_resize(ui_textbox* tb, vec2 size) {
  
  ui_rectangle_resize(tb->outer, size);
  ui_rectangle_resize(tb->inner, vec2_sub(size, vec2_new(10,10)));
  ui_text_move(tb->contents, ui_rectangle_center(tb->inner));
  ui_text_move(tb->label, ui_rectangle_center(tb->inner));
  
}

void ui_textbox_set_font(ui_textbox* tb, asset_hndl f) {
  ui_text_set_font(tb->label, f);
  ui_text_set_font(tb->contents, f);
}

void ui_textbox_set_label(ui_textbox* tb, char* label) {
  ui_text_draw_string(tb->label, label);
}

void ui_textbox_set_contents(ui_textbox* tb, char* label) {
  ui_text_draw_string(tb->contents, label);
}

void ui_textbox_set_alignment(ui_textbox* tb, int halign, int valign) {

  ui_text_align(tb->contents, halign, valign);
  ui_text_align(tb->label, halign, valign);
  
  float x = 0, y = 0;
  
  if (halign == TEXT_ALIGN_LEFT)   { x = tb->inner->top_left.x; }
  if (halign == TEXT_ALIGN_RIGHT)  { x = tb->inner->bottom_right.x; }
  if (halign == TEXT_ALIGN_CENTER) { x = ui_rectangle_center(tb->inner).x; }
  if (valign == TEXT_ALIGN_TOP)    { y = tb->inner->top_left.y; }
  if (valign == TEXT_ALIGN_BOTTOM) { y = tb->inner->bottom_right.y; }
  if (valign == TEXT_ALIGN_CENTER) { y = ui_rectangle_center(tb->inner).y; }
  
  ui_text_move(tb->contents, vec2_new(x, y));
  ui_text_move(tb->label, vec2_new(x, y));

}

void ui_textbox_disable(ui_textbox* tb) {
  tb->enabled = false;
}

void ui_textbox_enable(ui_textbox* tb) {
  tb->enabled = true;
}

static const float time_to_delete = 0.05;
static float time_delete = 0;

void ui_textbox_event(ui_textbox* tb, SDL_Event e) {
  
  if (e.type == SDL_MOUSEBUTTONDOWN) {
    if (ui_textbox_contains_point(tb, vec2_new(e.motion.x, e.motion.y))) {
      tb->selected = true;
    } else {
      tb->selected = false;
    }
  }
  
  if (tb->selected) {
    
    if (e.type == SDL_KEYDOWN) {
      
      if (e.key.keysym.sym == SDLK_BACKSPACE) {
        time_delete = -0.5;
        ui_textbox_rmchar(tb);
        return;
      }
      
      if (e.key.keysym.sym == SDLK_SPACE) {
        ui_textbox_addchar(tb, ' ');
        return;
      }
      
    }
    
    if (e.type == SDL_TEXTINPUT) {
      
      const char* valid = "abcdefghijklmnopqrstuvwxyz"
                          "ABCDEFGHIJKLMNOPQRSTUVWXY"
                          "Z1234567890!\"$%^&*()-=_+"
                          "[{}]:;@'~#<,>.?/\\| \0";   
      
      if (strlen(e.text.text) != 1 || !strchr(valid, e.text.text[0])) { return; }
      ui_textbox_addchar(tb, e.text.text[0]);
      
    }
    
  }
  
}

void ui_textbox_update(ui_textbox* tb) {
  
  const Uint8* keystate = SDL_GetKeyboardState(NULL);
  
  if (keystate[SDL_SCANCODE_BACKSPACE]) {
    
    time_delete += frame_time();
    if (time_delete > time_to_delete) {
      time_delete = 0;
      ui_textbox_rmchar(tb);
    }
    
  } else {
    time_delete = 0;
  }
  
}

void ui_textbox_render(ui_textbox* tb) {
  
  if (tb->selected || !tb->enabled) {
    ui_rectangle_set_color(tb->inner, ui_style_current->box_down_color);
  } else {
    ui_rectangle_set_color(tb->inner, ui_style_current->box_inset_color);
  }
  
  ui_rectangle_render(tb->outer);
  ui_rectangle_render(tb->inner);
  
  if (tb->selected || strlen(tb->contents->string) != 0) {
    
    if (tb->password) {
      
      char* buffer = malloc(strlen(tb->contents->string) + 1);
      strcpy(buffer, tb->contents->string);
      for(int i = 0; i < strlen(tb->contents->string); i++) {
        buffer[i] = '*';
      }
      
      ui_text* hidden = ui_text_new();
      ui_text_set_color(hidden, tb->contents->color);
      ui_text_align(hidden, TEXT_ALIGN_CENTER, TEXT_ALIGN_CENTER);
      ui_text_move(hidden, tb->contents->position);
      ui_text_set_font(hidden, tb->contents->font);
      ui_text_draw_string(hidden, buffer);
      
      ui_text_render(hidden);
      
      free(buffer);
      ui_text_delete(hidden);
      
    } else {
      ui_text_render(tb->contents);
    }
    
  } else {
    ui_text_render(tb->label);
  }
}

bool ui_textbox_contains_point(ui_textbox* tb, vec2 p) {
  return ui_rectangle_contains_point(tb->inner, p);
}
