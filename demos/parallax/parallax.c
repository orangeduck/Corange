#include "corange.h"
#include <unistd.h>

void renderer_custom(static_object * object,asset_hndl parallax)
{
    if(object == NULL || asset_hndl_isnull(&parallax))
    {
        return;
    };
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    shader_program* shader = material_first_program(asset_hndl_ptr(&parallax));
    shader_program_enable(shader);
    //я не знаю какие данные передавать поэтому просто передаю uniform всё что есть
    //пример взят из оригинального src/renderer.c render_static()
    shader_program_set_mat4(shader, "world",            mat4_world(object->position, object->scale, object->rotation));
    shader_program_set_mat4(shader, "view",             camera_view_matrix(entity_get("camera")));
    shader_program_set_mat4(shader, "proj",             camera_proj_matrix(entity_get("camera")));

     shader_program_set_float(shader, "clip_near", entity_get_as("camera",camera)->near_clip);
     shader_program_set_float(shader, "clip_far",  entity_get_as("camera",camera)->far_clip);

    shader_program_set_vec3(shader, "camera_position",  entity_get_as("camera",camera)->position);

    renderable* r = asset_hndl_ptr(&object->renderable);


 
    for(int i=0; i < r->num_surfaces; i++) 
    {
      renderable_surface* s = r->surfaces[i];
      int mentry_id = min(i, ((material*)asset_hndl_ptr(&r->material))->num_entries-1);
      material_entry* me = material_get_entry(asset_hndl_ptr(&r->material), mentry_id);
      shader_program_set_texture(shader, "diffuse_map",  0, material_entry_item(me,"diffuse_map").as_asset);
      shader_program_set_texture(shader, "normal_map",   1, material_entry_item(me,"normal_map").as_asset);
      shader_program_set_texture(shader, "specular_map", 2, material_entry_item(me,"specular_map").as_asset);
      shader_program_set_texture(shader, "parallax_map", 3, material_entry_item(me,"parallax_map").as_asset);
      shader_program_set_float(shader,   "glossiness_level",material_entry_item(me,"glossiness_level").as_float);
      shader_program_set_float(shader,   "specular_level",  material_entry_item(me,"specular_level").as_float);
      shader_program_set_float(shader,   "parallax_level",  material_entry_item(me,"parallax_level").as_float);
  
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

int main(int argc, char **argv)
{
  corange_init("../../assets_core");
  folder_load(P("./assets/"));
  graphics_viewport_set_title("Parallax mapping witch custom renderer");
  graphics_viewport_set_size(800, 600);
  camera * cam = entity_new("camera",camera);
  static_object   * cube  = entity_new("cube",static_object);
  cube->renderable        = asset_hndl_new(P("./assets/box.obj"));
  renderable * r          = asset_hndl_ptr(&cube->renderable);
  r->material             = asset_hndl_new(P("./assets/box.mat"));
  asset_hndl parallax_shader = asset_hndl_new_load(P("./assets/parallax.mat"));
  int running = 1;
  float rotate = 1.0;
  while(running)
  {

        frame_begin();
        SDL_Event e = {0};
        while(SDL_PollEvent(&e))
        {
            camera_control_orbit(cam,e);
            switch(e.type){
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            if (e.key.keysym.sym == SDLK_ESCAPE) { running = 0; }
            if (e.key.keysym.sym == SDLK_PRINTSCREEN) { graphics_viewport_screenshot(); }
            if (e.key.keysym.sym == SDLK_r && e.key.keysym.mod == KMOD_LCTRL)
            {
                asset_reload_all();
            };
        break;
        case SDL_QUIT:
        running = 0;
        break;
      }
        }

        camera_control_freecam(cam,frame_time()/8);
        cube->rotation =quat_rotation_y(rotate += frame_time());
        renderer_custom(cube,parallax_shader);
        graphics_swap();
        frame_end();
  };
  corange_finish();
  return 0;
}
