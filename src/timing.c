#include <time.h>
#include <stdio.h>

#include "timing.h"

static long time_start = 0;
static long time_split = 0;
static long time_stop = 0;

void timer_start() {
  
  time_start = clock();
  time_split = clock();
  printf("Timer Started\n");

}

void timer_split() {
  
  long time = clock();
  float difference = (float)(time - time_split) / (float)CLOCKS_PER_SEC;

  printf("Timer Split: %f\n", difference);
  
  time_split = clock();
  
}

void timer_stop() {

  long time = clock();
  float difference = (float)(time - time_start) / (float)CLOCKS_PER_SEC;

  printf("Timer End: %f\n", difference);
  
  time_stop = clock();

}


static char frame_rate_string_var[12] = "";

static int frame_rate_var = 0;
static float frame_time_var = 0.0;
static float frame_update_rate = 0.1;

static long frame_start_time, frame_end_time;

static int frame_counter = 0;
static float frame_acc_time = 0.0;

void frame_begin() {
  frame_start_time = clock();
};

void frame_end() {
  
  frame_end_time = clock();
  
  frame_counter++;
  
  frame_time_var = ((float)(frame_end_time - frame_start_time) / (float)CLOCKS_PER_SEC);
  frame_acc_time += frame_time_var;
  
  if (frame_acc_time > frame_update_rate) {  
    frame_rate_var = (int)(frame_counter / frame_update_rate);
    frame_counter = 0;
    frame_acc_time = 0.0;  
  }

  itoa(frame_rate_var, frame_rate_string_var, 10);
  
};

float frame_rate() {
  return frame_rate_var;
};

float frame_time() {
  return frame_time_var;
};

char* frame_rate_string() {
  return frame_rate_string_var;
};