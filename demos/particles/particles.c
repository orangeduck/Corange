#include "corange.h"



int main(int argc, char *argv[])
{
    corange_init("../../assets_core");
    asset_hndl opt_graphics = asset_hndl_new_load(P("./graphics.cfg"));
    renderer * dr = renderer_new(opt_graphics);
    entity_new("cam",camera);
    entity_get_as("cam",camera)->position.z +=20;
    entity_new("fire",particles);
    entity_get_as("fire",particles)->position.y +=30;
    folder_load_recursive(P("./assets"));
    landscape* world = entity_new("world", landscape);
    world->scale  = 0.2;
    world->size_x = 256;
    world->size_y = 256;
    world->heightmap  = asset_hndl_new_load(P("./assets/map/map.raw"));
    world->attribmap  = asset_hndl_new_load(P("./assets/map/att.tga"));
    world->ground0    = asset_hndl_new_load(P("./assets/map/diffuse.tga"));
    world->ground3    = asset_hndl_new_load(P("./assets/map/diffuse.tga"));
    world->ground2    = asset_hndl_new_load(P("./assets/map/diffuse.tga"));
    world->ground1    = asset_hndl_new_load(P("./assets/map/diffuse.tga"));
    world->ground0_nm = asset_hndl_new_load(P("./assets/map/normal.tga"));
    world->ground3_nm = asset_hndl_new_load(P("./assets/map/normal.tga"));
    world->ground2_nm = asset_hndl_new_load(P("./assets/map/normal.tga"));
    world->ground1_nm = asset_hndl_new_load(P("./assets/map/normal.tga"));

    landscape_blobtree_generate(world);
    particles_set_effect(entity_get("fire"),asset_hndl_new(P("./assets/fire/fire.effect")));
    
    for (int i = 0; i < 12; i++)
    {
        entity_new("snow%i",particles,i);
        particles_set_effect(entity_get("snow%i",i),asset_hndl_new_load(P("./assets/snow/snow.effect")));
    };

    renderer_set_camera(dr,entity_get("cam"));
    renderer_set_sea_enabled(dr,true);
    renderer_set_skydome_enabled(dr,true);
    renderer_set_tod(dr,0.15,1);

    audio_music_play(asset_get(P("./assets/sound/font/font.mp3")));
    audio_sound_play(asset_get(P("./assets/sound/steps/1.wav")),100);
    audio_sound_play(asset_get(P("./assets/sound/steps/2.wav")),100);
    audio_sound_play(asset_get(P("./assets/sound/steps/3.wav")),100);
    audio_sound_play(asset_get(P("./assets/sound/wall/wall.wav")),100);
    audio_sound_play(asset_get(P("./assets/sound/fire/fire.wav")),100);
    audio_sound_pause(0);
    audio_sound_pause(1);
    audio_sound_pause(2);
    sound * s = asset_get(P("./assets/sound/wall/wall.wav"));
    s->sample->volume=15;
    s = asset_get(P("./assets/sound/steps/1.wav"));
    s->sample->volume=80;
    s = asset_get(P("./assets/sound/steps/2.wav"));
    s->sample->volume=80;
    s = asset_get(P("./assets/sound/steps/3.wav"));
    s->sample->volume=80;
    s = asset_get(P("./assets/sound/fire/fire.wav"));
    s->sample->volume=50;
    audio_music_set_volume(0.4);
    bool running = true;


    while(running)
    {
        frame_begin();
        frame_end_at_rate(60);
        SDL_Event e = {0};
        while(SDL_PollEvent(&e))
        {
            switch(e.type) {
            case SDL_KEYDOWN:{
                                if(e.key.keysym.sym == SDLK_a) audio_sound_resume(2);
                                if(e.key.keysym.sym == SDLK_w) audio_sound_resume(0);
                                if(e.key.keysym.sym == SDLK_s) audio_sound_resume(1);
                                if(e.key.keysym.sym == SDLK_d) audio_sound_resume(2);
                             }break;
            case SDL_KEYUP:
                             {
                                if(e.key.keysym.sym == SDLK_a) audio_sound_pause(2);
                                if(e.key.keysym.sym == SDLK_w) audio_sound_pause(0);
                                if(e.key.keysym.sym == SDLK_s) audio_sound_pause(1);
                                if(e.key.keysym.sym == SDLK_d) audio_sound_pause(2);
                             };
              if (e.key.keysym.sym == SDLK_ESCAPE) { running = false; }
              if (e.key.keysym.sym == SDLK_p) { graphics_viewport_screenshot(); }
              if (e.key.keysym.sym == SDLK_r) {
                asset_reload_all();
              };
            break;
            case SDL_QUIT:
            running = 0;
            break;
          }
        };

        camera_control_freecam(entity_get("cam"),frame_time());
        camera* cam = entity_get("cam");
        landscape* world = entity_get("world");
    
        vec3 cam_dir = vec3_normalize(vec3_sub(cam->target, cam->position));
        float height = landscape_height(world, vec2_new(cam->position.x, cam->position.z));
        cam->position.y = height + 15;
        cam->target = vec3_add(cam->position, cam_dir); 

        particles_update(entity_get("fire"),frame_time()*2.1,entity_get("cam"));
        renderer_add(dr,render_object_particles(entity_get("fire")));
        renderer_add(dr,render_object_landscape(entity_get("world")));


        for (int i = 0; i < 12; i++)
        {

            particles_update(entity_get("snow%i",i),frame_time(),entity_get("cam"));
            renderer_add(dr,render_object_particles(entity_get("snow%i",i))); 
        }
        for (int i = 0; i < 12; i++)
        {
            particles * par = entity_get("snow%i",i);
            par->position = entity_get_as("cam",camera)->position;
            par->position.y += 25;
            par->position.z += (-15 + i*3+0.5);
 
        }

        renderer_render(dr);
        graphics_swap();
        frame_end();
    };

    corange_finish();
    return 0;
}
