#include "corange.h"

typedef SDL_Event event;
static event event_common_event = { 0 };
static bool  event_common_state = false;
static float tod = 0.75;   //default sun
static bool goup = false;   //scene prepare lauch
static bool gogoup = false; //scene go to home
static bool running = true;
static renderer * dr = NULL;
static bool the_end = false;

Uint8 distconvert(float value,float max)
{
    return clamp(((254/max)*value),0,254);
}

int sound_play_at_looped_custom(sound* s, vec3 pos, vec3 cam_pos, vec3 cam_dir, int loops) {
  int c = sound_play_looped(s, loops);
  
  const float HEARING = 5.0;//5
  
  float distance = vec3_dist(pos, cam_pos);

  Uint8 dist_val =  distconvert(distance,1500);//maximum listen distance 
  const float DEGREES =114.591559026;// 2 radian for 360 angles

  vec3 to_position = vec3_normalize(vec3_sub(pos, cam_pos));
  vec3 to_forward  = vec3_normalize(cam_dir);
  float angle = acos( vec3_dot(to_position, to_forward));
  Sint16 angle_val = DEGREES * angle;

  Mix_SetPosition(c, angle_val, dist_val);

  return c;
}
int sound_play_at_custom(sound* s, vec3 pos, vec3 cam_pos, vec3 cam_dir) {
  return sound_play_at_looped_custom(s, pos, cam_pos, cam_dir, 0);
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
void talks(int count)
{
    switch(count)
    {
        case 270:
        {
            ui_toast_popup("S: Hi Billi, so that we could pick you up you have to clear the landing area - Reception");
        }break;
        case 265:
        {
            ui_toast_popup("B: Yes, I have already set up a robot manipulator, on this planet something strange! - Reception");
        }break;
        case 260:
        {
            ui_toast_popup("S:What do you mean?");
        }break;
        case 255:
        {
            ui_toast_popup("B:Um, that garbage you see from ortiba, it's u cupcake");
        }break;
        case 240:
        {
            ui_toast_popup("S:What?! This is an ordinary planetoid, did you use something?");
        }break;
        case 235:
        {
            ui_toast_popup("B:No, I'm really telling you this cupcakes! From dough and raisins!");
        }break;
        case 220:
        {
            ui_toast_popup("S:Billy, wait a minute, keep cleaning ... cupcakes from the playground");
        }break;
        case 215:
        {
            ui_toast_popup("B:Yes, I clean it will take quite a lot of time.");
        }break;
        case 210:
        {
            ui_toast_popup("S:We have time, we will get back to you soon. Billy");
        }break;
        case 200:
        {
            ui_toast_popup("B:Okey");
        }break;
        case 199:
        {
            ui_toast_popup("S:...");
        }break;
        case 160:
        {
            ui_toast_popup("S:Billy?");
        }break;
        case 155:
        {
            ui_toast_popup("B:Yes, I'm here, continue cleaning");
        }break;
        case 150:
        {
            ui_toast_popup("S:Well, Billy, and what do you do with what you collected, well, with these cupcakes?");
        }break;
        case 145:
        {
            ui_toast_popup("B:I eat them in the reactor");
        }break;
        case 140:
        {
            ui_toast_popup("S:Could you save the samples? We investigate them and then decide ... how to react to all this.");
        }break;
        case 135:
        {
            ui_toast_popup("B:OK, no problem, I'll put a few kilograms in a container");
        }break;
        case 130:
        {
            ui_toast_popup("B:But I can and a couple of tone if you need ahaha");
        }break;
        case 125:
        {
            ui_toast_popup("B:It's really strange, why aren't you surprised, eh? There at the station hey!");
        }break;
        case 120:
        {
            ui_toast_popup("S:Billy's main goal is to pick you up from there, we can't take care of anything else right now.");
        }break;
        case 115:
        {
            ui_toast_popup("B:I understand you, continue to work");
        }break;
        case 110:
        {
            ui_toast_popup("...");
        }break;
        case 100:
        {
            ui_toast_popup("...");
        }break;
        case 90:
        {
            ui_toast_popup("S:Billy, we see you are almost finished.");
        }break;
        case 85:
        {
            ui_toast_popup("S:First, as soon as you finish, turn on the green lighthouse.");
        }break;
        case 80:
        {
            ui_toast_popup("S:And do not direct the laser to the descent gate to calibrate the landing spot.");
        }break;
        case 75:
        {
            ui_toast_popup("S:Do not forget about the calibration laser!");
        }break;
        case 60:
        {
            ui_toast_popup("S:The shuttle is already ready, it is in ultra-low orbit");
        }break;
        case 55:
        {
            ui_toast_popup("B:I understood you! Hurry to finish the reactor is full");
        }break;
        case 40:
        {
            ui_toast_popup("B:I have to increase the size of my composite transdimensional manipulator");
        }break;
        case 35:
        {
            ui_toast_popup("S:We fly for you!");
        }break;
        case 31:
        {
            ui_toast_popup("B:Hooray! Home soon");
        }break;
    }
}
//light pulse
float head_pulse()
{
    static float val = 0;
    if((val+=10) < 1000)
    {
        return val;
    }else{
        val = 0;
        return val;
    }
    return 0;
}
//light pulse
float sonar_pulse()
{
    static float val  = 0;
    static float max  = 500;
    static float step = 100;
    static bool  once = true;
    if(goup && once)
    {
        step*=5;
        max*=10;
        once = false;
    }
    if((val+=step) < max)
    {
        return val;
    }else{
        val = 0;
        vec3 op = entity_get_as("player_body29",static_object)->position;
        camera* cam = entity_get("cam");
        asset_get_as(P("./assets/sound/sonar.wav"),sound)->sample->volume = 10;
        if(goup)
        {
           asset_get_as(P("./assets/sound/sonar.wav"),sound)->sample->volume = 100;
        }
        sound_play_at_custom(asset_get(P("./assets/sound/sonar.wav")),op,cam->position,camera_direction(cam));
        return val;
    }
    return 0;
}

void camera_director(vec3 pos,vec3 pose,float speed)
{
    camera * cam = entity_get("cam");
    cam->position = vec3_lerp(cam->position,pos,frame_time()/speed);
    cam->target = vec3_lerp(cam->target,pose,frame_time()/speed);
}

static int counter = 270;
static float size = 5;
vec3 * lpose(vec3 current_pose)
{
   static int once  = 1;
   static int count = 30;
   if(count == 299)
   {
       if(once)
       {   
            camera * cam = entity_get("cam");
           // asset_get_as(P("./assets/sound/takeend.wav"),sound)->sample->volume = 50;
           // sound_play_at_looped_custom(asset_get(P("./assets/sound/takeend.wav")),vec3_new(10,10,10),cam->position,camera_direction(cam),10);
            asset_get_as(P("./assets/sound/goup.wav"),sound)->sample->volume = 211;
            sound_play_at_custom(asset_get(P("./assets/sound/goup.wav")),vec3_new(10,10,10),cam->position,camera_direction(cam));
            once =0;
       }
        goup=true;
        static vec3 pos;
        pos = entity_get_as("planet",static_object)->position;
        pos.y =30;
        pos.x =150;
        entity_get_as("l2",light)->diffuse_color  = vec3_green();
        entity_get_as("l2",light)->ambient_color  = vec3_green();
        entity_get_as("l2",light)->specular_color = vec3_green();
        entity_get_as("player_body299",static_object)->scale = vec3_new(0,0,0);
        return &pos; 
   };

   camera * cam = entity_get("cam");
   static_object * so = entity_get("player_body%i",count);

   if(count < 299){
   static_object * so2 = entity_get("player_body%i",(count+1));
   entity_get_as("l4",light)->position = so2->position;
   entity_get_as("l4",light)->position.y +=20;
   so2->scale = vec3_add(so2->scale,vec3_new(0.01,0.01,0.01));
   };

   vec3 aa = vec3_sub(current_pose,so->position);
   if(aa.x < 0.001 && aa.y < 0.001 && aa.z < 0.001)
   {
       asset_get_as(P("./assets/sound/take.wav"),sound)->sample->volume = 100;
       sound_play_at_custom(asset_get(P("./assets/sound/take.wav")),current_pose,cam->position,camera_direction(cam));
       size +=0.1;
       talks(counter);
       so->scale = vec3_new(0,0,0);
       if(count++ == 299){count=30;};
       counter--;
   };
    return &so->position;
}

void player_new()
{
    //create composite transdimensional manipulator
    for (int i = 0; i < 30; ++i)
    {
      static_object * so = entity_new("player_body%d",static_object,i);
      so->scale = vec3_new(5,5,5);
      so->position.z -=4*i;
      so->renderable = asset_hndl_new_load(P("./assets/objects/body.obj"));
      renderable * r = asset_hndl_ptr(&so->renderable);
      r->material = asset_hndl_new_load(P("./assets/objects/red.mat"));
    };
    //create UFO cakes!
    for (int i = 30; i < 300; ++i)
    {
      static_object * sop = entity_new("player_body%d",static_object,i);
      sop->scale = vec3_new(6,6,6);
      sop->position.x = randf_range(-300.0,300.0);
      sop->position.y = randf_range(5.0,150.0);
      sop->position.z = randf_range(-300.0,300.0);
      sop->renderable = asset_hndl_new_load(P("./assets/objects/cakes.obj"));
    };

    light * l1 =  entity_new("l1",light);
    light_set_type(l1,LIGHT_TYPE_DIRECTIONAL);
    l1->ambient_color = vec3_green();
    l1->diffuse_color = vec3_blue();
    l1->specular_color = vec3_blue();
    l1->falloff = 1.5;

    light * l2 =  entity_new("l2",light);
    light_set_type(l2,LIGHT_TYPE_POINT);
    l2->ambient_color = vec3_red();
    l2->diffuse_color = vec3_red();
    l2->specular_color = vec3_red();
    l2->falloff = 1.8;
    l2->power = 50;

    light * l3 =  entity_new("l3",light);
    light_set_type(l3,LIGHT_TYPE_POINT);
    l3->ambient_color = vec3_blue();
    l3->diffuse_color = vec3_blue();
    l3->specular_color = vec3_blue();
    l3->falloff = 1.5;
    l3->power = 500;
    l3->target = entity_get_as("cam",camera)->position;

    light * l4 =  entity_new("l4",light);
    light_set_type(l4,LIGHT_TYPE_SPOT);
    l4->ambient_color = vec3_red();
    l4->diffuse_color = vec3_white();
    l4->specular_color = vec3_white();
    l4->falloff = 1.1;
    l4->power = 100;
}

void player_update()
{
   if(goup)
   {
      light * l4 = entity_get("l4");
      static_object * so = entity_get("player_body299");
      static_object * so1 = entity_get("player_body29");
      static_object * sol = entity_get("planet");
      static_object * sor = entity_get("rakete");
      ui_text * mes = ui_elem_get("message");
      camera * cam  = entity_get("cam");
      camera_director(vec3_mul(so->position,2),vec3_mul(so->position,2),8);
      if(!gogoup)
      {
        sor->position =vec3_lerp(sor->position,sol->position,frame_time());
        mes->scale = vec2_new(2,2);
        mes->color = vec4_green();
        ui_text_draw_string(mes,"Prepare lauch wait!");
      }
      //check position
      vec3 aa = vec3_sub(sor->position,sol->position);
      if(aa.x < 0.001 && aa.y < 0.001 && aa.z < 0.001)
      {
           gogoup = true;
      }
      if(gogoup)
      {
           sor->position = vec3_lerp(sor->position,vec3_new(5,3000,5),frame_time());
           l4->position = sor->position;
           sol->position.y -=3; 
           vec3 sor_pos = sor->position;
           sor_pos.x +=100;
           sor_pos.y +=400;
           camera_director(sor_pos,sor_pos,1);
           if(sor->position.y < 2500){
               ui_text_draw_string(mes,"Goooooo!");
           }
           tod=0.01; //reset to hight
           if(sor->position.y > 2500) // final scene
           {
               static bool once = true;
               if(once)
               {
                 void done(ui_button * btn,void * data)
                 {
                   running = false;
                 }
                 ui_button * btn =  ui_elem_new("the_end",ui_button);
                 ui_button_resize(btn,vec2_new(150,30));
                 ui_button_move(btn,vec2_new(graphics_viewport_width()-160,30));
                 ui_button_set_label(btn,"Exit.");
                 ui_button_set_label_color(btn,vec4_red());
                 ui_text_set_scale(btn->label,vec2_new(1.2,1.2));
                 ui_button_set_onclick(btn,done);
                 once = false;
               }
               ui_text_draw_string(mes,"The end");
           }
           if(sor->position.y > 2999)//enable last phylosophy move
           {

               static bool once = true;
               cam->position.x +=5;
               cam->target.x +=5;
               if(once)
               {
                sor->scale = vec3_new(0,0,0);
                static_object * s = entity_get("player_body100");
                s->scale = vec3_new(100,100,100);
                s->position = sor->position;
                ui_toast_popup("S:WTF! Billy! Billy, we became cakes!");
                ui_toast_popup("S:Biiiiiiiiillyyyyy What have you done!");
                ui_toast_popup("B:Nanana, I personally became raisins, so do not yell at me");
                ui_toast_popup("B:So sweet!");
                ui_toast_popup("B:And so weird");
                once=false;
               };
           };
      }else{
      tod =0.17;//reset do day
      };

      entity_get_as("fire",particles)->position = sor->position;
      entity_get_as("l1",light)->position = sor->position;
      particles_update(entity_get("fire"),frame_time(),entity_get("cam"));
      renderer_add(dr,render_object_particles(entity_get("fire")));
      renderer_add(dr,render_object_line(so1->position,sor->position,
                                                           vec3_new(randf_range(0.1,1),
                                                           randf_range(0.1,1),
                                                           randf_range(0.1,1)),10));
   }

   for (int i = 0; i < 30; ++i)
   {
       static_object * so = entity_get("player_body%d",i);
       vec3 * pos = lpose(so->position);
       so->scale = vec3_new(size,size,size);
       if(i==1)// composite transdimensional manipulator head 
       {

          entity_get_as("l1",light)->position = so->position;
          entity_get_as("l1",light)->power = head_pulse();  //head  pulse
          entity_get_as("l2",light)->power = sonar_pulse(); //sonar pulse
          entity_get_as("l1",light)->target = *pos;
       }
       if(i==5)// composite transdimensional manipulator middle body
       {
          entity_get_as("l3",light)->position = so->position;

       }
       if(i==29)// composite transdimensional manipulator sonar body
       {
          entity_get_as("l2",light)->position = so->position;
       }
       so->position = vec3_lerp(so->position, *pos ,(frame_time()/(float)i)*(15+i));
   }

}

void player_render()
{
    renderer_add_dyn_light(dr,entity_get("l1"));
    renderer_add_dyn_light(dr,entity_get("l2"));
    renderer_add_dyn_light(dr,entity_get("l3"));
    renderer_add_dyn_light(dr,entity_get("l4"));
    for (int i = 0; i  < 299; ++i)
    {
        renderer_add(dr,render_object_static(entity_get("player_body%d",i)));
    };
}

int main(int argc, char *argv[])
{
    corange_init("../../assets_core");
    folder_load_recursive(P("./assets/"));
    graphics_viewport_set_size(1280,720);
    dr = renderer_new(asset_hndl_new(P("./assets/graphics.cfg")));
    renderer_set_camera(dr,entity_new("cam",camera));
    renderer_set_skydome_enabled(dr,false);
    camera * cam = entity_get("cam");
    cam->far_clip *=16;
    cam->position = vec3_new(200.16,100.79,3.42);
    cam->target = vec3_new(10.20,15.05,4.09);

    static_object * so = entity_new("planet",static_object);
    so->renderable = asset_hndl_new_load(P("./assets/objects/planet.obj"));
    so->scale = vec3_new(5,5,5);

    particles * p = entity_new("fire",particles);
    particles_set_effect(p,asset_hndl_new(P("./assets/effect/effect.effect")));

    static_object * sor = entity_new("rakete",static_object);
    sor->renderable = asset_hndl_new_load(P("./assets/objects/rakete.obj"));
    sor->scale =  vec3_new(100,100,100);
    sor->position = vec3_new(10,10,10);
    sor->position.y = 20000;

    ui_elem_new("rate",ui_text);
    ui_text_move(ui_elem_new("time",ui_text),vec2_new(30,0));
    ui_text_move(ui_elem_new("score",ui_text),vec2_new(100,0));
    ui_text_move(ui_elem_new("message",ui_text),vec2_new(graphics_viewport_width()-300,30));

    ui_text_set_color(ui_elem_get("rate"),vec4_white());
    ui_text_set_color(ui_elem_get("time"),vec4_white());
    ui_text_set_color(ui_elem_get("score"),vec4_white());
    ui_text_set_color(ui_elem_get("message"),vec4_white());
    player_new();
    while(running)
    {
        frame_begin();
        frame_end_at_rate(60.0);
        while(event_update())
        {
            ui_event(event_get());
        };
        ui_update();
        renderer_set_tod(dr,tod,0);
        player_update();
        player_render();
        camera_control_freecam(entity_get("cam"),frame_time());
        renderer_add(dr,render_object_static(so));
        renderer_add(dr,render_object_static(sor));
        renderer_render(dr);
        ui_render();
        ui_text_draw_string(ui_elem_get("rate"),frame_rate_string());
        char tbuf[12]={0};
        snprintf(tbuf,11,"%f",frame_time());
        ui_text_draw_string(ui_elem_get("time"),tbuf);
        char buf[6]={0};
        snprintf(buf,5,"%i",counter);
        ui_text_draw_string(ui_elem_get("score"),buf);
        graphics_swap();
        frame_end();
    }

    corange_finish();
    return 0;
}
