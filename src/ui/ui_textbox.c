
#include "ui/ui_textbox.h"

ui_textbox* ui_textbox_new() {
  ui_textbox* tb = malloc(sizeof(ui_textbox));

  tb->inner = ui_rectangle_new();
  ui_rectangle_set_texture(tb->inner, asset_hndl_new_load(P("$CORANGE/resources/back_wood.dds")), 128, 128, true);
  ui_rectangle_set_border(tb->inner, 1, vec4_black());
  ui_rectangle_set_glitch(tb->inner, 1.0);
  
  tb->outer = ui_rectangle_new();
  ui_rectangle_set_texture(tb->outer, asset_hndl_new_load(P("$CORANGE/resources/back_wood.dds")), 128, 128, true);
  ui_rectangle_set_border(tb->outer, 1, vec4_black());
  ui_rectangle_set_color(tb->outer, vec4_grey());
  ui_rectangle_set_glitch(tb->outer, 1.0);
  
  tb->contents = ui_text_new();
  ui_text_set_color(tb->contents, vec4_light_grey());
  ui_text_align(tb->contents, text_align_center, text_align_center);
  ui_text_move(tb->contents, ui_rectangle_center(tb->inner));
  ui_text_draw_string(tb->contents, "");
  
  tb->label = ui_text_new();
  ui_text_set_color(tb->label, vec4_dark_grey());
  ui_text_align(tb->label, text_align_center, text_align_center);
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
  
  if (halign == text_align_left) { x = tb->inner->top_left.x; }
  if (halign == text_align_right) { x = tb->inner->bottom_right.x; }
  if (halign == text_align_center) { x = ui_rectangle_center(tb->inner).x; }
  if (valign == text_align_top) { y = tb->inner->top_left.y; }
  if (valign == text_align_bottom) { y = tb->inner->bottom_right.y; }
  if (valign == text_align_center) { y = ui_rectangle_center(tb->inner).y; }
  
  ui_text_move(tb->contents, vec2_new(x, y));
  ui_text_move(tb->label, vec2_new(x, y));

}

void ui_textbox_disable(ui_textbox* tb) {
  tb->enabled = false;
}

void ui_textbox_enable(ui_textbox* tb) {
  tb->enabled = true;
}

void ui_textbox_event(ui_textbox* tb, SDL_Event e) {
  
  if (e.type == SDL_MOUSEBUTTONDOWN) {
    if (ui_textbox_contains_point(tb, vec2_new(e.motion.x, e.motion.y))) {
      tb->selected = true;
    } else {
      tb->selected = false;
    }
  }
  
  if (tb->selected) {
    
    if (e.type == SDL_KEYUP) {
      
      if (e.key.keysym.sym == SDLK_BACKSPACE) {
        ui_textbox_rmchar(tb);
        return;
      }
      
      char ch;
      switch (e.key.keysym.sym) {
        case SDLK_a: ch = 'a'; break;
        case SDLK_b: ch = 'b'; break;
        case SDLK_c: ch = 'c'; break;
        case SDLK_d: ch = 'd'; break;
        case SDLK_e: ch = 'e'; break;
        case SDLK_f: ch = 'f'; break;
        case SDLK_g: ch = 'g'; break;
        case SDLK_h: ch = 'h'; break;
        case SDLK_i: ch = 'i'; break;
        case SDLK_j: ch = 'j'; break;
        case SDLK_k: ch = 'k'; break;
        case SDLK_l: ch = 'l'; break;
        case SDLK_m: ch = 'm'; break;
        case SDLK_n: ch = 'n'; break;
        case SDLK_o: ch = 'o'; break;
        case SDLK_p: ch = 'p'; break;
        case SDLK_q: ch = 'q'; break;
        case SDLK_r: ch = 'r'; break;
        case SDLK_s: ch = 's'; break;
        case SDLK_t: ch = 't'; break;
        case SDLK_u: ch = 'u'; break;
        case SDLK_v: ch = 'v'; break;
        case SDLK_w: ch = 'w'; break;
        case SDLK_x: ch = 'x'; break;
        case SDLK_y: ch = 'y'; break;
        case SDLK_z: ch = 'z'; break;
        
        case SDLK_1: ch = '1'; break;
        case SDLK_2: ch = '2'; break;
        case SDLK_3: ch = '3'; break;
        case SDLK_4: ch = '4'; break;
        case SDLK_5: ch = '5'; break;
        case SDLK_6: ch = '6'; break;
        case SDLK_7: ch = '7'; break;
        case SDLK_8: ch = '8'; break;
        case SDLK_9: ch = '9'; break;
        case SDLK_0: ch = '0'; break;
        
        case SDLK_SPACE:     ch = ' '; break;
        
        default: ch = 0; break;
      }
      
      bool uppercase = 
        (e.key.keysym.mod & KMOD_RSHIFT) ||
        (e.key.keysym.mod & KMOD_LSHIFT) ||
        (e.key.keysym.mod & KMOD_CAPS);
      
      if ((ch >= 'a') && (ch <= 'z') && uppercase) { ch = ch - 32; }
      if (ch == '1' && uppercase) { ch = '!'; }
      if (ch == '2' && uppercase) { ch = '"'; }
      if (ch == '2' && uppercase) { ch = '£'; }
      if (ch == '4' && uppercase) { ch = '$'; }
      if (ch == '5' && uppercase) { ch = '%'; }
      if (ch == '6' && uppercase) { ch = '^'; }
      if (ch == '7' && uppercase) { ch = '&'; }
      if (ch == '8' && uppercase) { ch = '*'; }
      if (ch == '9' && uppercase) { ch = '('; }
      if (ch == '0' && uppercase) { ch = ')'; }
      
      if (ch != 0) {
        ui_textbox_addchar(tb, ch);
      }
    }
    
  }
  
}

void ui_textbox_update(ui_textbox* tb) {

}

void ui_textbox_render(ui_textbox* tb) {
  
  if (tb->selected || !tb->enabled) {
    ui_rectangle_set_color(tb->inner, vec4_dark_grey());
  } else {
    ui_rectangle_set_color(tb->inner, vec4_new(0.1, 0.1, 0.1, 1));
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
      ui_text_align(hidden, text_align_center, text_align_center);
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