#include <time.h>
#include <stdio.h>

#include "frame.h"

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