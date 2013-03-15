#include "corange.h"
  
static float perlin_noise2D(vec2 v) {
  
  vec2 amount = vec2_fmod(v,1.0);
  
  vec2 p1 = vec2_floor(v);
  vec2 p2 = vec2_add(p1, vec2_new(1,0) );
  vec2 p3 = vec2_add(p1, vec2_new(0,1) );
  vec2 p4 = vec2_add(p1, vec2_new(1,1) );
  
  int h1 = vec2_mix_hash(p1);
  int h2 = vec2_mix_hash(p2);
  int h3 = vec2_mix_hash(p3);
  int h4 = vec2_mix_hash(p4);
  
  double result = bismootherstep_interp(h2, h1, h4, h3, amount.x, amount.y);
  
  return result / INT_MAX;
  
}


static image* perlin_noise_generate(int x_size, int y_size, int octaves) {
  
  unsigned char* data = malloc(sizeof(char) * x_size * y_size);
  
  int i, x, y;
  
  /* Clear data to average */
  for(x = 0; x < x_size; x++)
  for(y = 0; y < y_size; y++) {
    data[x + (y * x_size)] = 128;
  }
  
  srand(time(NULL));
  
  debug("Generating perlin noise.");
  
  for(i = 0; i < octaves; i++ ) {
  
    float wavelength = pow( 2, i);
    float amplitude = pow( 0.5, octaves-i );
    vec2 seed = vec2_new(rand(),rand());
    
    debug("Octave: %i Wavelength: %f Amplitude: %f", i, wavelength, amplitude);

    for(x = 0; x < x_size; x++)
    for(y = 0; y < y_size; y++) {
      
      /* Four positions are required for tiling behaviour */
      
      vec2 pos, pos_x, pos_y, pos_xy;
      
      pos = vec2_div( vec2_new(x, y) , wavelength );
      pos_x = vec2_div( vec2_new(x - x_size, y) , wavelength );
      pos_y = vec2_div( vec2_new(x, y - y_size) , wavelength );
      pos_xy = vec2_div( vec2_new(x - x_size, y - y_size) , wavelength );
      
      pos = vec2_add( pos, seed );
      pos_x = vec2_add( pos_x, seed );
      pos_y = vec2_add( pos_y, seed );
      pos_xy = vec2_add( pos_xy, seed );
      
      float val = perlin_noise2D(pos) * amplitude;
      float val_x = perlin_noise2D(pos_x) * amplitude;
      float val_y = perlin_noise2D(pos_y) * amplitude;
      float val_xy = perlin_noise2D(pos_xy) * amplitude;
      
      val = bilinear_interp(val_x, val, val_xy, val_y, (float)x/x_size, (float)y/y_size);
      
      data[x + (y * x_size)] += val * 128;
    }
  
  }
  
  unsigned char* image_data = malloc(sizeof(char) * 4 * x_size * y_size);
  for(x = 0; x < x_size; x++)
  for(y = 0; y < y_size; y++) {
    int amount = data[x + y * x_size];
    image_data[x * 4 + y * x_size * 4 + 0] = amount;
    image_data[x * 4 + y * x_size * 4 + 1] = amount;
    image_data[x * 4 + y * x_size * 4 + 2] = amount;
    image_data[x * 4 + y * x_size * 4 + 3] = 255;
  }
  
  free(data);
  
  return image_new(x_size, y_size, image_data);
}

static float shader_time = 0.0;

static void noise_render() {
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
	glMatrixMode(GL_PROJECTION);
  glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
  
	glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
	glLoadIdentity();
  
  material* noise_mat = asset_get(P("./shaders/noise.mat"));
  
  GLuint handle = shader_program_handle(material_get_entry(noise_mat, 0)->program);
  GLuint random_tex = texture_handle(asset_get(P("$CORANGE/resources/random.dds")));
  
  glUseProgram(handle);
  
  glActiveTexture(GL_TEXTURE0 + 0 );
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, random_tex);
  glUniform1i(glGetUniformLocation(handle, "noise_texture"), 0);
  
  glUniform1f(glGetUniformLocation(handle, "time"), shader_time);
  
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0,  0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0,  1.0,  0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0,  1.0,  0.0f);
	glEnd();
	
	glActiveTexture(GL_TEXTURE0 + 0 );
  glDisable(GL_TEXTURE_2D);
	
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  
  glUseProgram(0);
  
}

static bool currently_saving = false;
static int save_noise_to_file_thread(void* unused) {

  image* noise = perlin_noise_generate(512, 512, 8);
  image_tga_save_file(noise, "./perlin_noise.tga");
  debug("Noise saved as perlin_noise.tga");
  image_delete(noise);
  
  ui_spinner* save_spinner = ui_elem_get("save_spinner");
  ui_rectangle* spinner_box = ui_elem_get("spinner_box");
  save_spinner->color = vec4_new(1,1,1,0);
  spinner_box->color = vec4_new(0,0,0,0);
  spinner_box->border_color = vec4_new(1,1,1,0);
  
  currently_saving = false;
  
  return 0;
}

static SDL_Thread* save_thread = NULL;
static void save_noise_to_file(ui_button* b) {
  
  ui_spinner* save_spinner = ui_elem_get("save_spinner");
  save_spinner->color = vec4_new(1,1,1,1);
  
  save_thread = SDL_CreateThread(save_noise_to_file_thread, NULL);
  
}


int main(int argc, char **argv) {
  
  corange_init("../../core_assets");
  
  graphics_viewport_set_dimensions(1280, 720);
  graphics_viewport_set_title("Noise");
  
  folder_load(P("./shaders/"));
  file_load(P("$CORANGE/resources/random.dds"));
  
  glClearColor(1.0, 0.0, 0.0, 1.0);
  
  ui_button* info_button = ui_elem_new("info_button", ui_button);
  ui_button_move(info_button, vec2_new(10, 10));
  ui_button_resize(info_button, vec2_new(460,25));
  ui_button_set_label(info_button, "Procedural texture from perlin noise and feedback functions.");
  
  ui_button* save_button = ui_elem_new("save_button", ui_button);
  ui_button_move(save_button, vec2_new(480, 10));
  ui_button_resize(save_button, vec2_new(380,25));
  ui_button_set_label(save_button, "Click Here to save tileable perlin noise to file.");
  ui_button_set_onclick(save_button, save_noise_to_file);
  
  ui_rectangle* spinner_box = ui_elem_new("spinner_box", ui_rectangle);
  spinner_box->color = vec4_black();
  spinner_box->border_color = vec4_white();
  spinner_box->border_size = 1;
  spinner_box->top_left = vec2_new(870, 7);
  spinner_box->bottom_right = vec2_new(902, 39);
  
  ui_spinner* save_spinner = ui_elem_new("save_spinner", ui_spinner);
  save_spinner->color = vec4_new(1,1,1,0);
  save_spinner->top_left = vec2_new(874, 11);
  save_spinner->bottom_right = vec2_add(save_spinner->top_left, vec2_new(24,24));
  
  srand(time(NULL));
  shader_time = (float)rand() / (RAND_MAX / 1000);
  
  bool running = true;
  while(running) {
    frame_begin();
    
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      
      switch(event.type){
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        if (event.key.keysym.sym == SDLK_ESCAPE) { running = 0; }
        if (event.key.keysym.sym == SDLK_PRINT) { graphics_viewport_screenshot(); }
        break;
      case SDL_QUIT:
        running = 0;
        break;
      break;
      }
      
      ui_event(event);
      
    }
    
    shader_time += frame_time();
    ui_update();
    
    noise_render();
    ui_render();
    
    SDL_GL_SwapBuffers();
    
    frame_end();
    
  }  
  
  SDL_WaitThread(save_thread, NULL);
  
  corange_finish();
  
  return 0;
}
