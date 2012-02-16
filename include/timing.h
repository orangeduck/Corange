#ifndef timing_h
#define timing_h

void timer_start();
void timer_split();
void timer_stop();

void frame_begin();
void frame_end();
void frame_end_at_rate(double fps);

double frame_rate();
double frame_time();
char* frame_rate_string();

void timestamp_sm(char* out);

#endif