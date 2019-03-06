#include "corange.h"

void renderer_custom(static_object * object,asset_hndl cubemap_shader)
{
    if(object == NULL || asset_hndl_isnull(&cubemap_shader))
    {
        return;
    };
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader_program* shader = material_first_program(asset_hndl_ptr(&cubemap_shader));
    shader_program_enable(shader);
                                                        //for static cubemap set camera->position, big size and rotate quat_id()
    shader_program_set_mat4(shader, "world",            mat4_world(object->position , object->scale,object->rotation));
    shader_program_set_mat4(shader, "view",             camera_view_matrix(entity_get("camera")));
    shader_program_set_mat4(shader, "proj",             camera_proj_matrix(entity_get("camera")));
    shader_program_set_vec3(shader, "camera_position",  entity_get_as("camera",camera)->position);

    renderable* r = asset_hndl_ptr(&object->renderable);

    for(int i=0; i < r->num_surfaces; i++) 
    {
       renderable_surface * s = r->surfaces[i];
       material_entry * me = material_get_entry(asset_hndl_ptr(&r->material),i);
 
        shader_program_set_texture(shader, "cubemap",  0, material_entry_item(me,"cubemap").as_asset);
        glBindBuffer(GL_ARRAY_BUFFER, s->vertex_vbo);
      
      shader_program_enable_attribute(shader, "vPosition",  3, 18, (void*)0);
      shader_program_enable_attribute(shader, "vNormal",    3, 18, (void*)(sizeof(float) * 3));
      shader_program_enable_attribute(shader, "vTangent",   3, 18, (void*)(sizeof(float) * 6));
      shader_program_enable_attribute(shader, "vBinormal",  3, 18, (void*)(sizeof(float) * 9));
     
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->triangle_vbo);
        glDrawElements(GL_TRIANGLES, s->num_triangles * 3, GL_UNSIGNED_INT, (void*)0);
  
      shader_program_disable_attribute(shader, "vPosition");
      shader_program_disable_attribute(shader, "vNormal");
      shader_program_disable_attribute(shader, "vTangent");
      shader_program_disable_attribute(shader, "vBinormal");
 
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    };
    shader_program_disable(shader);
}

int main(int argc, char **argv)
{
  corange_init("../../assets_core");
  folder_load(P("./assets/"));
  graphics_viewport_set_title("Multi Cubemaps");
  graphics_viewport_set_size(800, 600);

  camera * cam  = entity_new("camera",camera);
  static_object * cube = entity_new("cube",static_object);
  vec3 last_position   = cube->position;
  cube->renderable = asset_hndl_new(P("./assets/box.obj"));
  renderable * ren = asset_hndl_ptr(&cube->renderable);
  ren->material    = asset_hndl_new(P("./assets/box.mat"));
  asset_hndl cubemap_shader = asset_hndl_new_load(P("./assets/cubemap.mat"));

  float speed       = 1.0; 
  bool  running     = true;
  int   position_id = 0;

  ui_button * sky = ui_elem_new("sky",ui_button);
  ui_button * tab = ui_elem_new("tab",ui_button);
  ui_button * sea = ui_elem_new("sea",ui_button);
  ui_button * esc = ui_elem_new("esc",ui_button);

  ui_button_set_label(sky,"skycube");
  ui_button_set_label(tab,"tabcube");
  ui_button_set_label(sea,"seacube");
  ui_button_set_label(esc,"exit");


  ui_button_resize(sky,vec2_new(80,30));
  ui_button_resize(tab,vec2_new(80,30));
  ui_button_resize(sea,vec2_new(80,30));
  ui_button_resize(esc,vec2_new(50,30));


  ui_button_move(sky,vec2_new(10,10));
  ui_button_move(tab,vec2_new(100,10));
  ui_button_move(sea,vec2_new(190,10));
  ui_button_move(esc,vec2_new(280,10));


  void pos2(){position_id=2;};
  void pos1(){position_id=1;};
  void pos3(){position_id=3;};
  void escp(){running=false;};

  ui_button_set_onclick(sky,pos2);
  ui_button_set_onclick(tab,pos1);
  ui_button_set_onclick(sea,pos3);
  ui_button_set_onclick(esc,escp);

  while(running)
  {

        frame_begin();
        SDL_Event e = {0};
        while(SDL_PollEvent(&e))
        {
            ui_update();
            ui_event(e);
            switch(e.type)
            {
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    if (e.key.keysym.sym == SDLK_ESCAPE)
                    { 
                        running = false; 
                    };
                    if (e.key.keysym.sym == SDLK_PRINTSCREEN)
                    { 
                        graphics_viewport_screenshot(); 
                    };
                    if (e.key.keysym.sym == SDLK_r && e.key.keysym.mod == KMOD_LCTRL)
                    {
                        asset_reload_all();
                    };
                break;
                case SDL_QUIT:
                running = 0;
                break;
                default:break;
            };
        };
        /*Happy motion :)*/
        switch(position_id)
        {
            case 1:if(vec3_neq(cube->position,vec3_add(cam->position,vec3_new(0,-3,0))))
                   {
                       cube->position =  vec3_smootherstep(vec3_add(cam->position,vec3_new(0,-3,0.0)),last_position,speed -=frame_time()/3);
                   }else{
                       speed = 1.0;
                       last_position = vec3_add(cam->position,vec3_new(0,-3,0));
                       position_id = 0;
                   }break;
            case 2:if(vec3_neq(cube->position,vec3_add(cam->position,vec3_new(0,-3,+12))))
                   {
                       cube->position =  vec3_smootherstep(vec3_add(cam->position,vec3_new(0,-3,+12)),last_position,speed -=frame_time()/2);
                   }else{
                       speed = 1.0;
                       last_position = vec3_add(cam->position,vec3_new(0,-3,+12));
                       position_id = 0;
                   }break;
            case 3:if(vec3_neq(cube->position,vec3_add(cam->position,vec3_new(0,-3,-12))))
                   {
                       cube->position =  vec3_smootherstep(vec3_add(cam->position,vec3_new(0,-3,-12)),last_position,speed -=frame_time()/2);
                   }else{
                       speed = 1.0;
                       last_position = vec3_add(cam->position,vec3_new(0,-3,-12));
                       position_id = 0;
                   }break;
            default:break;
        };

        camera_control_freecam(cam,frame_time()/8);
        renderer_custom(cube,cubemap_shader);
        ui_render();

        graphics_swap();
        frame_end();
  };
  corange_finish();
  return 0;
}
