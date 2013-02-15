#include <math.h>

#include "teapot.h"

asset_hndl teapot_shader;
asset_hndl teapot_object;

void teapot_init() {
  
  graphics_viewport_set_dimensions(1280, 720);
  graphics_viewport_set_title("Teapot");
  
  camera* cam = entity_new("camera", camera);
  cam->position = vec3_new(5, 5, 5);
  cam->target =  vec3_new(0, 0, 0);
  
  teapot_shader = asset_hndl_new_load(P("./assets/teapot.mat"));
  teapot_object = asset_hndl_new_load(P("./assets/teapot.obj"));
}

void teapot_update() {

  camera* cam = entity_get("camera");
  
}

void teapot_render() {
  
  camera* cam = entity_get("camera");
  
  glClearColor(0.25, 0.25, 0.25, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  
  shader_program* shader = material_first_program(asset_hndl_ptr(teapot_shader));
  shader_program_enable(shader);
  shader_program_set_mat4(shader, "world", mat4_id());
  shader_program_set_mat4(shader, "view", camera_view_matrix(cam));
  shader_program_set_mat4(shader, "proj", camera_proj_matrix(cam));
  
  renderable* r = asset_hndl_ptr(teapot_object);
  
  for(int i=0; i < r->num_surfaces; i++) {
    
    renderable_surface* s = r->surfaces[i];
    
    int mentry_id = min(i, ((material*)asset_hndl_ptr(r->material))->num_entries-1);
    material_entry* me = material_get_entry(asset_hndl_ptr(r->material), mentry_id);
    
    glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
    
    shader_program_enable_attribute(shader, "vPosition",  3, 18, (void*)0);
    shader_program_enable_attribute(shader, "vNormal",    3, 18, (void*)(sizeof(float) * 3));
    shader_program_enable_attribute(shader, "vTangent",   3, 18, (void*)(sizeof(float) * 6));
    shader_program_enable_attribute(shader, "vBinormal",  3, 18, (void*)(sizeof(float) * 9));
    shader_program_enable_attribute(shader, "vTexcoord",  2, 18, (void*)(sizeof(float) * 12));
    
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
      glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
    
    shader_program_disable_attribute(shader, "vPosition");
    shader_program_disable_attribute(shader, "vNormal");
    shader_program_disable_attribute(shader, "vTangent");
    shader_program_disable_attribute(shader, "vBinormal");
    shader_program_disable_attribute(shader, "vTexcoord");
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

  }
  
  shader_program_disable(shader);
  
  glDisable(GL_DEPTH_TEST);
  
}

void teapot_event(SDL_Event event) {

  camera* cam = entity_get("camera");
  
  camera_control_orbit(cam, event);

}

void teapot_finish() {
}

int main(int argc, char **argv) {
  
  corange_init("../../core_assets");
  
  teapot_init();
  
  int running = 1;
  SDL_Event e;
  
  while(running) {
    
    frame_begin();
    
    while(SDL_PollEvent(&e)) {
      switch(e.type){
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        if (e.key.keysym.sym == SDLK_ESCAPE) { running = 0; }
        if (e.key.keysym.sym == SDLK_PRINT) { graphics_viewport_screenshot(); }
        if (e.key.keysym.sym == SDLK_r &&
            e.key.keysym.mod == KMOD_LCTRL) {
            asset_reload_all();
        }
        break;
      case SDL_QUIT:
        running = 0;
        break;
      }
      teapot_event(e);
      ui_event(e);
    }
    
    teapot_update();
    ui_update();
    
    teapot_render();
    ui_render();
    
    SDL_GL_SwapBuffers();
    
    frame_end();
  }
  
  teapot_finish();
  
  corange_finish();
  
  return 0;
  
}
