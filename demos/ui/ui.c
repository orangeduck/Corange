#include "corange.h"

//sample splash and loading colored bar
void splash(float speed,bool fake_loading);
static bool running = true;
int main(int argc, char *argv[])
{
    //default init and loading resources
    corange_init("../../assets_core");
    folder_load_recursive(P("./assets"));
    splash(0.008,true);
    graphics_viewport_set_size(1280,720);
    audio_music_play(asset_get(P("./assets/sound/adventure.mp3")));
    audio_music_set_volume(1.0);
    splash(0.005,false);
    asset_hndl opt_graphics = asset_hndl_new_load(P("./graphics.cfg"));
    renderer * dr = renderer_new(opt_graphics);
    entity_new("cam",camera);
    entity_get_as("cam",camera)->far_clip *=4;
    renderer_set_camera(dr,entity_get("cam"));
    renderer_set_sea_enabled(dr,true);
    renderer_set_skydome_enabled(dr,true);
    renderer_set_tod(dr,0.17,1);

    //just magic effect :D
    entity_new("magic",particles);
    particles_set_effect(entity_get("magic"),asset_hndl_new(P("./assets/effect/magic.effect")));

    //just objects for background
    static_object * object = entity_new("object",static_object);
    static_object * box = entity_new("box",static_object);
    object->renderable = asset_hndl_new(P("./assets/object/object.obj"));
    box->renderable = asset_hndl_new(P("./assets/object/box.obj"));
    object->scale = vec3_new(10,10,10);
    box->scale = vec3_new(2,2,2);

    //change current style back image
    extern ui_style * ui_style_current;
    ui_style_current->box_back_image = P("./assets/themes/custom.tga");

    //ui button example 
    ui_elem_new("button",ui_button);
    ui_button_resize(ui_elem_get("button"),vec2_new(80,30));
    ui_button_move(ui_elem_get("button"),vec2_new(10,10));
    void handle_button(ui_button * btn,void * data)
    {
        ui_toast_popup("Click framerate button");
    }
    ui_button_set_onclick(ui_elem_get("button"),handle_button);

    //ui options example
    ui_elem_new("option",ui_option);
    char * options_level[]= { "SIMPLE","NORMAL","HARDRORE"};
    ui_option_set_options(ui_elem_get("option"),"Options:",3,options_level);
    ui_option_move(ui_elem_get("option"),vec2_new(120,10));
    
    void handle_option(ui_option * opt)
    {
        switch(ui_option_get_selected(opt))
        {   //ui popup example
            case 0: ui_toast_popup("Option select: SIMPLE");   break;
            case 1: ui_toast_popup("Option select: NORMAL");   break;
            case 2: ui_toast_popup("Option select: HARDCORE"); break;
            default:break;
        }
    }
    ui_option_set_onselect(ui_elem_get("option"),handle_option);

    //ui browser example, need more implements functions, just do it! :D
    ui_browser * browser = ui_elem_new("browser",ui_browser);
    void browser_handle(ui_text * text)
    {
        //change directory and select files
        if(!SDL_PathIsDirectory(text->string))
        {
            text->color = vec4_green();
            ui_toast_popup("Browser select file: %s",text->string);
        }else if(SDL_PathIsDirectory(text->string)){
            text->color = vec4_red();
            ui_browser_chdir(ui_elem_get("browser"),P(text->string));
            ui_toast_popup("Browser chande dir: %s",text->string);
        };
    }
    ui_listbox_set_onselect(browser->inner,browser_handle);


    //ui dialog example (dialog not have "active" option, i just create/delete dialog) 
    ui_button * ebtn = ui_elem_new("exit",ui_button);
    ebtn->up_color = vec4_red();
    ui_button_set_label(ui_elem_get("exit"),"Exit");
    ui_button_set_label_color(ui_elem_get("exit"),vec4_red());
    ui_button_resize(ui_elem_get("exit"),vec2_new(120,30));
    ui_button_move(ui_elem_get("exit"),vec2_new(845,10));
    void create_dialog(ui_button * btn,void * data)
    {
    ui_dialog * dia = ui_elem_new("dialog",ui_dialog);
    ui_dialog_set_title(ui_elem_get("dialog"),"Exit :( Or not! :)");
    ui_rectangle_set_texture(dia->back->back,asset_hndl_new(P("./assets/themes/mlp.tga")),0,0,false);
    ui_dialog_set_contents(ui_elem_get("dialog"),"Are you sure you want to exit\n"
                                                 "No, you sure?\n"
                                                 "Are you sure sure? :D\n"
                                                 "Let`s go write code!");
    void dialog_handler(ui_button * btn,void * data)
    {
        if(strstr(btn->label->string,"Exit"))
        {
            running = false;
        }else{
            ui_dialog * dia = ui_elem_get("dialog");
            ui_elem_delete("dialog"); 
        };
    }
    ui_dialog_set_button_left(ui_elem_get("dialog"),"Exit",dialog_handler);
    ui_dialog_set_button_right(ui_elem_get("dialog"),"Stay",dialog_handler);
    }
    ui_button_set_onclick(ui_elem_get("exit"),create_dialog);

    //ui listbox example
    ui_elem_new("listbox",ui_listbox);
    ui_listbox_move(ui_elem_get("listbox"),vec2_new(10,80));
    ui_listbox_resize(ui_elem_get("listbox"),vec2_new(180,210));
    void listbox_handler(ui_text * text)
    {
        ui_toast_popup("Textbox select item: %s",text->string);
    };
    ui_listbox_set_onselect(ui_elem_get("listbox"),listbox_handler); 

    ui_listbox_add_item(ui_elem_get("listbox"),"Listbox example item 1");
    ui_listbox_add_item(ui_elem_get("listbox"),"Listbox example item 2");
    ui_listbox_add_item(ui_elem_get("listbox"),"Listbox example item 3");
    ui_listbox_add_item(ui_elem_get("listbox"),"Listbox example item 4");
    ui_listbox_add_item(ui_elem_get("listbox"),"Listbox example item 5");
    ui_listbox_add_item(ui_elem_get("listbox"),"Listbox example item 6");
    ui_listbox_add_item(ui_elem_get("listbox"),"Listbox example item 7");
    ui_listbox_add_item(ui_elem_get("listbox"),"Listbox example item 8");
    ui_listbox_add_item(ui_elem_get("listbox"),"Listbox example item 9");
    ui_listbox_add_item(ui_elem_get("listbox"),"Listbox example item 10");

    //ui textbox example
    ui_textbox * tb =  ui_elem_new("textbox",ui_textbox);
    tb->contents->color = vec4_new(0.9,0.6,0.7,1);
    ui_textbox_resize(ui_elem_get("textbox"),vec2_new(180,210));
    ui_textbox_move(ui_elem_get("textbox"),vec2_new(200,85));
    ui_textbox_set_max_chars(ui_elem_get("textbox"),20);
    ui_textbox_set_label(ui_elem_get("textbox"),"Textbox \n you can write text! \n limit 20 chars");


    //ui slider example
     ui_slider * vs = ui_elem_new("volume_slider",ui_slider);
     vs->pressed = false;
     vs->bar->color = vec4_green();
     ui_slider_set_amount(ui_elem_get("volume_slider"),1);
     ui_slider_activate(ui_elem_get("volume_slider"));
     ui_slider_set_label(ui_elem_get("volume_slider"),"Music volume");
     ui_slider_move(ui_elem_get("volume_slider"),vec2_new(10,300));

     ui_slider * ss = ui_elem_new("sun_slider",ui_slider);
     ss->pressed = false;
     ss->bar->color = vec4_red();
     ui_slider_set_amount(ui_elem_get("sun_slider"),1);
     ui_slider_activate(ui_elem_get("sun_slider"));
     ui_slider_set_label(ui_elem_get("sun_slider"),"Sun position");
     ui_slider_move(ui_elem_get("sun_slider"),vec2_new(10,350));
     
    //ui spinner example
    ui_spinner * rs = ui_elem_new("red_spinner",ui_spinner);
    rs->color = vec4_green();
    rs->top_left = vec2_new(10,410);
    rs->bottom_right = vec2_add(rs->top_left, vec2_new(24,24));
    
    ui_spinner * gs = ui_elem_new("green_spinner",ui_spinner);
    gs->color = vec4_red();
    gs->top_left = vec2_new(80,410);
    gs->bottom_right = vec2_add(gs->top_left, vec2_new(24,24));

    ui_spinner * bs = ui_elem_new("blue_spinner",ui_spinner);
    bs->color = vec4_blue();
    bs->top_left = vec2_new(160,410);
    bs->bottom_right = vec2_add(bs->top_left, vec2_new(24,24));

    // ui text example
    ui_elem_new("text",ui_text);
    ui_text_set_color(ui_elem_get("text"),vec4_green());
    ui_text_draw_string(ui_elem_get("text"),"Simple text ui element");
    ui_text_move(ui_elem_get("text"),vec2_new(600,600));
    ui_text_set_scale(ui_elem_get("text"),vec2_new(2,2));

    ui_elem_new("text2",ui_text);
    ui_text_set_color(ui_elem_get("text2"),vec4_red());
    ui_text_draw_string(ui_elem_get("text2"),"Simple text ui element");
    ui_text_move(ui_elem_get("text2"),vec2_new(600,640));
    ui_text_set_scale(ui_elem_get("text2"),vec2_new(1.8,1.8));

    ui_elem_new("text3",ui_text);
    ui_text_set_color(ui_elem_get("text3"),vec4_white());
    ui_text_draw_string(ui_elem_get("text3"),"Simple text ui element");
    ui_text_move(ui_elem_get("text3"),vec2_new(600,680));
    ui_text_set_scale(ui_elem_get("text3"),vec2_new(1.5,1.5));

    SDL_Event e = {0};
    float moving = 0.00005;
    float movinl = false;
    float rotate_object = 0.1;
    float rotate_box = 0.1;
    while(running)
    {
        frame_begin();
        frame_end_at_rate(240);
        ui_button_set_label(ui_elem_get("button"),frame_rate_string());
        audio_music_set_volume(ui_slider_get_amount(ui_elem_get("volume_slider")));
        renderer_set_tod(dr,ui_slider_get_amount(ui_elem_get("sun_slider")),1);
        while(SDL_PollEvent(&e))
        {
            ui_event(e);
            switch(e.type) {
            case SDL_KEYDOWN:{

                             }break;
            case SDL_KEYUP:
                             {

                             };
              if (e.key.keysym.sym == SDLK_ESCAPE) { create_dialog(NULL,NULL); }
              if (e.key.keysym.sym == SDLK_p) { graphics_viewport_screenshot(); }

            break;
            case SDL_QUIT:
            running = 0;
            break;
          }
        };
        entity_get_as("cam",camera)->position = vec3_smootherstep(vec3_new(10,10,10),vec3_new(90,120,80),moving);
        entity_get_as("cam",camera)->position = vec3_smootherstep(vec3_new(90,120,80),vec3_new(300,10,-25),moving);
        entity_get_as("cam",camera)->position = vec3_smootherstep(vec3_new(300,10,-25),vec3_new(10,10,10),moving);
        entity_get_as("cam",camera)->position = vec3_smootherstep(vec3_new(-300,10,-25),vec3_new(10,10,10),moving);

        if(movinl == true && moving > 1.5)
        {
            movinl = false;
        };
        if(movinl == false && moving < -0.5)
        {
            movinl = true;
        };
        if(movinl)
        {
            moving += frame_time() / 2;
        }else{
            moving -= frame_time() / 2;
        };

        entity_get_as("magic",particles)->position = entity_get_as("cam",camera)->position;
        entity_get_as("magic",particles)->position.x  = 10;
        entity_get_as("magic",particles)->position.z += 10;
        particles_update(entity_get("magic"),frame_time()*8,entity_get("cam"));

        entity_get_as("object",static_object)->rotation = quat_rotation_y(rotate_object -=frame_time()*5);
        entity_get_as("box",static_object)->rotation = quat_rotation_y(rotate_box +=frame_time()*5);

        renderer_add(dr,render_object_particles(entity_get("magic")));
        renderer_add(dr,render_object_static(entity_get("object")));
        renderer_add(dr,render_object_static(entity_get("box")));

        renderer_render(dr);
        ui_update();
        ui_render();
        graphics_swap();
        frame_end();
    };

    corange_finish();
    return 0;
}







//example how use ui_rectangle
void splash(float speed, bool fake_loading)
{
    //create splash
    ui_rectangle * splash = ui_elem_new("_splash_",ui_rectangle);
    splash->top_left = vec2_new(0,0);
    ui_rectangle_resize(splash,vec2_new(graphics_viewport_width(),graphics_viewport_height()));
    //set texture and filtrating for splash
    texture * splash_texture = asset_get(P("./assets/splash/splash.tga"));
    texture_set_filtering_anisotropic(splash_texture);
    ui_rectangle_set_texture(splash,asset_hndl_new(P("./assets/splash/splash.tga")),0,0,false);
    //create loading bar
    ui_rectangle * fake_load = ui_elem_new("_load_",ui_rectangle);
    fake_load->color = vec4_red();
    ui_rectangle_resize(ui_elem_get("_load_"),vec2_new(10,10));
    ui_rectangle_move(ui_elem_get("_load_"),vec2_new(100,graphics_viewport_height()-250));
    //enable disable loading bar
    if(fake_loading)
    {
        fake_load->active = true;
    }else{
        fake_load->active = false;
    };
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
            //resize load bar to right
            if(fake_load->top_left.x < graphics_viewport_width()-100)
            {
                fake_load->top_left.x +=10;
            }
            //dynamic color load bar red to green
            fake_load->color.x -= speed;
            fake_load->color.y += speed;
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
    ui_elem_delete("_load_");
};


