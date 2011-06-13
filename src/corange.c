#include <stdlib.h>

#define GLEW_STATIC
#include "GL/glew.h"

#define NO_SDL_GLEXT
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_image.h"

#include "frame.h"
#include "font.h"
#include "texture.h"
#include "camera.h"
#include "vector.h"
#include "geometry.h"
#include "material.h"
#include "scripting.h"
#include "shader.h"

#include "deferred_renderer.h"
#include "forward_renderer.h"
#include "text_renderer.h"

#include "asset_manager.h"
#include "obj_loader.h"

#include "logger.h"

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600

void PrintGL_Error() {
  
  GLenum error_code = glGetError();
  
  if (error_code == GL_NO_ERROR) {
    log_error("OpenGL Error: No Error\n",0);
  } else if (error_code == GL_INVALID_ENUM) {
    log_error("OpenGL Error: Invalid Enum\n",0);
  } else if (error_code == GL_INVALID_VALUE) {
    log_error("OpenGL Error: Invalid Value\n",0);
  } else if (error_code == GL_INVALID_OPERATION) {
    log_error("OpenGL Error: Invalid Operation\n",0);
  } else if (error_code == GL_STACK_OVERFLOW) {
    log_error("OpenGL Error: Stack Overflow\n",0);
  } else if (error_code == GL_STACK_UNDERFLOW) {
    log_error("OpenGL Error: Stack Underflow\n",0);
  } else if (error_code == GL_OUT_OF_MEMORY) {
    log_error("OpenGL Error: Out of Memory\n",0);
  }
  
}

main(int argc, char *argv[]) {
  
  /* Stop Redirect of stdout and stderr */
  
  FILE * ctt = fopen("CON", "w" );
  freopen( "CON", "w", stdout );
  freopen( "CON", "w", stderr );
  
  /* SDL Setup */
  
  SDL_Surface *screen;
  
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    log_error("Unable to initialize SDL: %s\n", SDL_GetError());
    return 1;
  }
  
  /* Init SDL image */
  
  int flags = IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF;
  int initted = IMG_Init(flags);
  if(initted&flags != flags) {
      log_error("IMG_Init: Failed to init required jpg and png support!\n",0);
      log_error("IMG_Init: %s\n", IMG_GetError());
  }
  
  /* Set Window properties */
  
  SDL_WM_SetCaption("Corange","Corange");    
  SDL_Surface* image = IMG_Load("icon.png");
  SDL_WM_SetIcon(image, NULL);
  
  SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 8 );
  
  screen = SDL_SetVideoMode(DEFAULT_WIDTH, DEFAULT_HEIGHT, 0, SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL);
  if (screen == NULL) {
    log_error("Unable to set video mode: %s\n", SDL_GetError());
    return 1;
  }
  
  /* OpenGL setup */
  
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    log_error("Glew Error: %s\n", glewGetErrorString(err));
  }
  
  /* New Camera */
  
  camera* cam = camera_new( v3(20.0, 0.0, 0.0) , v3_zero() );
  
  /* Renderer Setup */
  
#ifdef DEFERRED_RENDER
  deferred_renderer_init(DEFAULT_WIDTH, DEFAULT_HEIGHT);
  deferred_renderer_set_camera(cam);
#else
  forward_renderer_init(DEFAULT_WIDTH, DEFAULT_HEIGHT);
  forward_renderer_set_camera(cam);
#endif
  
  /* End openGL setup */
  
  /* Setup Scripting */
  
  scripting_init();
  
  /* Load Assets */
  
  asset_manager_init();
  
  asset_manager_handler("obj", (void*(*)(char*))obj_load_file, (void(*)(void*))model_delete);
  
  asset_manager_handler("dds", (void*(*)(char*))dds_load_file, (void(*)(void*))texture_delete);
  asset_manager_handler("bmp", (void*(*)(char*))bmp_load_file, (void(*)(void*))texture_delete);
  asset_manager_handler("png", (void*(*)(char*))png_load_file, (void(*)(void*))texture_delete);
  asset_manager_handler("tif", (void*(*)(char*))tif_load_file, (void(*)(void*))texture_delete);
  asset_manager_handler("jpg", (void*(*)(char*))jpg_load_file, (void(*)(void*))texture_delete);
  
  asset_manager_handler("vs" , (void*(*)(char*))vs_load_file,  (void(*)(void*))shader_delete);
  asset_manager_handler("fs" , (void*(*)(char*))fs_load_file,  (void(*)(void*))shader_delete);
  asset_manager_handler("prog",(void*(*)(char*))prog_load_file,(void(*)(void*))shader_program_delete);
  
  asset_manager_handler("fnt", (void*(*)(char*))font_load_file,(void(*)(void*))font_delete);
  asset_manager_handler("mat", (void*(*)(char*))mat_load_file, (void(*)(void*))material_delete);
  asset_manager_handler("lua", (void*(*)(char*))lua_load_file, (void(*)(void*))script_delete);
  
  load_folder("./Engine/Assets/Shaders/");
  load_folder("./Engine/Assets/Textures/");
  load_folder("./Engine/Assets/Meshes/");
  load_folder("./Engine/Assets/Fonts/");
  load_folder("./Engine/Scripts/");
  
  /* Get reference to the Piano */
  
  render_model* piano = (render_model*)asset_get("./Engine/Assets/Meshes/piano.obj");
  material* piano_mat = (material*)asset_get("./Engine/Assets/Meshes/piano.mat");
  
  render_model* cello = (render_model*)asset_get("./Engine/Assets/Meshes/cello.obj");
  
  /* Put some text on the screen */
  
  font* console_font = (font*)asset_get("./Engine/Assets/Fonts/console_font.fnt");
  
  render_text* rt_framerate = render_text_new("hello", 10, console_font);
  rt_framerate->position = v2(-1.0,-1.0);
  rt_framerate->scale = v2(1.0,1.0);
  rt_framerate->color = v4(0,0,0,1);
  render_text_update(rt_framerate);
  
  render_text* rt_test_text = render_text_new("Corange v0.1", 512, console_font);
  rt_test_text->position = v2(-1.0,-0.95);
  rt_test_text->color = v4(0,0,1,1);
  render_text_update(rt_test_text);
  
  script* s = (script*)asset_get("./Engine/Scripts/hello_world.lua");
  scripting_run_script(s);
  
  /* Start */
  
  SDL_Event event;
  int running = 1;
  
  while(running) {
  
    frame_begin();
  
    while(SDL_PollEvent(&event)) {
      switch(event.type){
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        
        if (event.key.keysym.sym == SDLK_ESCAPE) { running = 0; }
        
        if (event.key.keysym.sym == SDLK_UP) { cam->position.y += 1; }
        if (event.key.keysym.sym == SDLK_DOWN) { cam->position.y -= 1; }
        
        if (event.key.keysym.sym == SDLK_LEFT) { cam->position.z -= 1; }
        if (event.key.keysym.sym == SDLK_RIGHT) { cam->position.z += 1; }

        if (event.key.keysym.sym == SDLK_LEFTBRACKET) { cam->position.x += 1; }
        if (event.key.keysym.sym == SDLK_RIGHTBRACKET) { cam->position.x -= 1;}
        
        break;
      case SDL_QUIT:
        running = 0;
        break;
      }
    }
    
    /* Begin Rendering */
    
    
#ifdef DEFERRED_RENDER
  deferred_renderer_begin();
  deferred_renderer_render_model(piano, piano_mat);
  //deferred_renderer_render_model(cello);
  deferred_renderer_end();
#else
  forward_renderer_begin();
  forward_renderer_render_model(piano, piano_mat);
  //forward_renderer_render_model(cello);
  forward_renderer_end();
#endif
    
    
    /* Render text */
    
    render_text_update_string(rt_framerate, frame_rate_string());
    
    render_text_render(rt_framerate);
    render_text_render(rt_test_text);
    
    glFlush();
    SDL_GL_SwapBuffers();    
    
    /* End Rendering */    
    
    frame_end();
  
  }	
  
  /* Finish */
  
#ifdef DEFERRED_RENDER
  deferred_renderer_finish();
#else
  forward_renderer_finish();
#endif
  
  /* Unload assets */
  
  asset_manager_finish();
  
  scripting_finish();
  
  render_text_delete(rt_framerate);
  render_text_delete(rt_test_text);
  
  camera_delete(cam);
  
  IMG_Quit();
  SDL_Quit();
  
  return 0;
}
