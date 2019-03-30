#include "corange.h"

typedef SDL_Event event;
static renderer * dr = NULL;
static bool  gravity_enable = true;
static bool  running = true;
static float max_po = 150.0;
static float max_po_new = 150.0;
static event event_common_event = { 0 };
static bool  event_common_state = false;

/*see code bottom main() custom physics handlers becouse original code in wip long time :) */
void physics_object_update_custom(physics_object * po,float ftime);
void physics_object_collide_static_custom(physics_object * po,static_object * so,float ftime);
void splash();

vec2 mouse_get_position(void)
{
    int x=0;
    int y=0;
    SDL_GetMouseState(&x,&y);
    return (vec2){(float)x,(float)y};
}

inline static event event_get()
{
    return  event_common_event;
}

bool event_update()
{
    event_common_state = SDL_PollEvent(&event_common_event);
    return event_common_state;
}

void exit_callback(ui_button * btn,void * data)
{
    running = false;
}

void random_callback(ui_button * btn, void * data)
{
    ui_toast_popup("Drop balls random");
    for (int i = 0; i < max_po; i++)
    {
       physics_object * po = entity_get("po%i",i);
       po->velocity.x -=randf_range(-0.5,0.5);
       po->velocity.y +=randf_range(-1,1);
       po->velocity.z +=randf_range(-0.5,0.5);
    };
}

void drop_callback(ui_button * btn, void * data)
{
    ui_toast_popup("Drop balls");
    vec3 * velocity = (vec3*)data;
    for (int i = 0; i < max_po; i++)
    {
       physics_object *  po = entity_get("po%i",i);
       po->velocity   = *velocity;
    };
}

void gravity_trigger(ui_button * btn, void * data)
{
    gravity_enable =!gravity_enable;
    if(gravity_enable)
    {
        btn->up_color = vec4_green();
        ui_toast_popup("Gravity Enable");
    }else{
        btn->up_color = vec4_red();
        ui_toast_popup("Gravity Disable");
    };
}

void disable_dialog(ui_button * btn,void * data)
{
   ui_elem_delete("help");
}

void respawn_callback(ui_button * btn,void * data)
{
    ui_toast_popup("Respawn new balls");
    for (int i = 0; i < max_po; ++i)
    {
      entity_delete("po%i",i);
      entity_delete("poc%i",i);
    }
    if(max_po != max_po_new)
    {
        max_po = max_po_new;
    };
    for (int i = 0; i < max_po; i++)
    {
      physics_object * po = entity_new("po%i", physics_object,i);
      po->renderable      = asset_hndl_new_load(P("./assets/po/sphere.obj"));
      po->position        = vec3_new(0,i,0);
      po->scale           = vec3_new(5,5,5);
      po->collision_body  = asset_hndl_new_load(P("$CORANGE/objects/sphere.col"));
      /*duplicate objects for renderer becouse physic_object not implement*/
      static_object * poc = entity_new("poc%i", static_object,i);
      poc->renderable     = asset_hndl_new_load(P("./assets/po/sphere.obj"));
      poc->position       = vec3_new(0,i,0);
      poc->scale          = vec3_new(5,i,5);
      renderable * pocr   = asset_hndl_ptr(&poc->renderable);
      pocr->material      = asset_hndl_new_load(P("./assets/po/material.mat"));
    }
}

int main(int argc, char *argv[])
{
    corange_init("../../assets_core");
    graphics_viewport_set_size(1280,720);
    graphics_viewport_set_title("Physics Demo Experiment");
    folder_load_recursive(P("./assets/"));
    splash();
    extern ui_style * ui_style_current;
    ui_style_current->box_back_image = P("./assets/theme/ui.tga");

    dr = renderer_new(asset_hndl_new_load(P("./graphics.cfg")));
    dr->mesh_skydome = asset_hndl_new_load(P("./assets/po/sphere.obj"));
    entity_new("cam",camera);

    entity_get_as("cam",camera)->position = vec3_new(1.93,147.0,139.98);
    entity_get_as("cam",camera)->far_clip *=4;
    renderer_set_camera(dr,entity_get("cam"));
    renderer_set_skydome_enabled(dr,true);
    renderer_set_tod(dr,0.17,1);


    ui_elem_new("framerate",ui_button);
    ui_button_move(ui_elem_get("framerate"),vec2_new(10,10));
    ui_button_resize(ui_elem_get("framerate"),vec2_new(80,30));

    ui_elem_new("respawn",ui_button);
    ui_elem_new("drop up",ui_button);
    ui_elem_new("drop down",ui_button);
    ui_elem_new("drop left",ui_button);
    ui_elem_new("drop right",ui_button);
    ui_elem_new("drop forw",ui_button);
    ui_elem_new("drop back",ui_button);

    ui_button * gravity = ui_elem_new("gravity",ui_button);
    gravity->up_color = vec4_green();

    ui_button * exit_btn = ui_elem_new("exit",ui_button);
    exit_btn->up_color = vec4_red();

    ui_button_set_onclick(ui_elem_get("respawn"),respawn_callback);
    ui_button_set_onclick(ui_elem_get("drop up"),drop_callback);
    ui_button_set_onclick(ui_elem_get("drop down"),drop_callback);
    ui_button_set_onclick(ui_elem_get("drop left"),drop_callback);
    ui_button_set_onclick(ui_elem_get("drop right"),drop_callback);
    ui_button_set_onclick(ui_elem_get("drop forw"),drop_callback);
    ui_button_set_onclick(ui_elem_get("drop back"),drop_callback);
    ui_button_set_onclick(ui_elem_get("exit"),exit_callback);
    ui_button_set_onclick(ui_elem_get("gravity"),gravity_trigger);
    enum
    {
        veloc_top,
        veloc_down,
        veloc_left,
        veloc_right,
        veloc_forw,
        veloc_back
    };

    vec3  veloc[] = 
    {
        [0] = {0,2,0},
        [1] = {0,0,0},
        [2] = {-2,0,0},
        [3] = {2,0,0},
        [4] = {0,0,-2},
        [5] = {0,0,2}
    };

    ui_button_set_onclick_data(ui_elem_get("drop up"),&veloc[0]);
    ui_button_set_onclick_data(ui_elem_get("drop down"),&veloc[1]);
    ui_button_set_onclick_data(ui_elem_get("drop left"),&veloc[2]);
    ui_button_set_onclick_data(ui_elem_get("drop right"),&veloc[3]);
    ui_button_set_onclick_data(ui_elem_get("drop forw"),&veloc[4]);
    ui_button_set_onclick_data(ui_elem_get("drop back"),&veloc[5]);

    ui_button_set_label(ui_elem_get("respawn"),"respawn [r]");
    ui_button_set_label(ui_elem_get("drop up"),"drop up");
    ui_button_set_label(ui_elem_get("drop down"),"drop down");
    ui_button_set_label(ui_elem_get("drop left"),"drop left");
    ui_button_set_label(ui_elem_get("drop right"),"drop right");
    ui_button_set_label(ui_elem_get("drop forw"),"drop forw");
    ui_button_set_label(ui_elem_get("drop back"),"drop back");
    ui_button_set_label(ui_elem_get("exit"),"exit");
    ui_button_set_label(ui_elem_get("gravity"),"gravity [e]");

    ui_button_resize(ui_elem_get("respawn"),vec2_new(100,30));
    ui_button_resize(ui_elem_get("drop up"),vec2_new(100,30));
    ui_button_resize(ui_elem_get("drop down"),vec2_new(100,30));
    ui_button_resize(ui_elem_get("drop left"),vec2_new(100,30));
    ui_button_resize(ui_elem_get("drop right"),vec2_new(100,30));
    ui_button_resize(ui_elem_get("drop forw"),vec2_new(100,30));
    ui_button_resize(ui_elem_get("drop back"),vec2_new(100,30));
    ui_button_resize(ui_elem_get("exit"),vec2_new(100,30));
    ui_button_resize(ui_elem_get("gravity"),vec2_new(100,30));

    ui_button_move(ui_elem_get("respawn"),vec2_new(100,10));
    ui_button_move(ui_elem_get("drop up"),vec2_new(210,10));
    ui_button_move(ui_elem_get("drop down"),vec2_new(320,10));
    ui_button_move(ui_elem_get("drop left"),vec2_new(430,10));
    ui_button_move(ui_elem_get("drop right"),vec2_new(540,10));
    ui_button_move(ui_elem_get("drop forw"),vec2_new(650,10));
    ui_button_move(ui_elem_get("drop back"),vec2_new(760,10));
    ui_button_move(ui_elem_get("exit"),vec2_new(1170,10));
    ui_button_move(ui_elem_get("gravity"),vec2_new(870,10));

    ui_slider * podium = ui_elem_new("podium",ui_slider);
    podium->active = true;
    podium->pressed = false;
    podium->bar->color = vec4_red();
    podium->slot->color = vec4_green();
    ui_slider_move(ui_elem_get("podium"),vec2_new(10,655));
    ui_slider_set_label(ui_elem_get("podium"),"podium y axis");
    ui_slider_set_amount(ui_elem_get("podium"),0);

    ui_slider * objects = ui_elem_new("objects",ui_slider);
    objects->active = true;
    objects->pressed = false;
    objects->bar->color = vec4_red();
    objects->slot->color = vec4_green();
    ui_slider_move(ui_elem_get("objects"),vec2_new(510,655));
    ui_slider_set_label(ui_elem_get("objects"),"balls num");
    ui_slider_set_amount(ui_elem_get("objects"),0.1);

    ui_elem_new("w",ui_button);
    ui_elem_new("a",ui_button);
    ui_elem_new("s",ui_button);
    ui_elem_new("d",ui_button);
    ui_elem_new("space",ui_button);

    ui_button_set_onclick(ui_elem_get("space"),random_callback);
    ui_button_set_onclick(ui_elem_get("w"),drop_callback);
    ui_button_set_onclick(ui_elem_get("s"),drop_callback);
    ui_button_set_onclick(ui_elem_get("a"),drop_callback);
    ui_button_set_onclick(ui_elem_get("d"),drop_callback);

    ui_button_set_onclick_data(ui_elem_get("w"),&veloc[4]);
    ui_button_set_onclick_data(ui_elem_get("s"),&veloc[5]);
    ui_button_set_onclick_data(ui_elem_get("a"),&veloc[2]);
    ui_button_set_onclick_data(ui_elem_get("d"),&veloc[3]);

    ui_button_set_label(ui_elem_get("w"),"u");
    ui_button_set_label(ui_elem_get("a"),"l");
    ui_button_set_label(ui_elem_get("s"),"d");
    ui_button_set_label(ui_elem_get("d"),"s");
    ui_button_set_label(ui_elem_get("space"),"space [random]");

    ui_button_resize(ui_elem_get("w"),vec2_new(30,30));
    ui_button_resize(ui_elem_get("a"),vec2_new(30,30));
    ui_button_resize(ui_elem_get("s"),vec2_new(30,30));
    ui_button_resize(ui_elem_get("d"),vec2_new(30,30));
    ui_button_resize(ui_elem_get("space"),vec2_new(150,30));

    ui_button_move(ui_elem_get("w"),vec2_new(1195,620));
    ui_button_move(ui_elem_get("a"),vec2_new(1160,655));
    ui_button_move(ui_elem_get("s"),vec2_new(1195,655));
    ui_button_move(ui_elem_get("d"),vec2_new(1230,655));
    ui_button_move(ui_elem_get("space"),vec2_new(1000,655));


    ui_dialog * help = ui_elem_new("help",ui_dialog);
    ui_rectangle_set_texture(help->back->back,asset_hndl_new_load(P("./assets/theme/help.tga")),0,0,false);
    help->contents->color = vec4_green();
    help->contents->scale = vec2_new(1.1,1.1);
    ui_dialog_set_single_button(ui_elem_get("help"),true);
    ui_dialog_set_title(ui_elem_get("help"),"");
    ui_dialog_set_contents(ui_elem_get("help"),
    "drop balls [up/down/left/right]\n"
    "drop balls random  [space]\n"
    "on/off gravity     [e]\n"
    "respawn balls      [r]\n");
    ui_dialog_set_button_left(ui_elem_get("help"),"Ok",disable_dialog);

    //generate first objects
    for (int i = 0; i < max_po; i++)
    {
      physics_object * po = entity_new("po%i", physics_object,i);
      po->renderable      = asset_hndl_new_load(P("./assets/po/sphere.obj"));
      po->position        = vec3_new(0,i,0);
      po->scale           = vec3_new(5,5,5);
      po->collision_body  = asset_hndl_new_load(P("$CORANGE/objects/sphere.col"));

      static_object * poc = entity_new("poc%i", static_object,i);
      poc->renderable     = asset_hndl_new_load(P("./assets/po/sphere.obj"));
      poc->position       = vec3_new(0,i,0);
      poc->scale          = vec3_new(5,5,5);
      renderable * pocr   = asset_hndl_ptr(&poc->renderable);
      pocr->material      = asset_hndl_new_load(P("./assets/po/material.mat"));
 
    }

    entity_new("so",static_object);
    entity_get_as("so",static_object)->renderable = asset_hndl_new_load(P("./assets/so/so.obj"));
    renderable * sor = asset_hndl_ptr(&entity_get_as("so",static_object)->renderable);
    sor->material = asset_hndl_new(P("./assets/so/so.mat"));
    entity_get_as("so",static_object)->collision_body = asset_hndl_new_load(P("./assets/so/so.col"));

    float tod = 0.17;
    int mpos_x;
    int mpos_y;
    while(running)
    {
        frame_begin();
        //frame_set_rate(60);
        ui_button_set_label(ui_elem_get("framerate"),frame_rate_string());
   
       //detect hover slider
       if(!ui_rectangle_contains_point(podium->bar,mouse_get_position()) &&
          !ui_rectangle_contains_point(objects->bar,mouse_get_position())){
          camera_control_freecam(entity_get("cam"),frame_time());};

    //rotate podium
    char buff[64];
    entity_get_as("so",static_object)->rotation = quat_rotation_z(ui_slider_get_amount(ui_elem_get("podium")));
    sprintf(buff,"podium axis %.3f",entity_get_as("so",static_object)->rotation.z);
    ui_slider_set_label(ui_elem_get("podium"),buff);
    
    //update num objects
    max_po_new = ui_slider_get_amount(ui_elem_get("objects")) * 1000;//get balls numbers
    max_po_new = (int)max_po_new;//rounding
    sprintf(buff,"objects %i",(int)max_po_new);
    ui_slider_set_label(ui_elem_get("objects"),buff);

    ui_update();
    while(event_update())
    {
        ui_event(event_get());
        if(event_get().type == SDL_KEYDOWN)
        {
            if(event_get().key.keysym.sym == SDLK_SPACE)
            {
                random_callback(NULL,NULL);
            };
            if(event_get().key.keysym.sym == SDLK_UP)
            {
                drop_callback(NULL,&veloc[veloc_forw]);
            };
            if(event_get().key.keysym.sym == SDLK_DOWN)
            {
               drop_callback(NULL,&veloc[veloc_back]);
            };
            if(event_get().key.keysym.sym == SDLK_LEFT)
            {
               drop_callback(NULL,&veloc[veloc_left]);
            };
            if(event_get().key.keysym.sym == SDLK_RIGHT)
            {
               drop_callback(NULL,&veloc[veloc_right]);
            };
            if(event_get().key.keysym.sym == SDLK_e)
            {
               gravity_trigger(ui_elem_get("gravity"),NULL);
            };
            if(event_get().key.keysym.sym == SDLK_r)
            {
               respawn_callback(NULL,NULL);
            };
        };
    }

    renderer_add(dr,render_object_static(entity_get("so")));

    for (int i = 0; i < max_po; i++)
    {
        physics_object * po  = entity_get("po%i",i);
        static_object  * poc = entity_get("poc%i",i);
        poc->position = po->position;
        poc->position.x +=6;
        poc->scale    = po->scale;
        renderer_add(dr,render_object_static(poc));
    }


    for (int i = 0; i < max_po; ++i)
    {
      physics_object_update_custom(entity_get("po%i",i),frame_time());
      physics_object_collide_static_custom(entity_get("po%i",i),entity_get("so"),frame_time());
    }

    renderer_render(dr);

    ui_render();
    graphics_swap();
    frame_end();
    };
    corange_finish();
    return 0;
}

/*-----------------------------------------------------------------------*/
/**
*** custom physics handle, becouse need more implements in original code
**/
/*-----------------------------------------------------------------------*/

/*origunal runction locate in physics.c*/
void collision_response_slide_custom(vec3* position, vec3* velocity, collision (*colfunc)(void))
{
  collision col = colfunc();
  int count = 0;
  while (col.collided)
  {
    *velocity = vec3_mul(*velocity,1.1);
    *velocity = vec3_reflect(*velocity, col.norm);
    *position = vec3_add(*position, *velocity);
    col = colfunc();

    if(count++ > 100){ break; }
  };
    *position = vec3_add(*position, *velocity);

}


mat4 physics_object_world(physics_object* p)
{
  return mat4_world(p->position, p->scale, p->rotation);
}

mat3 physics_object_world_normal(physics_object* p)
{
  return mat3_transpose(mat3_inverse(mat4_to_mat3(physics_object_world(p))));
}

/*original code locate in entities/physics_object.c*/
void physics_object_update_custom(physics_object* po, float timestep)
{
  /*this calculate from sea demo*/

  vec3 gravity_update  = vec3_add(po->velocity, vec3_mul((gravity_enable)?vec3_new(0,-9.8,0):vec3_new(0,0,0),timestep));
  vec3 movement_update = vec3_lerp(po->velocity, vec3_mul(vec3_zero(), 0.5), saturate(5 *timestep));
  po->velocity         = vec3_lerp(gravity_update, movement_update, 0.1);
}

/*original code locate in entities/physics_object.c*/
void physics_object_collide_static_custom(physics_object* po, static_object* so, float timestep)
{

  cmesh * po_col = asset_hndl_ptr(&po->collision_body);
  cmesh * so_col = asset_hndl_ptr(&so->collision_body);

  if ((po_col == NULL) || (so_col == NULL))
  {
    error("Cannot collide objects. One or more has no collision body - First: %p Second: %p).", po_col, so_col);
  }

  mat4 so_world  = static_object_world(so);
  mat4 po_world  = physics_object_world(po);
  mat3 so_normal = static_object_world_normal(so);
  ellipsoid po_ellipsoid = ellipsoid_transform(ellipsoid_new(vec3_one(),vec3_one()),po_world);

  collision collision_call()
  {
     return ellipsoid_collide_mesh(po_ellipsoid,po->velocity,so_col,so_world,so_normal);
  };
  collision_response_slide_custom(&po->position,&po->velocity, collision_call);
}



void splash()
{
    float speed = 0.005;
    //create splash
    ui_rectangle * splash = ui_elem_new("_splash_",ui_rectangle);
    splash->top_left = vec2_new(0,0);
    ui_rectangle_resize(splash,vec2_new(graphics_viewport_width(),graphics_viewport_height()));
    //set texture and filtrating for splash
    texture * splash_texture = asset_get(P("./assets/splash/splash.tga"));
    texture_set_filtering_anisotropic(splash_texture);
    ui_rectangle_set_texture(splash,asset_hndl_new(P("./assets/splash/splash.tga")),0,0,false);

    //create and play splash sound
    asset_get_as(P("./assets/splash/splash.wav"),sound)->sample->volume = 50;
    audio_sound_play(asset_get(P("./assets/splash/splash.wav")),0);
    //render splash
    while(splash->active)
    {
        frame_begin();
        frame_end_at_rate(60);
        if(splash->color.w > 0 || splash->color.x > 0 || splash->color.y > 0 || splash->color.z > 0)
        {   //splash colors down
            splash->color.w-=speed;
            splash->color.x-=speed;
            splash->color.y-=speed;
            splash->color.z-=speed;

        }else{
            //hide splash if alpha is 0
            splash->active = false;
        };
        ui_render();
        graphics_swap();
        frame_end();
    };
    //delete elements and exit
    ui_elem_delete("_splash_");
};


