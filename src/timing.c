#include <time.h>
#include <stdio.h>
#include <math.h>

#include "SDL/SDL.h"

#include "vector.h"
#include "error.h"

#include "timing.h"

static unsigned long time_start = 0;
static unsigned long time_split = 0;
static unsigned long time_stop = 0;

void timer_start() {
  
  time_start = SDL_GetTicks();
  time_split = SDL_GetTicks();
  debug("Timer Started");

}

void timer_split() {
  
  unsigned int time = SDL_GetTicks();
  double difference = (double)(time - time_split) / 1000.0;

  debug("Timer Split: %f", difference);
  
  time_split = SDL_GetTicks();
  
}

void timer_stop() {

  unsigned int time = SDL_GetTicks();
  double difference = (double)(time - time_start) / 1000.0f;

  debug("Timer End: %f", difference);
  
  time_stop = SDL_GetTicks();

}

void timestamp_sm(char* out) {
    
    time_t ltime;
    struct tm *time_val;

    ltime=time(NULL);
    time_val=localtime(&ltime);

    sprintf(out, "%d%d%d%d%d%d",
            time_val->tm_mday,
            time_val->tm_mon,
            time_val->tm_year,
            time_val->tm_hour,
            time_val->tm_min,
            time_val->tm_sec);
}

static char frame_rate_string_var[12];

static int frame_rate_var = 0;
static double frame_time_var = 0.0;

static unsigned long frame_start_time = 0.0;
static unsigned long frame_end_time = 0.0;

static const double frame_update_rate = 0.5;

static int frame_counter = 0;
static double frame_acc_time = 0.0;

void frame_begin() {
  frame_start_time = SDL_GetTicks();
}

void frame_end() {
  
  frame_end_time = SDL_GetTicks();
  
  frame_time_var = ((double)(frame_end_time - frame_start_time) / 1000.0f);
  frame_acc_time += frame_time_var;
  frame_counter++;
  
  if (frame_acc_time > frame_update_rate) {  
    frame_rate_var = round((double)frame_counter / frame_acc_time);
    frame_counter = 0;
    frame_acc_time = 0.0;  
  }
  
  sprintf(frame_rate_string_var,"%i",frame_rate_var);
  
}

void frame_end_at_rate(double fps) {
  
  unsigned long end_ticks = SDL_GetTicks();
  double active_frame_time = ((double)(end_ticks - frame_start_time) / 1000.0f);
  
  double wait = (1.0f / fps) - active_frame_time;
  int milliseconds = max(wait, 0) * 1000;
  
  SDL_Delay(milliseconds);
  
  frame_end();
  
}

double frame_rate() {
  return frame_rate_var;
}

double frame_time() {
  return frame_time_var;
}

char* frame_rate_string() {
  return frame_rate_string_var;
}