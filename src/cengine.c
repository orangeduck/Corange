#include "cengine.h"

char error_buf[ERROR_BUFFER_SIZE];
char error_str[ERROR_BUFFER_SIZE];

char warning_buf[WARNING_BUFFER_SIZE];
char warning_str[WARNING_BUFFER_SIZE];

char debug_buf[DEBUG_BUFFER_SIZE];
char debug_str[DEBUG_BUFFER_SIZE];

fpath P(const char* path) {
  fpath p;
  
  if (unlikely(strlen(path) >= PATH_MAX)) {
    error("Path too long '%s'", path);
  } else {  
    strcpy(p.ptr, path);
  }
  
  return p;
}

fpath fpath_full(fpath path) {
  fpath ret;
  SDL_PathFullName(ret.ptr, path.ptr);
  return ret;
}

fpath fpath_file(fpath path) {
  fpath ret;
  SDL_PathFileName(ret.ptr, path.ptr);
  return ret;
}

fpath fpath_file_location(fpath path) {
  fpath ret;
  SDL_PathFileLocation(ret.ptr, path.ptr);
  return ret;
}

fpath fpath_file_extension(fpath path) {
  fpath ret;
  SDL_PathFileExtension(ret.ptr, path.ptr);
  return ret;
}

/* Error Functions */

typedef void (*error_func_t)(const char*);
typedef void (*warn_func_t)(const char*);
typedef void (*debug_func_t)(const char*);

#define MAX_AT_FUNCS 32
static error_func_t error_funcs[MAX_AT_FUNCS];
static warn_func_t warn_funcs[MAX_AT_FUNCS];
static debug_func_t debug_funcs[MAX_AT_FUNCS];

static int num_error_funcs = 0;
static int num_warn_funcs = 0;
static int num_debug_funcs = 0;

void at_error(void(*func)(const char*)) {
  if (num_error_funcs == MAX_AT_FUNCS) { 
    warning("Cannot register more than maximum of %i error functions", MAX_AT_FUNCS);
    return;
  }
  
  error_funcs[num_error_funcs] = (error_func_t)func;
  num_error_funcs++;
}

void at_warning(void(*func)(const char*)) {
  if (num_warn_funcs == MAX_AT_FUNCS) { 
    warning("Cannot register more than maximum of %i warning functions", MAX_AT_FUNCS);
    return;
  }
  
  warn_funcs[num_warn_funcs] = (warn_func_t)func;
  num_warn_funcs++;
}

void at_debug(void(*func)(const char*)) {
  if (num_debug_funcs == MAX_AT_FUNCS) { 
    warning("Cannot register more than maximum of %i debug functions", MAX_AT_FUNCS);
    return;
  }
  
  debug_funcs[num_debug_funcs] = (debug_func_t)func;
  num_debug_funcs++;
}

void error_(const char* str) {
  for (int i = 0; i < num_error_funcs; i++) {
    error_funcs[i](str);
  }
}

void warning_(const char* str) {
  for (int i = 0; i < num_warn_funcs; i++) {
    warn_funcs[i](str);
  }
}

void debug_(const char* str) {
  for (int i = 0; i < num_debug_funcs; i++) {
    debug_funcs[i](str);
  }
}

/* Timing Functions */

timer timer_start(int id, const char* tag) {
  
  timer t;
  t.id = id;
  t.start = SDL_GetTicks();
  t.split = SDL_GetTicks();
  t.end = 0;
  
  debug("Timer %d '%s' Start: %f", id, tag, 0.0f);
  
  return t;
}

timer timer_split(timer t, const char* tag) {
  
  long curr = SDL_GetTicks();
  
  double difference = (double)(curr - t.split) / 1000.0;
  debug("Timer %d '%s' Split: %f", t.id, tag, difference);
  
  t.split = curr;
  return t;
}

timer timer_stop(timer t, const char* tag) {
  
  long curr = SDL_GetTicks();
  
  double difference = (double)(curr - t.start) / 1000.0f;
  debug("Timer %d '%s' End: %f", t.id, tag, difference);
  
  t.end = SDL_GetTicks();
  return t;
}

static int timestamp_counter = 0;

void timestamp(char* out) {
    
    time_t ltime;
    struct tm *time_val;

    ltime=time(NULL);
    time_val=localtime(&ltime);

    sprintf(out, "%d%d%d%d%d%d%d",
            time_val->tm_mday,
            time_val->tm_mon,
            time_val->tm_year,
            time_val->tm_hour,
            time_val->tm_min,
            time_val->tm_sec,
            timestamp_counter);
            
  timestamp_counter++;
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

/* Type Functions */

#define MAX_TYPE_LEN 512
#define MAX_NUM_TYPES 1024

typedef char type_string[MAX_TYPE_LEN];
typedef size_t type_size;

static type_string type_names[MAX_NUM_TYPES];
static type_size type_sizes[MAX_NUM_TYPES];

static int type_index = 0;

int type_find(char* type, size_t size) {
  
  /* Ensure can enter type */
  if (type_index >= MAX_NUM_TYPES) {
    error("Too many types in type table already. Cannot add %s.", type);
  }
  
  if (strlen(type) >= MAX_TYPE_LEN) {
    error("Type name %s is too long to index into type table.", type);
  }
  
  /* Search Table for previous entry */
  for (int i = 0; i < type_index; i++) {
    if (strcmp(type, type_names[i]) == 0) {
      return i;
    }
  }
  
  /* Enter into Table */
  strcpy(type_names[type_index], type);
  type_sizes[type_index] = size;
  
  type_index++;
  
  return type_index-1;
}

char* type_id_name(int id) {
  return type_names[id];
}

/* Vector Maths */


int rawcast(float x) {
  union {
    float f;
    int i;
  } u;
  u.f = x;
  return u.i;
}

float max(float x, float y) {
  return x > y ? x : y;
}

float min(float x, float y) {
  return x < y ? x : y;
}

float clamp(float x, float bottom, float top) {
  x = max(x, bottom);
  x = min(x, top);
  return x;
}

bool between(float x, float bottom, float top) {
  return (x > bottom) && (x < top);
}

bool between_or(float x, float bottom, float top) {
  return (x >= bottom) && (x <= top);
}

float saturate(float x) {
  x = max(x, 0.0);
  x = min(x, 1.0);
  return x;
}

float lerp(float p1, float p2, float amount) {
  return (p2 * amount) + (p1 * (1-amount));
}

float smoothstep(float p1, float p2, float amount) {
  float scaled_amount = amount*amount*(3 - 2*amount);
  return lerp( p1, p2, scaled_amount );
}

float smootherstep(float p1, float p2, float amount) {
  float scaled_amount = amount*amount*amount*(amount*(amount*6 - 15) + 10);
  return lerp( p1, p2, scaled_amount );
}

float cosine_interp(float p1, float p2, float amount) {
   float mu2 = (1-cos(amount*M_PI))/2;
   return (p2*(1-mu2)+p1*mu2);
}

float nearest_interp(float p1, float p2, float amount) {
  amount = roundf(amount);
  if (amount) { return p2; }
  else { return p1; }
}

float cubic_interp(float p1, float p2, float p3, float p4, float amount) {
  
  float amount_sqrd = amount * amount;
  float amount_cubd = amount * amount * amount;
  
  float a1 = p4 - p3 - p2 + p1;
  float a2 = p1 - p2 - a1;
  float a3 = p3 - p1;
  float a4 = p2;
  
  return (a1 * amount_cubd) + (a2 * amount_sqrd) + (a3 * amount) + a4;
}

float binearest_interp(float tl, float tr, float bl, float br, float x_amount, float y_amount) {
  
  x_amount = roundf(x_amount);
  y_amount = roundf(y_amount);
  
  if( x_amount && !y_amount ) { return br; }
  if( !x_amount && y_amount ) { return tl; }
  if( !x_amount && !y_amount ) { return bl; }
  if( x_amount && y_amount ) { return tr; }
  
  return 0.0f;
}

float bilinear_interp(float tl, float tr, float bl, float br, float x_amount, float y_amount) {
  float left = lerp(tl, bl, y_amount);
  float right = lerp(tr,br,  y_amount);
  return lerp(right, left, x_amount);
}

float bicosine_interp(float tl, float tr, float bl, float br, float x_amount, float y_amount) {
  float left = cosine_interp(tl, bl, y_amount);
  float right = cosine_interp(tr, br, y_amount);
  return cosine_interp(right, left, x_amount);
}

float bismoothstep_interp(float tl, float tr, float bl, float br, float x_amount, float y_amount) {
  float left = smoothstep(tl, bl, y_amount);
  float right = smoothstep(tr, br, y_amount);
  return smoothstep(right, left, x_amount);
}

float bismootherstep_interp(float tl, float tr, float bl, float br, float x_amount, float y_amount) {

  float left = smootherstep(tl, bl, y_amount);
  float right = smootherstep(tr, br, y_amount);
  return smootherstep(right, left, x_amount);
}

vec2 vec2_new(float x, float y) {
  vec2 v;
  v.x = x;
  v.y = y;
  return v;
}

vec2 vec2_zero() {
  return vec2_new(0, 0);
}

vec2 vec2_one() {
  return vec2_new(1, 1);
}

vec2 vec2_add(vec2 v1, vec2 v2) {
  vec2 v;
  v.x = v1.x + v2.x;
  v.y = v1.y + v2.y;
  return v;
}

vec2 vec2_sub(vec2 v1, vec2 v2) {
  vec2 v;
  v.x = v1.x - v2.x;
  v.y = v1.y - v2.y;
  return v;
}

vec2 vec2_div(vec2 v, float fac) {
  v.x = v.x / fac;
  v.y = v.y / fac;
  return v;
}

vec2 vec2_div_vec2(vec2 v1, vec2 v2) {
  v1.x = v1.x / v2.x;
  v1.y = v1.y / v2.y;
  return v1;
}

vec2 vec2_mul(vec2 v, float fac) {
  v.x = v.x * fac;
  v.y = v.y * fac;
  return v;
}

vec2 vec2_mul_vec2(vec2 v1, vec2 v2) {
  vec2 v;
  v.x = v1.x * v2.x;
  v.y = v1.y * v2.y;
  return v;
}

vec2 vec2_pow(vec2 v, float exp) {
  v.x = pow(v.x, exp);
  v.y = pow(v.y, exp);
  return v;
}

vec2 vec2_neg(vec2 v) {
  v.x = -v.x;
  v.y = -v.y;
  return v;
}

vec2 vec2_abs(vec2 v) {
  v.x = fabs(v.x);
  v.y = fabs(v.y);
  return v;
}

vec2 vec2_floor(vec2 v) {
  v.x = floor(v.x);
  v.y = floor(v.y);
  return v;
}

vec2 vec2_fmod(vec2 v, float val) {
  v.x = fmod(v.x, val);
  v.y = fmod(v.y, val);
  return v;
}

vec2 vec2_max(vec2 v, float x) {
  v.x = max(v.x, x);
  v.y = max(v.y, x);
  return v;
}

vec2 vec2_min(vec2 v, float x) {
  v.x = min(v.x, x);
  v.y = min(v.y, x);
  return v;
}

vec2 vec2_clamp(vec2 v, float b, float t) {
  v.x = clamp(v.x, b, t);
  v.y = clamp(v.y, b, t);
  return v;
}

void vec2_print(vec2 v) {
  printf("vec2(%4.2f,%4.2f)", v.x, v.y);
}

float vec2_dot(vec2 v1, vec2 v2) {
  return (v1.x * v2.x) + (v1.y * v2.y);
}

float vec2_length_sqrd(vec2 v) {
  float length = 0.0;
  length += v.x * v.x;
  length += v.y * v.y;
  return length;
}

float vec2_length(vec2 v) {
  return sqrt(vec2_length_sqrd(v));
}

float vec2_dist_sqrd(vec2 v1, vec2 v2) {
  return (v1.x - v2.x) * (v1.x - v2.x) + 
         (v1.y - v2.y) * (v1.y - v2.y);
}

float vec2_dist(vec2 v1, vec2 v2) {
  return sqrt(vec2_dist_sqrd(v1, v2));
}

float vec2_dist_manhattan(vec2 v1, vec2 v2) {
  return fabs(v1.x - v2.x) + fabs(v1.y - v2.y);
}

vec2 vec2_normalize(vec2 v) {
  float len = vec2_length(v);
  return vec2_div(v, len);
}

vec2 vec2_reflect(vec2 v1, vec2 v2) {
  return vec2_sub(v1, vec2_mul(v2, 2 * vec2_dot(v1, v2)));
}

vec2 vec2_from_string(char* s) {

  char* pEnd;
  double d1, d2;
  d1 = strtod(s,&pEnd);
  d2 = strtod(pEnd,NULL);  

  vec2 v;
  v.x = d1;
  v.y = d2;
  
  return v;
}

bool vec2_equ(vec2 v1, vec2 v2) {
  if(!(v1.x == v2.x)) { return false; }
  if(!(v1.y == v2.y)) { return false; }
  return true;
}

void vec2_to_array(vec2 v, float* out) {
  out[0] = v.x;
  out[1] = v.y;
}

int vec2_hash(vec2 v) {
  return abs(rawcast(v.x) ^ rawcast(v.y));
}

int vec2_mix_hash(vec2 v) {

  int raw_vx = abs(rawcast(v.x));
  int raw_vy = abs(rawcast(v.y));

  int h1 = raw_vx << 1;
  int h2 = raw_vy << 3;
  int h3 = raw_vx >> 8;
  
  int h4 = raw_vy << 7;
  int h5 = raw_vx >> 12;
  int h6 = raw_vy >> 15;

  int h7 = raw_vx << 2;
  int h8 = raw_vy << 6;
  int h9 = raw_vx >> 2;
  
  int h10 = raw_vy << 9;
  int h11 = raw_vx >> 21;
  int h12 = raw_vy >> 13;
  
  int res1 = h1 ^ h2 ^ h3;
  int res2 = h4 ^ h5 ^ h6;
  int res3 = h7 ^ h8 ^ h9;
  int res4 = h10 ^ h11 ^ h12;
  
  return (res1 * 10252247) ^ (res2 * 70209673) ^ (res3 * 104711) ^ (res4 * 63589);
}

vec2 vec2_saturate(vec2 v) {
  v.x = saturate(v.x);
  v.y = saturate(v.y);
  return v;
}

vec2 vec2_lerp(vec2 v1, vec2 v2, float amount) {
  vec2 v;
  v.x = lerp(v1.x, v2.x, amount);
  v.y = lerp(v1.y, v2.y, amount);
  return v;
}

vec2 vec2_smoothstep(vec2 v1, vec2 v2, float amount) {
  float scaled_amount = amount*amount*(3 - 2*amount);
  return vec2_lerp( v1, v2, scaled_amount );
}

vec2 vec2_smootherstep(vec2 v1, vec2 v2, float amount) {
  float scaled_amount = amount*amount*amount*(amount*(amount*6 - 15) + 10);
  return vec2_lerp( v1, v2, scaled_amount );
}

/* vec3 */

vec3 vec3_new(float x, float y, float z) {
  vec3 v;
  v.x = x;
  v.y = y;
  v.z = z;
  return v;
}

vec3 vec3_zero() {
  return vec3_new(0, 0, 0);
}

vec3 vec3_one() {
  return vec3_new(1, 1, 1);
}

vec3 vec3_red() {
  return vec3_new(1, 0, 0);
}

vec3 vec3_green() {
  return vec3_new(0, 1, 0);
}

vec3 vec3_blue() {
  return vec3_new(0, 0, 1);
}

vec3 vec3_white() {
  return vec3_new(1, 1, 1);
}

vec3 vec3_black() {
  return vec3_new(0, 0, 0);
}

vec3 vec3_grey() {
  return vec3_new(0.5, 0.5, 0.5);
}

vec3 vec3_light_grey() {
  return vec3_new(0.75,0.75,0.75);
}

vec3 vec3_dark_grey() {
  return vec3_new(0.25,0.25,0.25);
}

vec3 vec3_up() {
  return vec3_new(0, 1, 0);
}

vec3 vec3_add(vec3 v1, vec3 v2) {
  vec3 v;
  v.x = v1.x + v2.x;
  v.y = v1.y + v2.y;
  v.z = v1.z + v2.z;
  return v;
}

vec3 vec3_sub(vec3 v1, vec3 v2) {
  vec3 v;
  v.x = v1.x - v2.x;
  v.y = v1.y - v2.y;
  v.z = v1.z - v2.z;
  return v;
}

vec3 vec3_div(vec3 v, float fac) {
  v.x = v.x / fac;
  v.y = v.y / fac;
  v.z = v.z / fac;
  return v;
}

vec3 vec3_div_vec3(vec3 v1, vec3 v2) {
  vec3 v;
  v.x = v1.x / v2.x;
  v.y = v1.y / v2.y;
  v.z = v1.z / v2.z;
  return v;
}

vec3 vec3_mul(vec3 v, float fac) {
  v.x = v.x * fac;
  v.y = v.y * fac;
  v.z = v.z * fac;
  return v;
}

vec3 vec3_mul_vec3(vec3 v1, vec3 v2) {
  vec3 v;
  v.x = v1.x * v2.x;
  v.y = v1.y * v2.y;
  v.z = v1.z * v2.z;
  return v;
}

vec3 vec3_pow(vec3 v, float exp) {
  v.x = pow(v.x, exp);
  v.y = pow(v.y, exp);
  v.z = pow(v.z, exp);
  return v;
}

vec3 vec3_neg(vec3 v) {
  v.x = -v.x;
  v.y = -v.y;
  v.z = -v.z;
  return v;
}

vec3 vec3_abs(vec3 v) {
  v.x = fabs(v.x);
  v.y = fabs(v.y);
  v.z = fabs(v.z);
  return v;
}

vec3 vec3_floor(vec3 v) {
  v.x = floor(v.x);
  v.y = floor(v.y);
  v.z = floor(v.z);
  return v;
}

vec3 vec3_fmod(vec3 v, float val) {
  v.x = fmod(v.x, val);
  v.y = fmod(v.y, val);
  v.z = fmod(v.z, val);
  return v;
}

void vec3_print(vec3 v) {
  printf("vec3(%4.2f,%4.2f,%4.2f)", v.x, v.y, v.z);
}

float vec3_dot(vec3 v1, vec3 v2) {
  return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

vec3 vec3_cross(vec3 v1, vec3 v2) {
  vec3 v;
  v.x = (v1.y * v2.z) - (v1.z * v2.y);
  v.y = (v1.z * v2.x) - (v1.x * v2.z);
  v.z = (v1.x * v2.y) - (v1.y * v2.x);
  return v;
}

float vec3_length_sqrd(vec3 v) {
  float length = 0.0;
  length += v.x * v.x;
  length += v.y * v.y;
  length += v.z * v.z;
  return length;
}

float vec3_length(vec3 v) {
  return sqrt(vec3_length_sqrd(v));
}

float vec3_dist_sqrd(vec3 v1, vec3 v2) {
  return (v1.x - v2.x) * (v1.x - v2.x) + 
         (v1.y - v2.y) * (v1.y - v2.y) + 
         (v1.z - v2.z) * (v1.z - v2.z);
}

float vec3_dist(vec3 v1, vec3 v2) {
  return sqrt(vec3_dist_sqrd(v1, v2));
}

float vec3_dist_manhattan(vec3 v1, vec3 v2) {
  return fabs(v1.x - v2.x) + fabs(v1.y - v2.y) + fabs(v1.z - v2.z);
}

vec3 vec3_normalize(vec3 v) {
  float len = vec3_length(v);
  if (len == 0.0) {
    return vec3_zero();
  } else {
    return vec3_div(v, len);
  }
}

vec3 vec3_reflect(vec3 v1, vec3 v2) {
  return vec3_sub(v1, vec3_mul(v2, 2 * vec3_dot(v1, v2)));
}

vec3 vec3_project(vec3 v1, vec3 v2) {
  return vec3_sub(v1, vec3_mul(v2, vec3_dot(v1, v2)));
}

vec3 vec3_from_string(char* s) {
  
  char* pEnd;
  double d1, d2, d3;
  d1 = strtod(s,&pEnd);
  d2 = strtod(pEnd,&pEnd);  
  d3 = strtod(pEnd,NULL);  

  vec3 v;
  v.x = d1;
  v.y = d2;
  v.z = d3;
  
  return v;

}

bool vec3_equ(vec3 v1, vec3 v2) {
  if (v1.x != v2.x) { return false; }
  if (v1.y != v2.y) { return false; }
  if (v1.z != v2.z) { return false; }
  return true;
}

bool vec3_neq(vec3 v1, vec3 v2) {
  if (v1.x != v2.x) { return true; }
  if (v1.y != v2.y) { return true; }
  if (v1.z != v2.z) { return true; }
  return false;
}

void vec3_to_array(vec3 v, float* out) {
  
  out[0] = v.x;
  out[1] = v.y;
  out[2] = v.z;
  
}

int vec3_hash(vec3 v) {
  return abs( rawcast(v.x) ^ rawcast(v.y) ^ rawcast(v.z) );
}

vec4 vec3_to_homogeneous(vec3 v){
  return vec4_new(v.x, v.y, v.z, 1.0);
};

vec3 vec3_saturate(vec3 v) {
  v.x = saturate(v.x);
  v.y = saturate(v.y);
  v.z = saturate(v.z);
  return v;
}

vec3 vec3_lerp(vec3 v1, vec3 v2, float amount) {
  vec3 v;
  v.x = lerp(v1.x, v2.x, amount);
  v.y = lerp(v1.y, v2.y, amount);
  v.z = lerp(v1.z, v2.z, amount);
  return v;
}

vec3 vec3_smoothstep(vec3 v1, vec3 v2, float amount) {
  float scaled_amount = amount*amount*(3 - 2*amount);
  return vec3_lerp( v1, v2, scaled_amount );
}

vec3 vec3_smootherstep(vec3 v1, vec3 v2, float amount) {
  float scaled_amount = amount*amount*amount*(amount*(amount*6 - 15) + 10);
  return vec3_lerp( v1, v2, scaled_amount );
}

/* vec4 */

vec4 vec4_new(float x, float y, float z, float w) {
  vec4 v;
  v.x = x;
  v.y = y;
  v.z = z;
  v.w = w;
  return v;
}

vec4 vec4_zero() {
  return vec4_new(0, 0, 0, 0);
}

vec4 vec4_one() {
  return vec4_new(1, 1, 1, 1);
}

vec4 vec4_red() {
  return vec4_new(1,0,0,1);
}

vec4 vec4_green() {
  return vec4_new(0,1,0,1);
}

vec4 vec4_blue() {
  return vec4_new(0,0,1,1);
}

vec4 vec4_white() {
  return vec4_new(1,1,1,1);
}

vec4 vec4_black() {
  return vec4_new(0,0,0,1);
}

vec4 vec4_grey() {
  return vec4_new(0.5,0.5,0.5, 1);
}

vec4 vec4_light_grey() {
  return vec4_new(0.75,0.75,0.75, 1);
}

vec4 vec4_dark_grey() {
  return vec4_new(0.25,0.25,0.25, 1);
}

vec4 vec4_add(vec4 v1, vec4 v2) {
  vec4 v;
  v.x = v1.x + v2.x;
  v.y = v1.y + v2.y;
  v.z = v1.z + v2.z;
  v.w = v1.w + v2.w;
  return v;
}

vec4 vec4_sub(vec4 v1, vec4 v2) {
  vec4 v;
  v.x = v1.x - v2.x;
  v.y = v1.y - v2.y;
  v.z = v1.z - v2.z;
  v.w = v1.w - v2.w;
  return v;
}

vec4 vec4_div(vec4 v, float fac) {
  v.x = v.x / fac;
  v.y = v.y / fac;
  v.z = v.z / fac;
  v.w = v.w / fac;
  return v;
}

vec4 vec4_mul(vec4 v, float fac) {
  v.x = v.x * fac;
  v.y = v.y * fac;
  v.z = v.z * fac;
  v.w = v.w * fac;
  return v;
}

vec4 vec4_mul_vec4(vec4 v1, vec4 v2) {
  vec4 v;
  v.x = v1.x * v2.x;
  v.y = v1.y * v2.y;
  v.z = v1.z * v2.z;
  v.w = v1.w * v2.w;
  return v;
}

vec4 vec4_pow(vec4 v, float exp) {
  v.x = pow(v.x, exp);
  v.y = pow(v.y, exp);
  v.z = pow(v.z, exp);
  v.w = pow(v.w, exp);
  return v;
}

vec4 vec4_neg(vec4 v) {
  v.x = -v.x;
  v.y = -v.y;
  v.z = -v.z;
  v.w = -v.w;
  return v;
}

vec4 vec4_abs(vec4 v) {
  v.x = fabs(v.x);
  v.y = fabs(v.y);
  v.z = fabs(v.z);
  v.w = fabs(v.w);
  return v;
}

vec4 vec4_floor(vec4 v) {
  v.x = floor(v.x);
  v.y = floor(v.y);
  v.z = floor(v.z);
  v.w = floor(v.w);
  return v;
}

vec4 vec4_fmod(vec4 v, float val) {
  v.x = fmod(v.x, val);
  v.y = fmod(v.y, val);
  v.z = fmod(v.z, val);
  v.w = fmod(v.w, val);
  return v;  
}

vec4 vec4_sqrt(vec4 v) {
  v.x = sqrt(v.x);
  v.y = sqrt(v.y);
  v.z = sqrt(v.z);
  v.w = sqrt(v.w);
  return v;  
}

void vec4_print(vec4 v) {
  printf("vec4(%4.2f, %4.2f, %4.2f, %4.2f)", v.x, v.y, v.z,  v.w);
}

float vec4_dot(vec4 v1, vec4 v2) {
  return  (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) + (v1.w * v2.w);
}

float vec4_length_sqrd(vec4 v) {
  float length = 0.0;
  length += v.x * v.x;
  length += v.y * v.y;
  length += v.z * v.z;
  length += v.w * v.w;
  return length;
}

float vec4_length(vec4 v) {
  return sqrt(vec4_length_sqrd(v));
}

float vec4_dist_sqrd(vec4 v1, vec4 v2) {
  return (v1.x - v2.x) * (v1.x - v2.x) + 
         (v1.y - v2.y) * (v1.y - v2.y) +
         (v1.z - v2.z) * (v1.z - v2.z) +
         (v1.w - v2.w) * (v1.w - v2.w);
}

float vec4_dist(vec4 v1, vec4 v2) {
  return sqrt(vec4_dist_sqrd(v1, v2));
}

float vec4_dist_manhattan(vec4 v1, vec4 v2) {
  return fabs(v1.x - v2.x) + fabs(v1.y - v2.y) + fabs(v1.z - v2.z) + fabs(v1.w - v2.w);
}

vec4 vec4_normalize(vec4 v) {
  float len = vec4_length(v);
  if (len == 0.0) {
    return vec4_zero();
  } else {
    return vec4_div(v, len);
  }
}

vec4 vec4_reflect(vec4 v1, vec4 v2) {
  return vec4_sub(v1, vec4_mul(v2, 2 * vec4_dot(v1, v2)));
}

vec4 vec4_from_string(char* s) {
  char* end;
  double d1, d2, d3, d4;
  d1 = strtod(s,&end);
  d2 = strtod(end,&end);  
  d3 = strtod(end,&end);  
  d4 = strtod(end,NULL);  

  vec4 v;
  v.x = d1;
  v.y = d2;
  v.z = d3;
  v.w = d4;
  
  return v;
}

vec4 vec4_max(vec4 v1, vec4 v2) {
  v1.x = max(v1.x, v2.x);
  v1.y = max(v1.y, v2.y);
  v1.z = max(v1.z, v2.z);
  v1.w = max(v1.w, v2.w);
  return v1;
}

vec4 vec4_min(vec4 v1, vec4 v2) {
  v1.x = min(v1.x, v2.x);
  v1.y = min(v1.y, v2.y);
  v1.z = min(v1.z, v2.z);
  v1.w = min(v1.w, v2.w);
  return v1;
}

bool vec4_equ(vec4 v1, vec4 v2) {
  if(!(v1.x == v2.x)) { return false; }
  if(!(v1.y == v2.y)) { return false; }
  if(!(v1.z == v2.z)) { return false; }
  if(!(v1.w == v2.w)) { return false; }
  return true;
}

void vec4_to_array(vec4 v, float* out) {
  out[0] = v.x;
  out[1] = v.y;
  out[2] = v.z;
  out[3] = v.w;
}

vec3 vec4_from_homogeneous(vec4 v) {
  vec3 vec = vec3_new(v.x,v.y,v.z);
  return vec3_div(vec, v.w);
};

int vec4_hash(vec4 v) {
  return abs( rawcast(v.x) ^ rawcast(v.y) ^ rawcast(v.z) ^ rawcast(v.w) );
}

vec4 vec4_saturate(vec4 v) {

  v.x = saturate(v.x);
  v.y = saturate(v.y);
  v.z = saturate(v.z);
  v.w = saturate(v.w);
  
  return v;
}

vec4 vec4_lerp(vec4 v1, vec4 v2, float amount) {
  vec4 v;
  v.x = lerp(v1.x, v2.x, amount);
  v.y = lerp(v1.y, v2.y, amount);
  v.z = lerp(v1.z, v2.z, amount);
  v.w = lerp(v1.w, v2.w, amount);
  return v;
}

vec4 vec4_smoothstep(vec4 v1, vec4 v2, float amount) {
  float scaled_amount = amount*amount*(3 - 2*amount);
  return vec4_lerp( v1, v2, scaled_amount );
}

vec4 vec4_smootherstep(vec4 v1, vec4 v2, float amount) {
  float scaled_amount = amount*amount*amount*(amount*(amount*6 - 15) + 10);
  return vec4_lerp( v1, v2, scaled_amount );
}

vec4 vec4_nearest_interp(vec4 v1, vec4 v2, float amount) {
  vec4 v;
  v.w = nearest_interp(v1.w, v2.w, amount);
  v.x = nearest_interp(v1.x, v2.x, amount);
  v.y = nearest_interp(v1.y, v2.y, amount);
  v.z = nearest_interp(v1.z, v2.z, amount);
  return v;
}

vec4 vec4_binearest_interp(vec4 tl, vec4 tr, vec4 bl, vec4 br, float x_amount, float y_amount) {

  vec4 v;
  v.x = binearest_interp( tl.x, tr.x, bl.x, br.x, x_amount, y_amount );
  v.y = binearest_interp( tl.y, tr.y, bl.y, br.y, x_amount, y_amount );
  v.z = binearest_interp( tl.z, tr.z, bl.z, br.z, x_amount, y_amount );
  v.w = binearest_interp( tl.w, tr.w, bl.w, br.w, x_amount, y_amount );
  
  return v;

}

vec4 vec4_bilinear_interp(vec4 tl, vec4 tr, vec4 bl, vec4 br, float x_amount, float y_amount) {

  vec4 v;
  v.x = bilinear_interp( tl.x, tr.x, bl.x, br.x, x_amount, y_amount );
  v.y = bilinear_interp( tl.y, tr.y, bl.y, br.y, x_amount, y_amount );
  v.z = bilinear_interp( tl.z, tr.z, bl.z, br.z, x_amount, y_amount );
  v.w = bilinear_interp( tl.w, tr.w, bl.w, br.w, x_amount, y_amount );
  
  return v;

}

quat quat_id() {
  return quat_new(0, 0, 0, 1);
}

quat quat_new(float x, float y, float z, float w) {
  quat q;
  q.x = x;
  q.y = y;
  q.z = z;
  q.w = w;
  return q;
}

float quat_at(quat q, int i) {
  
  float* values = (float*)(&q);
  return values[i];

}

float quat_real(quat q) {
  return q.w;
}

vec3 quat_imaginaries(quat q) {
  return vec3_new(q.x, q.y, q.z);
}

quat quat_from_euler(vec3 r) {
  
  float fc1 = cosf( r.z / 2.0f );
  float fc2 = cosf( r.x / 2.0f );
  float fc3 = cosf( r.y / 2.0f );

  float fs1 = sinf( r.z / 2.0f );
  float fs2 = sinf( r.x / 2.0f );
  float fs3 = sinf( r.y / 2.0f );

  return quat_new(
    fc1 * fc2 * fs3 - fs1 * fs2 * fc3,
    fc1 * fs2 * fc3 + fs1 * fc2 * fs3,
    fs1 * fc2 * fc3 - fc1 * fs2 * fs3,
    fc1 * fc2 * fc3 + fs1 * fs2 * fs3
  );
  
}

quat quat_angle_axis(float angle, vec3 axis) {

  float sine = sinf( angle / 2.0f );
  float cosine = cosf( angle / 2.0f );

  return quat_normalize(quat_new(
    axis.x * sine,
    axis.y * sine,
    axis.z * sine,
    cosine));
    
}

quat quat_rotation_x(float angle) {
  return quat_angle_axis(angle, vec3_new(1,0,0));
}

quat quat_rotation_y(float angle) {
  return quat_angle_axis(angle, vec3_new(0,1,0));
}

quat quat_rotation_z(float angle) {
  return quat_angle_axis(angle, vec3_new(0,0,1));
}

void quat_to_angle_axis(quat q, vec3* axis, float* angle) {

  *angle = 2.0f * acosf( q.w );

  float divisor = sinf( *angle / 2.0f );

  if( fabs( divisor ) < FLT_EPSILON ) {
  
    axis->x = 0.0f;
    axis->y = 1.0f;
    axis->z = 0.0f;
    
  } else {
    
    axis->x = q.x / divisor;
    axis->y = q.y / divisor;
    axis->z = q.z / divisor;
    *axis = vec3_normalize(*axis);
    
  }

}

vec3 quat_to_euler(quat q) {

  float sqrx = q.x * q.x;
  float sqry = q.y * q.y;
  float sqrz = q.z * q.z;
  float sqrw = q.w * q.w;
  
  return vec3_new(
    asinf( -2.0f * ( q.x * q.z - q.y * q.w ) ),
    atan2f( 2.0f * ( q.y * q.z + q.x * q.w ), (-sqrx - sqry + sqrz + sqrw) ),
    atan2f( 2.0f * ( q.x * q.y + q.z * q.w ), ( sqrx - sqry - sqrz + sqrw) ));
    
}

quat quat_mul_quat(quat q1, quat q2) {

  return quat_new(
    (q1.w * q2.x) + (q1.x * q2.w) + (q1.y * q2.z) - (q1.z * q2.y),
    (q1.w * q2.y) - (q1.x * q2.z) + (q1.y * q2.w) + (q1.z * q2.x),
    (q1.w * q2.z) + (q1.x * q2.y) - (q1.y * q2.x) + (q1.z * q2.w),
    (q1.w * q2.w) - (q1.x * q2.x) - (q1.y * q2.y) - (q1.z * q2.z));

}

vec3 quat_mul_vec3(quat q, vec3 v) {

  quat work = q;
  work = quat_mul_quat(work, quat_normalize(quat_new(v.x, v.y, v.z, 0.0)));
  work = quat_mul_quat(work, quat_inverse(q));

  vec3 res = vec3_new(work.x, work.y, work.z);
  
  return vec3_mul(res, vec3_length(v));

}

quat quat_inverse(quat q) {

  float	scale = quat_length(q);
  quat result = quat_unit_inverse(q);

  if ( scale > FLT_EPSILON ) {    
    result.x /= scale;
    result.y /= scale;
    result.z /= scale;
    result.w /= scale;
  }
  
  return result;
}

quat quat_unit_inverse(quat q) {
  return quat_new(-q.x, -q.y, -q.z, q.w);
}

float quat_length(quat q) {
  return sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
}

quat quat_normalize(quat q) {

  float scale = quat_length(q);

  if ( scale > FLT_EPSILON ) {
    return quat_new(
      q.x / scale,
      q.y / scale,
      q.z / scale,
      q.w / scale);
  } else {
    return quat_new(0,0,0,0);
  }
  
}

quat quat_slerp(quat from, quat to, float amount) {
  
  float scale0, scale1;
  float	afto1[4];
  float cosom = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;
  
  if ( cosom < 0.0f ) {
    cosom = -cosom; 
    afto1[0] = -to.x;
    afto1[1] = -to.y;
    afto1[2] = -to.z;
    afto1[3] = -to.w;
  } else {
    afto1[0] = to.x;
    afto1[1] = to.y;
    afto1[2] = to.z;
    afto1[3] = to.w;
  }
  
  const float QUATERNION_DELTA_COS_MIN = 0.01f;

  if ( (1.0f - cosom) > QUATERNION_DELTA_COS_MIN ) {
    /* This is a standard case (slerp). */
    float omega = acosf(cosom);
    float sinom = sinf(omega);
    scale0 = sinf((1.0f - amount) * omega) / sinom;
    scale1 = sinf(amount * omega) / sinom;
  } else {
    /* "from" and "to" quaternions are very close */
    /*  so we can do a linear interpolation.      */
    scale0 = 1.0f - amount;
    scale1 = amount;
  }

  return quat_new(
    (scale0 * from.x) + (scale1 * afto1[0]),
    (scale0 * from.y) + (scale1 * afto1[1]),
    (scale0 * from.z) + (scale1 * afto1[2]),
    (scale0 * from.w) + (scale1 * afto1[3]));

}

float quat_dot(quat q1, quat q2) {
  return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}

quat quat_exp(vec3 w) {

  float theta = sqrt(vec3_dot(w, w));
  float len = theta < FLT_EPSILON ? 1 : sin(theta) / theta;
  vec3 v = vec3_mul(w, len);
  
  return quat_new(v.x, v.y, v.z, cos(theta));
}

vec3 quat_log(quat q) {
  
  float len = vec3_length(quat_imaginaries(q));
  float angle = atan2(len, q.w);
  
  len = len > FLT_EPSILON ? angle / len : 1;
  
  return vec3_mul(quat_imaginaries(q), len);
  
}

static quat quat_get_value(float t, vec3 axis) {
	return quat_exp( vec3_mul(axis, t / 2.0) );
}

quat quat_constrain(quat q, vec3 axis) {

  const quat orient = quat_new(0, 0, 0, 1);
	
	vec3 vs = quat_imaginaries(q);
	vec3 v0 = quat_imaginaries(orient);

	float a = q.w * orient.w + vec3_dot(vs, v0);
	float b = orient.w * vec3_dot(axis, vs) - q.w * vec3_dot(axis, v0) + vec3_dot(vs, vec3_mul_vec3(axis, v0));

	float alpha = atan2(a, b);

	float t1 = -2 * alpha + M_PI;
	float t2 = -2 * alpha - M_PI;
  
	if ( quat_dot(q, quat_get_value(t1, axis)) > 
       quat_dot(q, quat_get_value(t2, axis)) ) {
		return quat_get_value(t1, axis);
	}

	return quat_get_value(t2, axis);
}

quat quat_constrain_y(quat q) {
  return quat_constrain(q, vec3_new(0, 1, 0)); 
}

float quat_distance(quat q0, quat q1) {
  quat comb = quat_mul_quat(quat_inverse(q0), q1);
  return sin(vec3_length(quat_log(comb)));
}

quat quat_neg(quat q) {
  q.x = -q.x;
  q.y = -q.y;
  q.z = -q.z;
  q.w = -q.w;
  return q;
}

quat quat_scale(quat q, float f) {
  q.x = q.x * f;
  q.y = q.y * f;
  q.z = q.z * f;
  q.w = q.w * f;
  return q;
}

quat quat_interpolate(quat* qs, float* ws, int count) {
  
	quat ref = quat_id();
  quat ref_inv = quat_inverse(ref);
	
  vec3 acc = vec3_zero();

	for (int i = 0; i < count; i++ ) {
    
    vec3 qlog0 = quat_log(quat_mul_quat(ref_inv, qs[i]));
    vec3 qlog1 = quat_log(quat_mul_quat(ref_inv, quat_neg(qs[i])));
    
		if (vec3_length(qlog0) < vec3_length(qlog1) ) {
			acc = vec3_add(acc, vec3_mul(qlog0, ws[i]));
		} else {
			acc = vec3_add(acc, vec3_mul(qlog1, ws[i]));
    }
  }
  
  quat res = quat_mul_quat(ref, quat_exp(acc));
  
	return quat_normalize(res);
  
}

quat_dual quat_dual_new(quat real, quat dual) {
  quat_dual qd;
  qd.real = real;
  qd.dual = dual;
  return qd;
}

quat_dual quat_dual_id() {
  return quat_dual_new(quat_id(), vec4_zero());
}

quat_dual quat_dual_transform(quat q, vec3 t) {
  quat_dual qd;
  qd.real = q;
  qd.dual = quat_new(
     0.5 * ( t.x * q.w + t.y * q.z - t.z * q.y),
     0.5 * (-t.x * q.z + t.y * q.w + t.z * q.x),
     0.5 * ( t.x * q.y - t.y * q.x + t.z * q.w),
    -0.5 * ( t.x * q.x + t.y * q.y + t.z * q.z)
  );
  return qd;
}

quat_dual quat_dual_mul(quat_dual q0, quat_dual q1) {
  return quat_dual_new(
    quat_mul_quat(q0.real, q1.real), 
    vec4_add(
      quat_mul_quat(q0.real, q1.dual), 
      quat_mul_quat(q0.dual, q1.real)));
}

quat_dual quat_dual_normalize(quat_dual q) {
  float l = quat_length(q.real);
  quat real = vec4_mul(q.real, 1.0 / l);
  quat dual = vec4_mul(q.dual, 1.0 / l);
  return quat_dual_new(real, vec4_sub(dual, vec4_mul(real, quat_dot(real, dual))));
}

vec3 quat_dual_mul_vec3(quat_dual q, vec3 v) {
  
  vec3 rvc = vec3_cross(quat_imaginaries(q.real), v);
  vec3 real = vec3_cross(quat_imaginaries(q.real), vec3_add(rvc, vec3_mul(v, q.real.w)));

  vec3 rdc = vec3_cross(quat_imaginaries(q.real), quat_imaginaries(q.dual));  
  vec3 rimg = vec3_mul(quat_imaginaries(q.real), q.dual.w);
  vec3 dimg = vec3_mul(quat_imaginaries(q.dual), q.real.w);
  
  vec3 dual = vec3_sub(rimg, vec3_add(dimg, rdc));
  
  return vec3_add(v, vec3_add(vec3_mul(real, 2), vec3_mul(dual, 2)));
}

vec3 quat_dual_mul_vec3_rot(quat_dual q, vec3 v) {
  
  vec3 rvc = vec3_cross(quat_imaginaries(q.real), v);
  vec3 real = vec3_cross(quat_imaginaries(q.real), vec3_add(rvc, vec3_mul(v, q.real.w)));
  
  return vec3_add(v, vec3_mul(real, 2.0));
}

/* Matrix Functions */

mat2 mat2_id() {
  mat2 mat;
  mat.xx = 1.0f; mat.xy = 0.0f;
  mat.yx = 0.0f; mat.yy = 1.0f;
  return mat;
}

mat2 mat2_zero() {
  mat2 mat;
  mat.xx = 0.0f; mat.xy = 0.0f;
  mat.yx = 0.0f; mat.yy = 0.0f;
  return mat;
}

mat2 mat2_new(float xx, float xy, float yx, float yy) {
  mat2 mat;
  mat.xx = xx;
  mat.xy = xy;
  mat.yx = yx;
  mat.yy = yy;
  return mat;
}

mat2 mat2_mul_mat2(mat2 m1, mat2 m2) {
  mat2 mat;
  
  mat.xx = m1.xx * m2.xx + m1.xy * m2.yx;
  mat.xy = m1.xx * m2.xy + m1.xy * m2.yy;
  mat.yx = m1.yx * m2.xx + m1.yy * m2.yx;
  mat.yy = m1.yx * m2.xy + m1.yy * m2.yy;
  
  return mat;
}

vec2 mat2_mul_vec2(mat2 m, vec2 v) {
  vec2 vec;
  
  vec.x = v.x * m.xx + v.y * m.xy;
  vec.y = v.x * m.yx + v.y * m.yy;
  
  return vec;
}

mat2 mat2_transpose(mat2 m) {
  mat2 ret;
  ret.xx = m.xx;
  ret.xy = m.yx;
  ret.yx = m.xy;
  ret.yy = m.yy;
  return ret;
}

float mat2_det(mat2 m) {
  return m.xx * m.yy - m.xy * m.yx;
}

mat2 mat2_inverse(mat2 m) {

  float det = mat2_det(m);
  float fac = 1.0 / det;
  
  mat2 ret;
  
  ret.xx = fac * m.yy;
  ret.xy = fac * -m.xy;
  ret.yx = fac * -m.yx;
  ret.yy = fac * m.xx;
  
  return ret;
}

void mat2_to_array(mat2 m, float* out) {
  
  out[0] = m.xx;
  out[1] = m.xy;
  out[2] = m.yx;
  out[3] = m.yy;
  
}

void mat2_print(mat2 m) {
  printf("|%4.2f, %4.2f|\n", m.xx, m.xy);
  printf("|%4.2f, %4.2f|\n", m.yx, m.yy);
}

mat2 mat2_rotation(float a) {
  mat2 m;
  
  m.xx = cos(a);
  m.xy = -sin(a);
  m.yx = sin(a);
  m.yy = cos(a);
  
  return m;
}

/* Matrix 3x3 */

mat3 mat3_zero() {
  mat3 mat;
  
  mat.xx = 0.0f;
  mat.xy = 0.0f;
  mat.xz = 0.0f;
  
  mat.yx = 0.0f;
  mat.yy = 0.0f;
  mat.yz = 0.0f;
  
  mat.zx = 0.0f;
  mat.zy = 0.0f;
  mat.zz = 0.0f;
  
  return mat;
}

mat3 mat3_id() {
  mat3 mat;
  
  mat.xx = 1.0f;
  mat.xy = 0.0f;
  mat.xz = 0.0f;
  
  mat.yx = 0.0f;
  mat.yy = 1.0f;
  mat.yz = 0.0f;
  
  mat.zx = 0.0f;
  mat.zy = 0.0f;
  mat.zz = 1.0f;
  
  return mat;
}

mat3 mat3_new(float xx, float xy, float xz,
              float yx, float yy, float yz,
              float zx, float zy, float zz) {
  mat3 mat;
  
  mat.xx = xx;
  mat.xy = xy;
  mat.xz = xz;
  
  mat.yx = yx;
  mat.yy = yy;
  mat.yz = yz;
  
  mat.zx = zx;
  mat.zy = zy;
  mat.zz = zz;
  
  return mat;
}

mat3 mat3_mul_mat3(mat3 m1, mat3 m2) {
  mat3 mat;

  mat.xx = (m1.xx * m2.xx) + (m1.xy * m2.yx) + (m1.xz * m2.zx);
  mat.xy = (m1.xx * m2.xy) + (m1.xy * m2.yy) + (m1.xz * m2.zy);
  mat.xz = (m1.xx * m2.xz) + (m1.xy * m2.yz) + (m1.xz * m2.zz);

  mat.yx = (m1.yx * m2.xx) + (m1.yy * m2.yx) + (m1.yz * m2.zx);
  mat.yy = (m1.yx * m2.xy) + (m1.yy * m2.yy) + (m1.yz * m2.zy);
  mat.yz = (m1.yx * m2.xz) + (m1.yy * m2.yz) + (m1.yz * m2.zz);

  mat.zx = (m1.zx * m2.xx) + (m1.zy * m2.yx) + (m1.zz * m2.zx);
  mat.zy = (m1.zx * m2.xy) + (m1.zy * m2.yy) + (m1.zz * m2.zy);
  mat.zz = (m1.zx * m2.xz) + (m1.zy * m2.yz) + (m1.zz * m2.zz);
  
  return mat;
  
}

vec3 mat3_mul_vec3(mat3 m, vec3 v) {

  vec3 vec;
  
  vec.x = (m.xx * v.x) + (m.xy * v.y) + (m.xz * v.z);
  vec.y = (m.yx * v.x) + (m.yy * v.y) + (m.yz * v.z);
  vec.z = (m.zx * v.x) + (m.zy * v.y) + (m.zz * v.z);
  
  return vec;

}

mat3 mat3_transpose(mat3 m) {
  mat3 ret;
  ret.xx = m.xx;
  ret.xy = m.yx;
  ret.xz = m.zx;
  
  ret.yx = m.xy;
  ret.yy = m.yy;
  ret.yz = m.zy;
  
  ret.zx = m.xz;
  ret.zy = m.yz;
  ret.zz = m.zz;
  return ret;
}

float mat3_det(mat3 m) {
  return (m.xx * m.yy * m.zz) + (m.xy * m.yz * m.zx) + (m.xz * m.yx * m.zy) -
         (m.xz * m.yy * m.zx) - (m.xy * m.yx * m.zz) - (m.xx * m.yz * m.zy);
}

mat3 mat3_inverse(mat3 m) {

  float det = mat3_det(m);
  float fac = 1.0 / det;
  
  mat3 ret;
  ret.xx = fac * mat2_det(mat2_new(m.yy, m.yz, m.zy, m.zz));
  ret.xy = fac * mat2_det(mat2_new(m.xz, m.xy, m.zz, m.zy));
  ret.xz = fac * mat2_det(mat2_new(m.xy, m.xz, m.yy, m.yz));
  
  ret.yx = fac * mat2_det(mat2_new(m.yz, m.yx, m.zz, m.zx));
  ret.yy = fac * mat2_det(mat2_new(m.xx, m.xz, m.zx, m.zz));
  ret.yz = fac * mat2_det(mat2_new(m.xz, m.xx, m.yz, m.yx));
  
  ret.zx = fac * mat2_det(mat2_new(m.yx, m.yy, m.zx, m.zy));
  ret.zy = fac * mat2_det(mat2_new(m.xy, m.xx, m.zy, m.zx));
  ret.zz = fac * mat2_det(mat2_new(m.xx, m.xy, m.yx, m.yy));
  
  return ret;
  
}

void mat3_to_array(mat3 m, float* out) {

  out[0] = m.xx;
  out[1] = m.yx;
  out[2] = m.zx;
  
  out[3] = m.xy;
  out[4] = m.yy;
  out[5] = m.zy;
  
  out[6] = m.xz;
  out[7] = m.yz;
  out[8] = m.zz;
  
}

void mat3_print(mat3 m) {
  printf("|%4.2f, %4.2f, %4.2f|\n", m.xx, m.xy, m.xz);
  printf("|%4.2f, %4.2f, %4.2f|\n", m.yx, m.yy, m.yz);
  printf("|%4.2f, %4.2f, %4.2f|\n", m.zx, m.zy, m.zz);
}

mat3 mat3_rotation_x(float a) {

  mat3 m = mat3_id();
  
  m.yy = cos(a);
  m.yz = -sin(a);
  m.zy = sin(a);
  m.zz = cos(a);
  
  return m;
  
}

mat3 mat3_scale(vec3 s) {
  
  mat3 m = mat3_id();
  m.xx = s.x;
  m.yy = s.y;
  m.zz = s.z;  
  return m;

}

mat3 mat3_rotation_y(float a) {

  mat3 m = mat3_id();
  
  m.xx = cos(a);
  m.xz = sin(a);
  m.zx = -sin(a);
  m.zz = cos(a);

  return m;
  
}

mat3 mat3_rotation_z(float a) {

  mat3 m = mat3_id();
  
  m.xx = cos(a);
  m.xy = -sin(a);
  m.yx = sin(a);
  m.yy = cos(a);

  return m;
  
}

mat3 mat3_rotation_angle_axis(float a, vec3 v) {
  
  mat3 m;

  float c = cos(a);
  float s = sin(a);
  float nc = 1 - c;
  
  m.xx = v.x * v.x * nc + c;
  m.xy = v.x * v.y * nc - v.z * s;
  m.xz = v.x * v.z * nc + v.y * s;
  
  m.yx = v.y * v.x * nc + v.z * s;
  m.yy = v.y * v.y * nc + c;
  m.yz = v.y * v.z * nc - v.x * s;
  
  m.zx = v.z * v.x * nc - v.y * s;
  m.zy = v.z * v.y * nc + v.x * s;
  m.zz = v.z * v.z * nc + c;
  
  return m;
}

/* Matrix 4x4 */

mat4 mat4_zero() {
  mat4 mat;
  
  mat.xx = 0.0f;
  mat.xy = 0.0f;
  mat.xz = 0.0f;
  mat.xw = 0.0f;
  
  mat.yx = 0.0f;
  mat.yy = 0.0f;
  mat.yz = 0.0f;
  mat.yw = 0.0f;
  
  mat.zx = 0.0f;
  mat.zy = 0.0f;
  mat.zz = 0.0f;
  mat.zw = 0.0f;
  
  mat.wx = 0.0f;
  mat.wy = 0.0f;
  mat.wz = 0.0f;
  mat.ww = 0.0f;
  
  return mat;
}

mat4 mat4_id(){
  
  mat4 mat = mat4_zero();
  
  mat.xx = 1.0f;
  mat.yy = 1.0f;
  mat.zz = 1.0f;
  mat.ww = 1.0f;
  
  
  return mat;
}

float mat4_at(mat4 m, int x, int y) {
  float* arr = (float*)(&m);
  return arr[x + (y*4)];  
}

mat4 mat4_set(mat4 m, int x, int y, float v) {
  
  float* arr = (float*)(&m);
  arr[x + (y*4)] = v;
  
  return m;
}

mat4 mat4_new(float xx, float xy, float xz, float xw,
              float yx, float yy, float yz, float yw,
              float zx, float zy, float zz, float zw,
              float wx, float wy, float wz, float ww) {
         
  mat4 mat;
  
  mat.xx = xx;
  mat.xy = xy;
  mat.xz = xz;
  mat.xw = xw;
  
  mat.yx = yx;
  mat.yy = yy;
  mat.yz = yz;
  mat.yw = yw;
  
  mat.zx = zx;
  mat.zy = zy;
  mat.zz = zz;
  mat.zw = zw;
  
  mat.wx = wx;
  mat.wy = wy;
  mat.wz = wz;
  mat.ww = ww;
  
  return mat;
}

mat4 mat4_transpose(mat4 m) {
  mat4 mat;
  
  mat.xx = m.xx;
  mat.xy = m.yx;
  mat.xz = m.zx;
  mat.xw = m.wx;
  
  mat.yx = m.xy;
  mat.yy = m.yy;
  mat.yz = m.zy;
  mat.yw = m.wy;
  
  mat.zx = m.xz;
  mat.zy = m.yz;
  mat.zz = m.zz;
  mat.zw = m.wz;
  
  mat.wx = m.xw;
  mat.wy = m.yw;
  mat.wz = m.zw;
  mat.ww = m.ww;
  
  return mat;
}

mat4 mat3_to_mat4(mat3 m) {

  mat4 mat;
  
  mat.xx = m.xx;
  mat.xy = m.xy;
  mat.xz = m.xz;
  mat.xw = 0.0f;
  
  mat.yx = m.yx;
  mat.yy = m.yy;
  mat.yz = m.yz;
  mat.yw = 0.0f;
  
  mat.zx = m.zx;
  mat.zy = m.zy;
  mat.zz = m.zz;
  mat.zw = 0.0f;
  
  mat.wx = 0.0f;
  mat.wy = 0.0f;
  mat.wz = 0.0f;
  mat.ww = 1.0f;
  
  return mat;
}

mat4 mat4_mul_mat4(mat4 m1, mat4 m2) {

  mat4 mat;

  mat.xx = (m1.xx * m2.xx) + (m1.xy * m2.yx) + (m1.xz * m2.zx) + (m1.xw * m2.wx);
  mat.xy = (m1.xx * m2.xy) + (m1.xy * m2.yy) + (m1.xz * m2.zy) + (m1.xw * m2.wy);
  mat.xz = (m1.xx * m2.xz) + (m1.xy * m2.yz) + (m1.xz * m2.zz) + (m1.xw * m2.wz);
  mat.xw = (m1.xx * m2.xw) + (m1.xy * m2.yw) + (m1.xz * m2.zw) + (m1.xw * m2.ww);
  
  mat.yx = (m1.yx * m2.xx) + (m1.yy * m2.yx) + (m1.yz * m2.zx) + (m1.yw * m2.wx);
  mat.yy = (m1.yx * m2.xy) + (m1.yy * m2.yy) + (m1.yz * m2.zy) + (m1.yw * m2.wy);
  mat.yz = (m1.yx * m2.xz) + (m1.yy * m2.yz) + (m1.yz * m2.zz) + (m1.yw * m2.wz);
  mat.yw = (m1.yx * m2.xw) + (m1.yy * m2.yw) + (m1.yz * m2.zw) + (m1.yw * m2.ww);
 
  mat.zx = (m1.zx * m2.xx) + (m1.zy * m2.yx) + (m1.zz * m2.zx) + (m1.zw * m2.wx);
  mat.zy = (m1.zx * m2.xy) + (m1.zy * m2.yy) + (m1.zz * m2.zy) + (m1.zw * m2.wy);
  mat.zz = (m1.zx * m2.xz) + (m1.zy * m2.yz) + (m1.zz * m2.zz) + (m1.zw * m2.wz);
  mat.zw = (m1.zx * m2.xw) + (m1.zy * m2.yw) + (m1.zz * m2.zw) + (m1.zw * m2.ww);
  
  mat.wx = (m1.wx * m2.xx) + (m1.wy * m2.yx) + (m1.wz * m2.zx) + (m1.ww * m2.wx);
  mat.wy = (m1.wx * m2.xy) + (m1.wy * m2.yy) + (m1.wz * m2.zy) + (m1.ww * m2.wy);
  mat.wz = (m1.wx * m2.xz) + (m1.wy * m2.yz) + (m1.wz * m2.zz) + (m1.ww * m2.wz);
  mat.ww = (m1.wx * m2.xw) + (m1.wy * m2.yw) + (m1.wz * m2.zw) + (m1.ww * m2.ww);
  
  return mat;
  
}

vec4 mat4_mul_vec4(mat4 m, vec4 v) {
  
  vec4 vec;
  
  vec.x = (m.xx * v.x) + (m.xy * v.y) + (m.xz * v.z) + (m.xw * v.w);
  vec.y = (m.yx * v.x) + (m.yy * v.y) + (m.yz * v.z) + (m.yw * v.w);
  vec.z = (m.zx * v.x) + (m.zy * v.y) + (m.zz * v.z) + (m.zw * v.w);
  vec.w = (m.wx * v.x) + (m.wy * v.y) + (m.wz * v.z) + (m.ww * v.w);
  
  return vec;
}

vec3 mat4_mul_vec3(mat4 m, vec3 v) {
  
  vec4 v_homo = vec4_new(v.x, v.y, v.z, 1);
  v_homo = mat4_mul_vec4(m, v_homo);
  
  v_homo = vec4_div(v_homo, v_homo.w);
  
  return vec3_new(v_homo.x, v_homo.y, v_homo.z);
}

mat3 mat4_to_mat3(mat4 m) {

  mat3 mat;
  
  mat.xx = m.xx;
  mat.xy = m.xy;
  mat.xz = m.xz;
  
  mat.yx = m.yx;
  mat.yy = m.yy;
  mat.yz = m.yz;
  
  mat.zx = m.zx;
  mat.zy = m.zy;
  mat.zz = m.zz;
  
  return mat;
  
}

quat mat4_to_quat(mat4 m) {

  float tr = m.xx + m.yy + m.zz;

  if (tr > 0.0f) {
    
    float s = sqrtf( tr + 1.0f );
    
    float w = s / 2.0f;
    float x = ( mat4_at(m, 1, 2) - mat4_at(m, 2, 1) ) * (0.5f / s);
    float y = ( mat4_at(m, 2, 0) - mat4_at(m, 0, 2) ) * (0.5f / s);
    float z = ( mat4_at(m, 0, 1) - mat4_at(m, 1, 0) ) * (0.5f / s);
    return quat_new(x, y, z, w);
    
  } else {
    
    int nxt[3] = {1, 2, 0};
    float q[4];
    int  i, j, k;
    
    i = 0;
    if ( mat4_at(m, 1, 1) > mat4_at(m, 0, 0) ) {	i = 1;	}
    if ( mat4_at(m, 2, 2) > mat4_at(m, i, i) ) {	i = 2;	}
    j = nxt[i];
    k = nxt[j];

    float s = sqrtf( (mat4_at(m, i, i) - (mat4_at(m, j, j) + mat4_at(m, k, k))) + 1.0f );

    q[i] = s * 0.5f;

    if ( s != 0.0f )	{	s = 0.5f / s;	}

    q[3] = ( mat4_at(m, j, k) - mat4_at(m, k, j) ) * s;
    q[j] = ( mat4_at(m, i, j) + mat4_at(m, j, i) ) * s;
    q[k] = ( mat4_at(m, i, k) + mat4_at(m, k, i) ) * s;

    return quat_new(q[0], q[1], q[2], q[3]);
  }

}

quat_dual mat4_to_quat_dual(mat4 m) {
  quat rotation = mat4_to_quat(m);
  vec3 translation = mat4_mul_vec3(m, vec3_zero());
  return quat_dual_transform(rotation, translation);
}

float mat4_det(mat4 m) {
  
  float cofact_xx =  mat3_det(mat3_new(m.yy, m.yz, m.yw, m.zy, m.zz, m.zw, m.wy, m.wz, m.ww));
  float cofact_xy = -mat3_det(mat3_new(m.yx, m.yz, m.yw, m.zx, m.zz, m.zw, m.wx, m.wz, m.ww));
  float cofact_xz =  mat3_det(mat3_new(m.yx, m.yy, m.yw, m.zx, m.zy, m.zw, m.wx, m.wy, m.ww));
  float cofact_xw = -mat3_det(mat3_new(m.yx, m.yy, m.yz, m.zx, m.zy, m.zz, m.wx, m.wy, m.wz));
  
  return (cofact_xx * m.xx) + (cofact_xy * m.xy) + (cofact_xz * m.xz) + (cofact_xw * m.xw);
}

mat4 mat4_inverse(mat4 m) {
    
  float det = mat4_det(m);
  float fac = 1.0 / det;
  
  mat4 ret;
  ret.xx = fac *  mat3_det(mat3_new(m.yy, m.yz, m.yw, m.zy, m.zz, m.zw, m.wy, m.wz, m.ww));
  ret.xy = fac * -mat3_det(mat3_new(m.yx, m.yz, m.yw, m.zx, m.zz, m.zw, m.wx, m.wz, m.ww));
  ret.xz = fac *  mat3_det(mat3_new(m.yx, m.yy, m.yw, m.zx, m.zy, m.zw, m.wx, m.wy, m.ww));
  ret.xw = fac * -mat3_det(mat3_new(m.yx, m.yy, m.yz, m.zx, m.zy, m.zz, m.wx, m.wy, m.wz));
  
  ret.yx = fac * -mat3_det(mat3_new(m.xy, m.xz, m.xw, m.zy, m.zz, m.zw, m.wy, m.wz, m.ww));
  ret.yy = fac *  mat3_det(mat3_new(m.xx, m.xz, m.xw, m.zx, m.zz, m.zw, m.wx, m.wz, m.ww));
  ret.yz = fac * -mat3_det(mat3_new(m.xx, m.xy, m.xw, m.zx, m.zy, m.zw, m.wx, m.wy, m.ww));
  ret.yw = fac *  mat3_det(mat3_new(m.xx, m.xy, m.xz, m.zx, m.zy, m.zz, m.wx, m.wy, m.wz));
  
  ret.zx = fac *  mat3_det(mat3_new(m.xy, m.xz, m.xw, m.yy, m.yz, m.yw, m.wy, m.wz, m.ww));
  ret.zy = fac * -mat3_det(mat3_new(m.xx, m.xz, m.xw, m.yx, m.yz, m.yw, m.wx, m.wz, m.ww));
  ret.zz = fac *  mat3_det(mat3_new(m.xx, m.xy, m.xw, m.yx, m.yy, m.yw, m.wx, m.wy, m.ww));
  ret.zw = fac * -mat3_det(mat3_new(m.xx, m.xy, m.xz, m.yx, m.yy, m.yz, m.wx, m.wy, m.wz));
  
  ret.wx = fac * -mat3_det(mat3_new(m.xy, m.xz, m.xw, m.yy, m.yz, m.yw, m.zy, m.zz, m.zw));
  ret.wy = fac *  mat3_det(mat3_new(m.xx, m.xz, m.xw, m.yx, m.yz, m.yw, m.zx, m.zz, m.zw));
  ret.wz = fac * -mat3_det(mat3_new(m.xx, m.xy, m.xw, m.yx, m.yy, m.yw, m.zx, m.zy, m.zw));
  ret.ww = fac *  mat3_det(mat3_new(m.xx, m.xy, m.xz, m.yx, m.yy, m.yz, m.zx, m.zy, m.zz));
  
  ret = mat4_transpose(ret);
  
  return ret;
}

void mat4_to_array(mat4 m, float* out) {
  
  out[0] = m.xx;
  out[1] = m.yx;
  out[2] = m.zx;
  out[3] = m.wx;

  out[4] = m.xy;
  out[5] = m.yy;
  out[6] = m.zy;
  out[7] = m.wy;
  
  out[8] = m.xz;
  out[9] = m.yz;
  out[10] = m.zz;
  out[11] = m.wz;
  
  out[12] = m.xw;
  out[13] = m.yw;
  out[14] = m.zw;
  out[15] = m.ww;
  
}

void mat4_to_array_trans(mat4 m, float* out) {
  
  out[0] = m.xx;
  out[1] = m.xy;
  out[2] = m.xz;
  out[3] = m.xw;

  out[4] = m.yx;
  out[5] = m.yy;
  out[6] = m.yz;
  out[7] = m.yw;
  
  out[8] = m.zx;
  out[9] = m.zy;
  out[10] = m.zz;
  out[11] = m.zw;
  
  out[12] = m.wx;
  out[13] = m.wy;
  out[14] = m.wz;
  out[15] = m.ww;
  
}

void mat4_print(mat4 m) {

  printf("|%4.2f, %4.2f, %4.2f, %4.2f|\n", m.xx, m.xy, m.xz, m.xw);
  printf("|%4.2f, %4.2f, %4.2f, %4.2f|\n", m.yx, m.yy, m.yz, m.yw);
  printf("|%4.2f, %4.2f, %4.2f, %4.2f|\n", m.zx, m.zy, m.zz, m.zw);
  printf("|%4.2f, %4.2f, %4.2f, %4.2f|\n", m.wx, m.wy, m.wz, m.ww);
  
}

mat4 mat4_view_look_at(vec3 position, vec3 target, vec3 up) {
  
  vec3 zaxis = vec3_normalize( vec3_sub(target, position) );
  vec3 xaxis = vec3_normalize( vec3_cross(up, zaxis) );
  vec3 yaxis = vec3_cross(zaxis, xaxis);

  mat4 view_matrix = mat4_id();
  view_matrix.xx = xaxis.x;
  view_matrix.xy = xaxis.y;
  view_matrix.xz = xaxis.z;
  
  view_matrix.yx = yaxis.x;
  view_matrix.yy = yaxis.y;
  view_matrix.yz = yaxis.z;
  
  view_matrix.zx = -zaxis.x;
  view_matrix.zy = -zaxis.y;
  view_matrix.zz = -zaxis.z;
  
  view_matrix = mat4_mul_mat4(view_matrix, mat4_translation(vec3_neg(position)) );
  
  return view_matrix;
}

mat4 mat4_perspective(float fov, float near_clip, float far_clip, float ratio) {
  
  float right, left, bottom, top;
  
  right = -(near_clip * tanf(fov));
  left = -right;
  
  top = ratio * near_clip * tanf(fov);
  bottom = -top;
  
  mat4 proj_matrix = mat4_zero();
  proj_matrix.xx = (2.0 * near_clip) / (right - left);
  proj_matrix.yy = (2.0 * near_clip) / (top - bottom);
  proj_matrix.xz = (right + left) / (right - left);
  proj_matrix.yz = (top + bottom) / (top - bottom);
  proj_matrix.zz = (-far_clip - near_clip) / (far_clip - near_clip);
  proj_matrix.wz = -1.0;
  proj_matrix.zw = ( -(2.0 * near_clip) * far_clip) / (far_clip - near_clip);
  
  return proj_matrix;
}

mat4 mat4_orthographic(float left, float right, float bottom, float top, float clip_near, float clip_far) {

  mat4 m = mat4_id();
  
  m.xx = 2 / (right - left);
  m.yy = 2 / (top - bottom);
  m.zz = 1 / (clip_near - clip_far);
  
  m.xw = -1 - 2 * left / (right - left);
  m.yw =  1 + 2 * top  / (bottom - top);
  m.zw = clip_near / (clip_near - clip_far);
  
  return m;

}

mat4 mat4_translation(vec3 v) {

  mat4 m = mat4_id();
  m.xw = v.x;
  m.yw = v.y;
  m.zw = v.z;

  return m;
  
}

mat4 mat4_scale(vec3 v) {

  mat4 m = mat4_id();
  m.xx = v.x;
  m.yy = v.y;
  m.zz = v.z;

  return m;
}

mat4 mat4_rotation_x(float a) {

  mat4 m = mat4_id();
  
  m.yy = cos(a);
  m.yz = -sin(a);
  m.zy = sin(a);
  m.zz = cos(a);
  
  return m;
  
}

mat4 mat4_rotation_y(float a) {

  mat4 m = mat4_id();
  
  m.xx = cos(a);
  m.xz = sin(a);
  m.zx = -sin(a);
  m.zz = cos(a);

  return m;
  
}

mat4 mat4_rotation_z(float a) {

  mat4 m = mat4_id();
  
  m.xx = cos(a);
  m.xy = -sin(a);
  m.yx = sin(a);
  m.yy = cos(a);

  return m;
  
}

mat4 mat4_rotation_axis_angle(vec3 v, float angle) {

  mat4 m = mat4_id();

  float c = cos(angle);
  float s = sin(angle);
  float nc = 1 - c;
  
  m.xx = v.x * v.x * nc + c;
  m.xy = v.x * v.y * nc - v.z * s;
  m.xz = v.x * v.z * nc + v.y * s;
  
  m.yx = v.y * v.x * nc + v.z * s;
  m.yy = v.y * v.y * nc + c;
  m.yz = v.y * v.z * nc - v.x * s;
  
  m.zx = v.z * v.x * nc - v.y * s;
  m.zy = v.z * v.y * nc + v.x * s;
  m.zz = v.z * v.z * nc + c;
  
  return m;

}

mat4 mat4_rotation_euler(float x, float y, float z) {

  mat4 m = mat4_zero();

  float cosx = cos(x);
  float cosy = cos(y);
  float cosz = cos(z);
  float sinx = sin(x);
  float siny = sin(y);
  float sinz = sin(z);

  m.xx = cosy * cosz;
  m.yx = -cosx * sinz + sinx * siny * cosz;
  m.zx = sinx * sinz + cosx * siny * cosz;

  m.xy = cosy * sinz;
  m.yy = cosx * cosz + sinx * siny * sinz;
  m.zy = -sinx * cosz + cosx * siny * sinz;

  m.xz = -siny;
  m.yz = sinx * cosy;
  m.zz = cosx * cosy;

  m.ww = 1;
  
  return m;
}

mat4 mat4_rotation_quat(vec4 q) {

  float x2 = q.x + q.x; 
  float y2 = q.y + q.y; 
  float z2 = q.z + q.z;
  float xx = q.x * x2;  
  float yy = q.y * y2;  
  float wx = q.w * x2;  
  float xy = q.x * y2;   
  float yz = q.y * z2;   
  float wy = q.w * y2;
  float xz = q.x * z2;
  float zz = q.z * z2;  
  float wz = q.w * z2;  
  
  return mat4_new(
    1.0f - ( yy + zz ),	xy - wz, xz + wy,	0.0f,
    xy + wz, 1.0f - ( xx + zz ), yz - wx, 0.0f,
    xz - wy, yz + wx, 1.0f - ( xx + yy ), 0.0f,
    0.0f,	0.0f, 0.0f,	1.0f);
    
}

mat4 mat4_rotation_quat_dual(quat_dual q) {
  
  float rx = q.real.x, ry = q.real.y, rz = q.real.z, rw = q.real.w;
  float tx = q.dual.x, ty = q.dual.y, tz = q.dual.z, tw = q.dual.w;

  mat4 m = mat4_id();
  m.xx = rw*rw + rx*rx - ry*ry - rz*rz;              
  m.xy = 2.f*(rx*ry - rw*rz);                        
  m.xz = 2*(rx*rz + rw*ry);
  m.yx = 2*(rx*ry + rw*rz);                                  
  m.yy = rw*rw - rx*rx + ry*ry - rz*rz;      
  m.yz = 2*(ry*rz - rw*rx);
  m.zx = 2*(rx*rz - rw*ry);                                  
  m.zy = 2*(ry*rz + rw*rx);                          
  m.zz = rw*rw - rx*rx - ry*ry + rz*rz;

  m.xw = -2*tw*rx + 2*rw*tx - 2*ty*rz + 2*ry*tz;
  m.yw = -2*tw*ry + 2*tx*rz - 2*rx*tz + 2*rw*ty;
  m.zw = -2*tw*rz + 2*rx*ty + 2*rw*tz - 2*tx*ry;

  return m;
}

mat4 mat4_world(vec3 position, vec3 scale, quat rotation) {
  
  mat4 pos_m, sca_m, rot_m, result;
  
  pos_m = mat4_translation(position);
  rot_m = mat4_rotation_quat(rotation);
  sca_m = mat4_scale(scale);
  
  result = mat4_id();
  result = mat4_mul_mat4( result, pos_m );
  result = mat4_mul_mat4( result, rot_m );
  result = mat4_mul_mat4( result, sca_m );
  
  return result;
  
}

mat4 mat4_lerp(mat4 m1, mat4 m2, float amount) {
  mat4 m;
  
  m.xx = lerp(m1.xx, m2.xx, amount);
  m.xy = lerp(m1.xy, m2.xy, amount);
  m.xz = lerp(m1.xz, m2.xz, amount);
  m.xw = lerp(m1.xw, m2.xw, amount);
  
  m.yx = lerp(m1.yx, m2.yx, amount);
  m.yy = lerp(m1.yy, m2.yy, amount);
  m.yz = lerp(m1.yz, m2.yz, amount);
  m.yw = lerp(m1.yw, m2.yw, amount);
  
  m.zx = lerp(m1.zx, m2.zx, amount);
  m.zy = lerp(m1.zy, m2.zy, amount);
  m.zz = lerp(m1.zz, m2.zz, amount);
  m.zw = lerp(m1.zw, m2.zw, amount);
  
  m.wx = lerp(m1.wx, m2.wx, amount);
  m.wy = lerp(m1.wy, m2.wy, amount);
  m.wz = lerp(m1.wz, m2.wz, amount);
  m.ww = lerp(m1.ww, m2.ww, amount);
  
  return m;
}

mat4 mat4_smoothstep(mat4 m1, mat4 m2, float amount) {
  mat4 m;
  
  m.xx = smoothstep(m1.xx, m2.xx, amount);
  m.xy = smoothstep(m1.xy, m2.xy, amount);
  m.xz = smoothstep(m1.xz, m2.xz, amount);
  m.xw = smoothstep(m1.xw, m2.xw, amount);
  
  m.yx = smoothstep(m1.yx, m2.yx, amount);
  m.yy = smoothstep(m1.yy, m2.yy, amount);
  m.yz = smoothstep(m1.yz, m2.yz, amount);
  m.yw = smoothstep(m1.yw, m2.yw, amount);
  
  m.zx = smoothstep(m1.zx, m2.zx, amount);
  m.zy = smoothstep(m1.zy, m2.zy, amount);
  m.zz = smoothstep(m1.zz, m2.zz, amount);
  m.zw = smoothstep(m1.zw, m2.zw, amount);
  
  m.wx = smoothstep(m1.wx, m2.wx, amount);
  m.wy = smoothstep(m1.wy, m2.wy, amount);
  m.wz = smoothstep(m1.wz, m2.wz, amount);
  m.ww = smoothstep(m1.ww, m2.ww, amount);
  
  return m;
}

/* Geometry Functions */

plane plane_new(vec3 position, vec3 direction) {
  plane p;
  p.position = position;
  p.direction = direction;
  return p;
}

float plane_distance(plane p, vec3 point) {
  return vec3_dot(vec3_sub(point, p.position), p.direction);
}

plane plane_transform(plane p, mat4 world, mat3 world_normal) {
  p.position  = mat4_mul_vec3(world, p.position);
  p.direction = mat3_mul_vec3(world_normal, p.direction);
  p.direction = vec3_normalize(p.direction);
  return p;
}

plane plane_transform_space(plane p, mat3 space, mat3 space_normal) {
  p.position  = mat3_mul_vec3(space, p.position);
  p.direction = mat3_mul_vec3(space_normal, p.direction);
  p.direction = vec3_normalize(p.direction);
  return p;
}

bool point_inside_plane(vec3 point, plane p) {
  return vec3_dot(vec3_sub(point, p.position), p.direction) < 0;
}

bool point_outside_plane(vec3 point, plane p) {
  return vec3_dot(vec3_sub(point, p.position), p.direction) > 0;
}

bool point_intersects_plane(vec3 point, plane p) {
  return vec3_dot(vec3_sub(point, p.position), p.direction) == 0;
}

vec3 plane_project(plane p, vec3 v) {
  return vec3_sub(v, vec3_mul(p.direction, vec3_dot(v, p.direction)));
}

vec3 plane_closest(plane p, vec3 v) {
  return vec3_sub(v, vec3_mul(p.direction, plane_distance(p, v)));
}

bool point_swept_inside_plane(vec3 point, vec3 v, plane p) {

  float angle = vec3_dot(p.direction, v);
  float dist  = vec3_dot(p.direction, vec3_sub(point, p.position)); 
  
  if ( -dist <= 0.0 ) { return false; }
  
  return !between_or(-dist / angle, 0, 1);

}

bool point_swept_outside_plane(vec3 point, vec3 v, plane p) {

  float angle = vec3_dot(p.direction, v);
  float dist  = vec3_dot(p.direction, vec3_sub(point, p.position)); 
  
  if ( dist <= 0.0 ) { return false; }
  
  return !between_or(-dist / angle, 0, 1);

}

bool point_swept_intersects_plane(vec3 point, vec3 v, plane p) {

  float angle = vec3_dot(p.direction, v);
  float dist  = vec3_dot(p.direction, vec3_sub(point, p.position)); 
  
  if ( dist == 0.0 ) { return true; }
  
  return between_or(-dist / angle, 0, 1);

}

box box_new(float x_min, float x_max, float y_min, float y_max, float z_min, float z_max) {

  box bb;
  bb.top    = plane_new( vec3_new(0, y_max,0), vec3_new(0, 1,0));
  bb.bottom = plane_new( vec3_new(0, y_min,0), vec3_new(0,-1,0));
  bb.left   = plane_new( vec3_new( x_max,0,0), vec3_new( 1,0,0));
  bb.right  = plane_new( vec3_new( x_min,0,0), vec3_new(-1,0,0));
  bb.front  = plane_new( vec3_new(0,0, z_max), vec3_new(0,0, 1));
  bb.back   = plane_new( vec3_new(0,0, z_min), vec3_new(0,0,-1));
  return bb;

}

box box_sphere(vec3 center, float radius) {
  
  box bb;
  bb.top    = plane_new(vec3_add(center, vec3_new(0, radius,0)), vec3_new(0, 1,0));
  bb.bottom = plane_new(vec3_add(center, vec3_new(0,-radius,0)), vec3_new(0,-1,0));
  bb.left   = plane_new(vec3_add(center, vec3_new( radius,0,0)), vec3_new( 1,0,0));
  bb.right  = plane_new(vec3_add(center, vec3_new(-radius,0,0)), vec3_new(-1,0,0));
  bb.front  = plane_new(vec3_add(center, vec3_new(0,0, radius)), vec3_new(0,0, 1));
  bb.back   = plane_new(vec3_add(center, vec3_new(0,0,-radius)), vec3_new(0,0,-1));
  return bb;
  
}

box box_invert(box b) {
  b.front.direction  = vec3_neg(b.front.direction);
  b.back.direction   = vec3_neg(b.back.direction);
  b.left.direction   = vec3_neg(b.left.direction);
  b.right.direction  = vec3_neg(b.right.direction);
  b.top.direction    = vec3_neg(b.top.direction);
  b.bottom.direction = vec3_neg(b.bottom.direction);
  return b;
}

box box_invert_depth(box b) {
  b.front.direction = vec3_neg(b.front.direction);
  b.back.direction = vec3_neg(b.back.direction);
  return b;
}

box box_invert_width(box b) {
  b.left.direction = vec3_neg(b.left.direction);
  b.right.direction = vec3_neg(b.right.direction);
  return b;
}

box box_invert_height(box b) {
  b.top.direction = vec3_neg(b.top.direction);
  b.bottom.direction = vec3_neg(b.bottom.direction);
  return b;
}

bool point_inside_box(vec3 point, box b) {
  
  if (!point_inside_plane(point, b.top))    { return false; }
  if (!point_inside_plane(point, b.bottom)) { return false; }
  if (!point_inside_plane(point, b.left))   { return false; }
  if (!point_inside_plane(point, b.right))  { return false; }
  if (!point_inside_plane(point, b.front))  { return false; }
  if (!point_inside_plane(point, b.back))   { return false; }
  
  return true;
  
}

bool point_outside_box(vec3 point, box b) {
  return !(point_intersects_box(point, b) || point_inside_box(point, b));
}

bool point_intersects_box(vec3 point, box b) {
  
  if (point_intersects_plane(point, b.top)) { return true; }
  if (point_intersects_plane(point, b.bottom)) { return true; }
  if (point_intersects_plane(point, b.left)) { return true; }
  if (point_intersects_plane(point, b.right)) { return true; }
  if (point_intersects_plane(point, b.front)) { return true; }
  if (point_intersects_plane(point, b.back)) { return true; }
  
  return false;
  
}

box box_merge(box b1, box b2) {
  
  float b1_x_max = b1.left.position.x;
  float b1_x_min = b1.right.position.x;
  float b1_y_max = b1.top.position.y;
  float b1_y_min = b1.bottom.position.y;
  float b1_z_max = b1.front.position.z;
  float b1_z_min = b1.back.position.z;
  
  float b2_x_max = b2.left.position.x;
  float b2_x_min = b2.right.position.x;
  float b2_y_max = b2.top.position.y;
  float b2_y_min = b2.bottom.position.y;
  float b2_z_max = b2.front.position.z;
  float b2_z_min = b2.back.position.z;
  
  float x_min = min(b1_x_min, b2_x_min);
  float x_max = max(b1_x_max, b2_x_max);
  float y_min = min(b1_y_min, b2_y_min);
  float y_max = max(b1_y_max, b2_y_max);
  float z_min = min(b1_z_min, b2_z_min);
  float z_max = max(b1_z_max, b2_z_max);
  
  return box_new(x_min, x_max, y_min, y_max, z_min, z_max);
}

box box_transform(box bb, mat4 world, mat3 world_normal) {
  
  bb.top    = plane_transform(bb.top,    world, world_normal);
  bb.bottom = plane_transform(bb.bottom, world, world_normal);
  bb.left   = plane_transform(bb.left,   world, world_normal);
  bb.right  = plane_transform(bb.right,  world, world_normal);
  bb.front  = plane_transform(bb.front,  world, world_normal);
  bb.back   = plane_transform(bb.back,   world, world_normal);
  
  return bb;
  
}

frustum frustum_new(vec3 ntr, vec3 ntl, vec3 nbr, vec3 nbl, vec3 ftr, vec3 ftl, vec3 fbr, vec3 fbl) {
  frustum f;
  f.ntr = ntr;
  f.ntl = ntl;
  f.nbr = nbr;
  f.nbl = nbl;
  f.ftr = ftr;
  f.ftl = ftl;
  f.fbr = fbr;
  f.fbl = fbl;
  return f;
}

frustum frustum_new_clipbox() {
  return frustum_new(
    vec3_new( 1, 1,-1), vec3_new(-1, 1,-1), 
    vec3_new( 1,-1,-1), vec3_new(-1,-1,-1),
    vec3_new( 1, 1, 1), vec3_new(-1, 1, 1),
    vec3_new( 1,-1, 1), vec3_new(-1,-1, 1)); 
}

frustum frustum_new_camera(mat4 view, mat4 proj) {

  frustum f = frustum_new_clipbox();
  f = frustum_transform(f, mat4_inverse(proj));
  f = frustum_transform(f, mat4_inverse(view));
  return f;
  
}

frustum frustum_slice(frustum f, float start, float end) {
  frustum r;
  r.ntr = vec3_add(f.ntr, vec3_mul(vec3_sub(f.ftr, f.ntr), start));
  r.ftr = vec3_add(f.ntr, vec3_mul(vec3_sub(f.ftr, f.ntr), end  ));

  r.ntl = vec3_add(f.ntl, vec3_mul(vec3_sub(f.ftl, f.ntl), start));
  r.ftl = vec3_add(f.ntl, vec3_mul(vec3_sub(f.ftl, f.ntl), end  ));

  r.nbr = vec3_add(f.nbr, vec3_mul(vec3_sub(f.fbr, f.nbr), start));
  r.fbr = vec3_add(f.nbr, vec3_mul(vec3_sub(f.fbr, f.nbr), end  ));

  r.nbl = vec3_add(f.nbl, vec3_mul(vec3_sub(f.fbl, f.nbl), start));
  r.fbl = vec3_add(f.nbl, vec3_mul(vec3_sub(f.fbl, f.nbl), end  ));
  return r;
}

vec3 frustum_center(frustum f) {
  vec3 total = vec3_zero();
  total = vec3_add(total, f.ntr);
  total = vec3_add(total, f.ftr);
  total = vec3_add(total, f.ntl);
  total = vec3_add(total, f.ftl);
  total = vec3_add(total, f.nbr);
  total = vec3_add(total, f.fbr);
  total = vec3_add(total, f.nbl);
  total = vec3_add(total, f.fbl);
  return vec3_div(total, 8);
}

vec3 frustum_maximums(frustum f) {
  vec3 r;
  r.x = max(max(max(max(max(max(max(f.ntr.x, f.ftr.x), f.ntl.x), f.ftl.x), f.nbr.x), f.fbr.x), f.nbl.x), f.fbl.x);
  r.y = max(max(max(max(max(max(max(f.ntr.y, f.ftr.y), f.ntl.y), f.ftl.y), f.nbr.y), f.fbr.y), f.nbl.y), f.fbl.y);
  r.z = max(max(max(max(max(max(max(f.ntr.z, f.ftr.z), f.ntl.z), f.ftl.z), f.nbr.z), f.fbr.z), f.nbl.z), f.fbl.z);
  return r;
}

vec3 frustum_minimums(frustum f) {
  vec3 r;
  r.x = min(min(min(min(min(min(min(f.ntr.x, f.ftr.x), f.ntl.x), f.ftl.x), f.nbr.x), f.fbr.x), f.nbl.x), f.fbl.x);
  r.y = min(min(min(min(min(min(min(f.ntr.y, f.ftr.y), f.ntl.y), f.ftl.y), f.nbr.y), f.fbr.y), f.nbl.y), f.fbl.y);
  r.z = min(min(min(min(min(min(min(f.ntr.z, f.ftr.z), f.ntl.z), f.ftl.z), f.nbr.z), f.fbr.z), f.nbl.z), f.fbl.z);
  return r;
}

frustum frustum_transform(frustum f, mat4 m) {
  frustum r;
  r.ntr = mat4_mul_vec3(m, f.ntr);
  r.ftr = mat4_mul_vec3(m, f.ftr);
  r.ntl = mat4_mul_vec3(m, f.ntl);
  r.ftl = mat4_mul_vec3(m, f.ftl);
  r.nbr = mat4_mul_vec3(m, f.nbr);
  r.fbr = mat4_mul_vec3(m, f.fbr);
  r.nbl = mat4_mul_vec3(m, f.nbl);
  r.fbl = mat4_mul_vec3(m, f.fbl);
  return r;
}

frustum frustum_translate(frustum f, vec3 v) {
  frustum r;
  r.ntr = vec3_add(f.ntr, v);
  r.ftr = vec3_add(f.ftr, v);
  r.ntl = vec3_add(f.ntl, v);
  r.ftl = vec3_add(f.ftl, v);
  r.nbr = vec3_add(f.nbr, v);
  r.fbr = vec3_add(f.fbr, v);
  r.nbl = vec3_add(f.nbl, v);
  r.fbl = vec3_add(f.fbl, v);
  return r;
}

box frustum_box(frustum f) {
  
  box b;
  b.top     = plane_new(f.ntr, vec3_normalize(vec3_cross(vec3_sub(f.ftr, f.ntr), vec3_sub(f.ntl, f.ntr))));
  b.bottom  = plane_new(f.nbr, vec3_normalize(vec3_cross(vec3_sub(f.nbl, f.nbr), vec3_sub(f.fbr, f.nbr))));
  b.left    = plane_new(f.ntl, vec3_normalize(vec3_cross(vec3_sub(f.ftl, f.ntl), vec3_sub(f.nbl, f.ntl))));
  b.right   = plane_new(f.ntr, vec3_normalize(vec3_cross(vec3_sub(f.nbr, f.ntr), vec3_sub(f.ftr, f.ntr))));
  b.front   = plane_new(f.ftr, vec3_normalize(vec3_cross(vec3_sub(f.ftl, f.ftr), vec3_sub(f.fbr, f.ftr))));
  b.back    = plane_new(f.ntr, vec3_normalize(vec3_cross(vec3_sub(f.nbr, f.ntr), vec3_sub(f.ntl, f.ntr))));
  return b;
}

bool frustum_outside_box(frustum f, box b) {
  error("Unimplemented");
  return false;
}

bool sphere_outside_frustum(sphere s, frustum f) {
  return sphere_outside_box(s, frustum_box(f));
}

bool sphere_inside_frustum(sphere s, frustum f) {
  return sphere_inside_box(s, frustum_box(f));
}

bool sphere_intersects_frustum(sphere s, frustum f) {
  return sphere_intersects_box(s, frustum_box(f));
}

bool sphere_outside_sphere(sphere s1, sphere s2) {
  float rtot = (s1.radius + s2.radius);
  return vec3_dist_sqrd(s1.center, s2.center) > rtot * rtot;
}

sphere sphere_unit() {
  return sphere_new(vec3_zero(), 1);
}

sphere sphere_point() {
  return sphere_new(vec3_zero(), 0);
}

sphere sphere_new(vec3 center, float radius) {
  sphere bs;
  bs.center = center;
  bs.radius = radius;  
  return bs;
}

sphere sphere_of_box(box bb) {
  
  float x_max = bb.left.position.x;
  float x_min = bb.right.position.x;
  float y_max = bb.top.position.y;
  float y_min = bb.bottom.position.y;
  float z_max = bb.front.position.z;
  float z_min = bb.back.position.z;
  
  vec3 center;
  center.x = (x_min + x_max) / 2;
  center.y = (y_min + y_max) / 2;
  center.z = (z_min + z_max) / 2;
  
  float radius = 0;
  radius = max(radius, vec3_dist(center, vec3_new(x_min, y_min, z_min)));
  radius = max(radius, vec3_dist(center, vec3_new(x_max, y_min, z_min)));
  radius = max(radius, vec3_dist(center, vec3_new(x_min, y_max, z_min)));
  radius = max(radius, vec3_dist(center, vec3_new(x_min, y_min, z_max)));
  radius = max(radius, vec3_dist(center, vec3_new(x_min, y_max, z_max)));
  radius = max(radius, vec3_dist(center, vec3_new(x_max, y_max, z_min)));
  radius = max(radius, vec3_dist(center, vec3_new(x_max, y_min, z_max)));
  radius = max(radius, vec3_dist(center, vec3_new(x_max, y_max, z_max)));
  
  sphere bs;
  bs.center = center;
  bs.radius = radius;  
  return bs;
}

sphere sphere_of_frustum(frustum f) {
  
  sphere s;
  s.center = frustum_center(f);
  s.radius = 0;
  s.radius = max(s.radius, vec3_dist(s.center, f.ntr));
  s.radius = max(s.radius, vec3_dist(s.center, f.ftr));
  s.radius = max(s.radius, vec3_dist(s.center, f.ntl));
  s.radius = max(s.radius, vec3_dist(s.center, f.ftl));
  s.radius = max(s.radius, vec3_dist(s.center, f.nbr));
  s.radius = max(s.radius, vec3_dist(s.center, f.fbr));
  s.radius = max(s.radius, vec3_dist(s.center, f.nbl));
  s.radius = max(s.radius, vec3_dist(s.center, f.fbl));
  
  return s;
  
}

sphere sphere_merge(sphere bs1, sphere bs2) {
  
  vec3 dir = vec3_sub(bs2.center, bs1.center);
  vec3 dirnorm = vec3_normalize(dir);
  
  vec3 p0 = vec3_sub(bs1.center, vec3_mul(dirnorm, bs1.radius));
  vec3 p1 = vec3_add(bs2.center, vec3_mul(dirnorm, bs2.radius));
  
  vec3 center = vec3_div(vec3_add(p0, p1), 2);
  float dist = vec3_dist(center, p0);
  
  return sphere_new(center, dist);
}

sphere sphere_merge_many(sphere* s, int count) {
  
  sphere ret = s[0];
  for (int i = 1; i < count; i++) {
    ret = sphere_merge(ret, s[i]);
  }
  
  return ret;
}

bool sphere_inside_box(sphere s, box b) {
  
  if (!sphere_inside_plane(s, b.front))  { return false; }
  if (!sphere_inside_plane(s, b.back))   { return false; }
  if (!sphere_inside_plane(s, b.top))    { return false; }
  if (!sphere_inside_plane(s, b.bottom)) { return false; }
  if (!sphere_inside_plane(s, b.left))   { return false; }
  if (!sphere_inside_plane(s, b.right))  { return false; }
  
  return true;
  
}

bool sphere_outside_box(sphere s, box b) {
  return !(sphere_inside_box(s, b) || sphere_intersects_box(s, b));
}

bool sphere_intersects_box(sphere s, box b) {
  
  vec3 point;
  float radius;
  
  if (sphere_intersects_plane_point(s, b.top, &point, &radius)) {
   
    if (plane_distance(b.left,  point) <= radius &&
        plane_distance(b.right, point) <= radius &&
        plane_distance(b.front, point) <= radius &&
        plane_distance(b.back,  point) <= radius) {
      return true;
    }
    
  }
  
  if (sphere_intersects_plane_point(s, b.bottom, &point, &radius)) {

    if (plane_distance(b.left,  point) <= radius &&
        plane_distance(b.right, point) <= radius &&
        plane_distance(b.front, point) <= radius &&
        plane_distance(b.back,  point) <= radius) {
      return true;
    }
    
  }

  if (sphere_intersects_plane_point(s, b.left, &point, &radius)) {
    
    if (plane_distance(b.top,    point) <= radius &&
        plane_distance(b.bottom, point) <= radius &&
        plane_distance(b.front,  point) <= radius &&
        plane_distance(b.back,   point) <= radius) {
      return true;
    }
    
  }

  if (sphere_intersects_plane_point(s, b.right, &point, &radius)) {
    
    if (plane_distance(b.top,    point) <= radius &&
        plane_distance(b.bottom, point) <= radius &&
        plane_distance(b.front,  point) <= radius &&
        plane_distance(b.back,   point) <= radius) {
      return true;
    }
    
  }

  if (sphere_intersects_plane_point(s, b.front, &point, &radius)) {
    
    if (plane_distance(b.top,    point) <= radius &&
        plane_distance(b.bottom, point) <= radius &&
        plane_distance(b.left,   point) <= radius &&
        plane_distance(b.right,  point) <= radius) {
      return true;
    }
    
  }

  if (sphere_intersects_plane_point(s, b.back, &point, &radius)) {
    
    if (plane_distance(b.top,    point) <= radius &&
        plane_distance(b.bottom, point) <= radius &&
        plane_distance(b.left,   point) <= radius &&
        plane_distance(b.right,  point) <= radius) {
      return true;
    }
    
  }
  
  return false;
  
}

sphere sphere_transform(sphere s, mat4 world) {
  
  vec3 center = mat4_mul_vec3(world, s.center);
  float radius = s.radius * max(max(world.xx, world.yy), world.zz);
  
  return sphere_new(center, radius);
}

sphere sphere_translate(sphere s, vec3 x) {
  s.center = vec3_add(s.center, x);
  return s;
}

sphere sphere_scale(sphere s, float x) {
  s.radius *= x;
  return s;
}

sphere sphere_transform_space(sphere s, mat3 space) {

  vec3 center = mat3_mul_vec3(space, s.center);
  float radius = s.radius * max(max(space.xx, space.yy), space.zz);

  return sphere_new(center, radius);
}

bool point_inside_sphere(sphere s, vec3 point) {
  return vec3_dist(s.center, point) < s.radius;
}

bool point_outside_sphere(sphere s, vec3 point) {
  return vec3_dist(s.center, point) > s.radius;
}

bool point_intersects_sphere(sphere s, vec3 point) {
  return vec3_dist(s.center, point) == s.radius;
}

bool line_inside_sphere(sphere s, vec3 start, vec3 end) {
  return point_swept_inside_sphere(s, vec3_sub(end, start), start);
}

bool line_outside_sphere(sphere s, vec3 start, vec3 end) {
  return point_swept_outside_sphere(s, vec3_sub(end, start), start);
}

bool line_intersects_sphere(sphere s, vec3 start, vec3 end) {
  return point_swept_intersects_sphere(s, vec3_sub(end, start), start);
}

bool sphere_inside_plane(sphere s, plane p) {
  return -plane_distance(p, s.center) > s.radius;
}

bool sphere_outside_plane(sphere s, plane p) {
  return plane_distance(p, s.center) > s.radius;
}

bool sphere_intersects_plane(sphere s, plane p) {
  return fabs(plane_distance(p, s.center)) <= s.radius;
}

bool sphere_intersects_plane_point(sphere s, plane p, vec3* point, float* radius) {
  float d = plane_distance(p, s.center);
  vec3 proj = vec3_mul(p.direction, d);
  *point = vec3_sub(s.center, proj);
  *radius = sqrtf(max(s.radius * s.radius - d * d, 0));
  return fabs(d) <= s.radius; 
}

bool sphere_swept_inside_plane(sphere s, vec3 v, plane p) {

  float angle = vec3_dot(p.direction, v);
  float dist  = vec3_dot(p.direction, vec3_sub(s.center, p.position)); 
  
  if (-dist <= s.radius) { return false; }
  
  float t0 = ( s.radius - dist) / angle;
  float t1 = (-s.radius - dist) / angle;
  
  return (!between_or(t0, 0, 1) && !between_or(t1, 0, 1));
  
}

bool sphere_swept_outside_plane(sphere s, vec3 v, plane p) {

  float angle = vec3_dot(p.direction, v);
  float dist  = vec3_dot(p.direction, vec3_sub(s.center, p.position)); 
  
  if ( dist <= s.radius ) { return false; }
  
  float t0 = ( s.radius - dist) / angle;
  float t1 = (-s.radius - dist) / angle;
  
  return (!between_or(t0, 0, 1) && !between_or(t1, 0, 1));

}

bool sphere_swept_intersects_plane(sphere s, vec3 v, plane p) {

  float angle = vec3_dot(p.direction, v);
  float dist  = vec3_dot(p.direction, vec3_sub(s.center, p.position)); 
  
  if ( fabs(dist) <= s.radius ) { return true; }
  
  float t0 = ( s.radius - dist) / angle;
  float t1 = (-s.radius - dist) / angle;
  
  return (between_or(t0, 0, 1) || between_or(t1, 0, 1));

}

static bool quadratic(float a, float b, float c, float* t0, float* t1) {

  float descrim = b*b - 4*a*c;
  
  if (descrim < 0) {
  
    return false;
  
  } else {
    
    float d = sqrtf(descrim);
    float q = (b < 0) ? (-b - d) / 2.0 : (-b + d) / 2.0;
    
    *t0 = q / a;
    *t1 = c / q;
    
    return true;
  }

}

bool point_swept_inside_sphere(sphere s, vec3 v, vec3 point) {
  
  error("Unimplemented");
  return false;
  
}

bool point_swept_outside_sphere(sphere s, vec3 v, vec3 point) {

  float sdist = vec3_dist_sqrd(point, s.center);
  
  if (sdist <= s.radius * s.radius) { return false; }
  
  vec3  o = vec3_sub(point, s.center);  
  float A = vec3_dot(v, v);
  float B = 2 * vec3_dot(v, o);
  float C = vec3_dot(o, o) - (s.radius * s.radius);
  
  float t0, t1, t;
  if (!quadratic(A, B, C, &t0, &t1)) { return true; }
  
  return (!between_or(t0, 0, 1) && !between_or(t1, 0, 1));

}

bool point_swept_intersects_sphere(sphere s, vec3 v, vec3 point) {

  error("Unimplemented");
  return false;

}

bool sphere_swept_outside_sphere(sphere s1, vec3 v, sphere s2) {
  
  float sdist = vec3_dist_sqrd(s1.center, s2.center);
  float rtot = s1.radius + s2.radius;
  
  if (sdist <= rtot * rtot) { return false; }
  
  vec3  o = vec3_sub(s1.center, s2.center);  
  float A = vec3_dot(v, v);
  float B = 2 * vec3_dot(v, o);
  float C = vec3_dot(o, o) - (rtot * rtot);
  
  float t0, t1, t;
  if (!quadratic(A, B, C, &t0, &t1)) { return true; }
  
  return (!between_or(t0, 0, 1) && !between_or(t1, 0, 1));
  
}

bool sphere_swept_inside_sphere(sphere s1, vec3 v, sphere s2) {

  error("Unimplemented");
  return false;

}

bool sphere_swept_intersects_sphere(sphere s1, vec3 v, sphere s2) {

  error("Unimplemented");
  return false;

}

bool point_inside_triangle(vec3 p, vec3 v0, vec3 v1, vec3 v2) {

  vec3 d0 = vec3_sub(v2, v0);
  vec3 d1 = vec3_sub(v1, v0);
  vec3 d2 = vec3_sub(p, v0);

  float dot00 = vec3_dot(d0, d0);
  float dot01 = vec3_dot(d0, d1);
  float dot02 = vec3_dot(d0, d2);
  float dot11 = vec3_dot(d1, d1);
  float dot12 = vec3_dot(d1, d2);

  float inv_dom = 1.0f / (dot00 * dot11 - dot01 * dot01);
  float u = (dot11 * dot02 - dot01 * dot12) * inv_dom;
  float v = (dot00 * dot12 - dot01 * dot02) * inv_dom;

  return (u >= 0) && (v >= 0) && (u + v < 1);

}

bool sphere_intersects_face(sphere s, vec3 v0, vec3 v1, vec3 v2, vec3 norm) {
  
  if (!sphere_intersects_plane(s, plane_new(v0, norm))) { return false; }
  
  vec3 c = plane_closest(plane_new(v0, norm), s.center);

  return point_inside_triangle(c, v0, v1, v2);
  
}

ellipsoid ellipsoid_new(vec3 center, vec3 radiuses) {
  ellipsoid e;
  e.center = center;
  e.radiuses = radiuses;
  return e;
}

ellipsoid ellipsoid_of_sphere(sphere s) {
  vec3 radiuses = vec3_new(s.radius, s.radius, s.radius);
  return ellipsoid_new(s.center, radiuses);
}

ellipsoid ellipsoid_transform(ellipsoid e, mat4 m) {
  e.center = mat4_mul_vec3(m, e.center);
  e.radiuses = mat3_mul_vec3(mat4_to_mat3(m), e.radiuses);
  return e;
}

mat3 ellipsoid_space(ellipsoid e) {
  
  return mat3_new(
    1.0/e.radiuses.x, 0, 0,
    0, 1.0/e.radiuses.y, 0,
    0, 0, 1.0/e.radiuses.z);
  
}

mat3 ellipsoid_inv_space(ellipsoid e) {

  return mat3_new(
    e.radiuses.x, 0, 0,
    0, e.radiuses.y, 0,
    0, 0, e.radiuses.z);

}

capsule capsule_new(vec3 start, vec3 end, float radius) {
  capsule c;
  c.start = start;
  c.end = end;
  c.radius = radius;
  return c;
}

capsule capsule_transform(capsule c, mat4 m) {
  c.start  = mat4_mul_vec3(m, c.start);
  c.end    = mat4_mul_vec3(m, c.end);
  c.radius = c.radius * max(max(m.xx, m.yy), m.zz);
  return c;
}

bool capsule_inside_plane(capsule c, plane p) {
  return (sphere_inside_plane(sphere_new(c.start, c.radius), p) &&
          sphere_inside_plane(sphere_new(c.end,   c.radius), p));
}

bool capsule_outside_plane(capsule c, plane p) {
  return (sphere_outside_plane(sphere_new(c.start, c.radius), p) &&
          sphere_outside_plane(sphere_new(c.end,   c.radius), p));
}

bool capsule_intersects_plane(capsule c, plane p) {
  return (!capsule_inside_plane(c, p) && !capsule_outside_plane(c, p));
}

vertex vertex_new() {
  vertex v;
  memset(&v, 0, sizeof(vertex));
  return v;
}

bool vertex_equal(vertex v1, vertex v2) {
  
  if(!vec3_equ(v1.position, v2.position)) { return false; }
  if(!vec3_equ(v1.normal, v2.normal)) { return false; }
  if(!vec2_equ(v1.uvs, v2.uvs)) { return false; }
  
  return true;  
}

void vertex_print(vertex v) {

  printf("V(Position: "); vec3_print(v.position);
  printf(", Normal: "); vec3_print(v.normal);
  printf(", Tangent: "); vec3_print(v.tangent);
  printf(", Binormal: "); vec3_print(v.binormal);
  printf(", Color: "); vec4_print(v.color);
  printf(", Uvs: "); vec2_print(v.uvs);
  printf(")");
  
}

void mesh_print(mesh* m) {
  printf("Num Verts: %i\n", m->num_verts);
  printf("Num Tris: %i\n", m->num_triangles);
  for(int i=0; i < m->num_verts; i++) {
    vertex_print(m->verticies[i]); printf("\n");
  }
  printf("Triangle Indicies:");
  for(int i=0; i < m->num_triangles * 3; i++) {
    printf("%i ", m->triangles[i]);
  }
  printf("\n");
}

mesh* mesh_new() {
  
  mesh* m = malloc(sizeof(mesh));
  m->num_verts = 0;
  m->num_triangles = 0;
  m->verticies = malloc(sizeof(vertex) * m->num_verts);
  m->triangles = malloc(sizeof(int) * m->num_triangles * 3);
  
  return m;
  
}

void mesh_delete(mesh* m) {
  free(m->verticies);
  free(m->triangles);
  free(m);
}

void mesh_generate_tangents(mesh* m) {
  
  /* Clear all tangents to 0,0,0 */
  for(int i = 0; i < m->num_verts; i++) {
    m->verticies[i].tangent = vec3_zero();
    m->verticies[i].binormal = vec3_zero();
  }
  
  /* Loop over faces, calculate tangent and append to verticies of that face */
  int i = 0;
  while( i < m->num_triangles * 3) {
    
    int t_i1 = m->triangles[i];
    int t_i2 = m->triangles[i+1];
    int t_i3 = m->triangles[i+2];
    
    vertex v1 = m->verticies[t_i1];
    vertex v2 = m->verticies[t_i2];
    vertex v3 = m->verticies[t_i3];
    
    vec3 face_tangent = triangle_tangent(v1, v2, v3);
    vec3 face_binormal = triangle_binormal(v1, v2, v3);
    
    v1.tangent = vec3_add(face_tangent, v1.tangent);
    v2.tangent = vec3_add(face_tangent, v2.tangent);
    v3.tangent = vec3_add(face_tangent, v3.tangent);
    
    v1.binormal = vec3_add(face_binormal, v1.binormal);
    v2.binormal = vec3_add(face_binormal, v2.binormal);
    v3.binormal = vec3_add(face_binormal, v3.binormal);
    
    m->verticies[t_i1] = v1;
    m->verticies[t_i2] = v2;
    m->verticies[t_i3] = v3;
    
    i = i + 3;
  }
  
  /* normalize all tangents */
  for(int i = 0; i < m->num_verts; i++) {
    m->verticies[i].tangent = vec3_normalize( m->verticies[i].tangent );
    m->verticies[i].binormal = vec3_normalize( m->verticies[i].binormal );
  }
  
}

void mesh_generate_normals(mesh* m) {
  
  /* Clear all normals to 0,0,0 */
  for(int i = 0; i < m->num_verts; i++) {
    m->verticies[i].normal = vec3_zero();
  }
  
  /* Loop over faces, calculate normals and append to verticies of that face */
  int i = 0;
  while( i < m->num_triangles * 3) {
    
    int t_i1 = m->triangles[i];
    int t_i2 = m->triangles[i+1];
    int t_i3 = m->triangles[i+2];
    
    vertex v1 = m->verticies[t_i1];
    vertex v2 = m->verticies[t_i2];
    vertex v3 = m->verticies[t_i3];
    
    vec3 face_normal = triangle_normal(v1, v2, v3);
    
    v1.normal = vec3_add(face_normal, v1.normal);
    v2.normal = vec3_add(face_normal, v2.normal);
    v3.normal = vec3_add(face_normal, v3.normal);
    
    m->verticies[t_i1] = v1;
    m->verticies[t_i2] = v2;
    m->verticies[t_i3] = v3;
    
    i = i + 3;
  }
  
  /* normalize all normals */
  for(int i = 0; i < m->num_verts; i++) {
    m->verticies[i].normal = vec3_normalize( m->verticies[i].normal );
  }
  
}

void mesh_generate_orthagonal_tangents(mesh* m) {
  
  /* Clear all tangents to 0,0,0 */
  for(int i = 0; i < m->num_verts; i++) {
    m->verticies[i].tangent = vec3_zero();
    m->verticies[i].binormal = vec3_zero();
  }
  
  /* Loop over faces, calculate tangent and append to verticies of that face */
  int i = 0;
  while( i < m->num_triangles * 3) {
    
    int t_i1 = m->triangles[i];
    int t_i2 = m->triangles[i+1];
    int t_i3 = m->triangles[i+2];
    
    vertex v1 = m->verticies[t_i1];
    vertex v2 = m->verticies[t_i2];
    vertex v3 = m->verticies[t_i3];
    
    vec3 face_normal = triangle_normal(v1, v2, v3);    
    vec3 face_binormal_temp = triangle_binormal(v1, v2, v3);
    
    vec3 face_tangent = vec3_normalize( vec3_cross(face_binormal_temp, face_normal) );
    vec3 face_binormal = vec3_normalize( vec3_cross(face_tangent, face_normal) );
    
    v1.tangent = vec3_add(face_tangent, v1.tangent);
    v2.tangent = vec3_add(face_tangent, v2.tangent);
    v3.tangent = vec3_add(face_tangent, v3.tangent);
    
    v1.binormal = vec3_add(face_binormal, v1.binormal);
    v2.binormal = vec3_add(face_binormal, v2.binormal);
    v3.binormal = vec3_add(face_binormal, v3.binormal);
    
    m->verticies[t_i1] = v1;
    m->verticies[t_i2] = v2;
    m->verticies[t_i3] = v3;
    
    i = i + 3;
  }
  
  /* normalize all tangents */
  for(int i = 0; i < m->num_verts; i++) {
    m->verticies[i].tangent = vec3_normalize( m->verticies[i].tangent );
    m->verticies[i].binormal = vec3_normalize( m->verticies[i].binormal );
  }
}

void mesh_generate_texcoords_cylinder(mesh* m) {
  
	vec2 unwrap_vector = vec2_new(1, 0);
	
	float max_height = -99999999;
	float min_height = 99999999;
	
	for(int i = 0; i < m->num_verts; i++) {
		float v = m->verticies[i].position.y;
		max_height = max(max_height, v);
		min_height = min(min_height, v);
		
		vec2 proj_position = vec2_new(m->verticies[i].position.x, m->verticies[i].position.z);
		vec2 from_center = vec2_normalize(proj_position);
		float u = (vec2_dot(from_center, unwrap_vector) + 1) / 8;
		
		m->verticies[i].uvs = vec2_new(u, v);
	}
	
	float scale = (max_height - min_height);
	
	for(int i = 0; i < m->num_verts; i++) {
		m->verticies[i].uvs = vec2_new(m->verticies[i].uvs.x, m->verticies[i].uvs.y / scale);
	}
  
}

float mesh_surface_area(mesh* m) {
  
  float total = 0.0;
  
  int i = 0;
  while( i < m->num_triangles * 3) {
  
    int t_i1 = m->triangles[i];
    int t_i2 = m->triangles[i+1];
    int t_i3 = m->triangles[i+2];

    vertex v1 = m->verticies[t_i1];
    vertex v2 = m->verticies[t_i2];
    vertex v3 = m->verticies[t_i3];
    
    total += triangle_area(v1, v2, v3);
    
    i = i + 3;
  }
  
  return total;
  
}

void mesh_translate(mesh* m, vec3 translation) {

  int i = 0;
  while(i < m->num_triangles * 3) {
  
    int t_i1 = m->triangles[i];
    int t_i2 = m->triangles[i+1];
    int t_i3 = m->triangles[i+2];
    
    m->verticies[t_i1].position = vec3_add(m->verticies[t_i1].position, translation);
    m->verticies[t_i2].position = vec3_add(m->verticies[t_i2].position, translation);
    m->verticies[t_i3].position = vec3_add(m->verticies[t_i3].position, translation);
    
    i = i + 3;
  }

}

void mesh_scale(mesh* m, float scale) {

  int i = 0;
  while(i < m->num_triangles * 3) {
  
    int t_i1 = m->triangles[i];
    int t_i2 = m->triangles[i+1];
    int t_i3 = m->triangles[i+2];
    
    m->verticies[t_i1].position = vec3_mul(m->verticies[t_i1].position, scale);
    m->verticies[t_i2].position = vec3_mul(m->verticies[t_i2].position, scale);
    m->verticies[t_i3].position = vec3_mul(m->verticies[t_i3].position, scale);
    
    i = i + 3;
  }

}

void mesh_transform(mesh* m, mat4 transform) {

  int i = 0;
  while(i < m->num_triangles * 3) {
  
    int t_i1 = m->triangles[i];
    int t_i2 = m->triangles[i+1];
    int t_i3 = m->triangles[i+2];
    
    m->verticies[t_i1].position = mat4_mul_vec3(transform, m->verticies[t_i1].position);
    m->verticies[t_i2].position = mat4_mul_vec3(transform, m->verticies[t_i2].position);
    m->verticies[t_i3].position = mat4_mul_vec3(transform, m->verticies[t_i3].position);
    
    i = i + 3;
  }

}

sphere mesh_bounding_sphere(mesh* m) {
  
  sphere s = sphere_new(vec3_zero(), 0);
  
  for (int i = 0; i < m->num_verts; i++) {
    s.center = vec3_add(s.center, m->verticies[i].position);
  }
  s.center = vec3_div(s.center, m->num_verts);
  
  for (int i = 0; i < m->num_verts; i++) {
    s.radius = max(s.radius, vec3_dist(s.center, m->verticies[i].position));
  }
  
  return s;
}

void model_print(model* m) {
  for(int i=0; i<m->num_meshes; i++) {
    mesh_print( m->meshes[i] );
  }
}

model* model_new() {
  model* m = malloc(sizeof(model));
  m->num_meshes = 0;
  m->meshes = malloc(sizeof(mesh*) * m->num_meshes);
  return m;
}

void model_delete(model* m) {
  for(int i=0; i<m->num_meshes; i++) {
    mesh_delete( m->meshes[i] );
  }
  free(m);
}

void model_generate_normals(model* m) {

  for(int i = 0; i < m->num_meshes; i++) {
    mesh_generate_normals( m->meshes[i] );
  }
  
}

void model_generate_tangents(model* m) {

  for(int i = 0; i < m->num_meshes; i++) {
    mesh_generate_tangents( m->meshes[i] );
  }
  
}

void model_generate_orthagonal_tangents(model* m) {
  for(int i = 0; i < m->num_meshes; i++) {
    mesh_generate_orthagonal_tangents( m->meshes[i] );
  }
}

void model_generate_texcoords_cylinder(model* m) {
  for(int i = 0; i < m->num_meshes; i++) {
    mesh_generate_texcoords_cylinder( m->meshes[i] );
  }
}

float model_surface_area(model* m) {
  float total = 0.0f;
  
  for(int i = 0; i < m->num_meshes; i++) {
    total += mesh_surface_area( m->meshes[i] );
  }
  
  return total;
}

void model_translate(model* m, vec3 translation) {
  for(int i = 0; i < m->num_meshes; i++) {
    mesh_translate(m->meshes[i], translation);
  }
}

void model_scale(model* m, float scale) {
  for(int i = 0; i < m->num_meshes; i++) {
    mesh_scale(m->meshes[i], scale);
  }
}

void model_transform(model* m, mat4 transform) {
  for(int i = 0; i < m->num_meshes; i++) {
    mesh_transform(m->meshes[i], transform);
  }
}

vec3 triangle_tangent(vertex vert1, vertex vert2, vertex vert3) {
  
  vec3 pos1 = vert1.position;
  vec3 pos2 = vert2.position;
  vec3 pos3 = vert3.position;
  
  vec2 uv1 = vert1.uvs;
  vec2 uv2 = vert2.uvs;
  vec2 uv3 = vert3.uvs;
  
  /* Get component vectors */
  float x1 = pos2.x - pos1.x;
  float x2 = pos3.x - pos1.x;
  
  float y1 = pos2.y - pos1.y;
  float y2 = pos3.y - pos1.y;

  float z1 = pos2.z - pos1.z;
  float z2 = pos3.z - pos1.z;
  
  /* Generate uv space vectors */
  float s1 = uv2.x - uv1.x;
  float s2 = uv3.x - uv1.x;

  float t1 = uv2.y - uv1.y;
  float t2 = uv3.y - uv1.y;
  
  float r = 1.0f / ((s1 * t2) - (s2 * t1));
  
  vec3 tdir = vec3_new(
    (s1 * x2 - s2 * x1) * r, 
    (s1 * y2 - s2 * y1) * r,
    (s1 * z2 - s2 * z1) * r);
  
  return vec3_normalize(tdir);

}

vec3 triangle_binormal(vertex vert1, vertex vert2, vertex vert3) {
  
  vec3 pos1 = vert1.position;
  vec3 pos2 = vert2.position;
  vec3 pos3 = vert3.position;
  
  vec2 uv1 = vert1.uvs;
  vec2 uv2 = vert2.uvs;
  vec2 uv3 = vert3.uvs;
  
  /* Get component Vectors */
  float x1 = pos2.x - pos1.x;
  float x2 = pos3.x - pos1.x;
  
  float y1 = pos2.y - pos1.y;
  float y2 = pos3.y - pos1.y;

  float z1 = pos2.z - pos1.z;
  float z2 = pos3.z - pos1.z;
  
  /* Generate uv space vectors */
  float s1 = uv2.x - uv1.x;
  float s2 = uv3.x - uv1.x;

  float t1 = uv2.y - uv1.y;
  float t2 = uv3.y - uv1.y;
  
  float r = 1.0f / ((s1 * t2) - (s2 * t1));
  
  vec3 sdir = vec3_new(
          (t2 * x1 - t1 * x2) * r, 
          (t2 * y1 - t1 * y2) * r,
          (t2 * z1 - t1 * z2) * r
          );
  
  return vec3_normalize(sdir);

}

vec3 triangle_normal(vertex v1, vertex v2, vertex v3) {
  vec3 edge1 = vec3_sub(v2.position, v1.position);
  vec3 edge2 = vec3_sub(v3.position, v1.position);
  vec3 normal = vec3_cross(edge1, edge2);
  
  return vec3_normalize(normal);
}

float triangle_area(vertex v1, vertex v2, vertex v3) {
  
  vec3 ab = vec3_sub(v1.position, v2.position);
  vec3 ac = vec3_sub(v1.position, v3.position);
  
  float area = 0.5 * vec3_length(vec3_cross(ab, ac));
  
  return area;
}

vec3 triangle_random_position(vertex v1, vertex v2, vertex v3) {
  
  float r1 = (float)rand() / (float)RAND_MAX;
  float r2 = (float)rand() / (float)RAND_MAX;
  
  if(r1 + r2 >= 1) {
    r1 = 1 - r1;
    r2 = 1 - r2;
  }
  
  vec3 ab = vec3_sub(v1.position, v2.position);
  vec3 ac = vec3_sub(v1.position, v3.position);
  
  vec3 a = v1.position;
  a = vec3_sub(a, vec3_mul(ab , r1) );
  a = vec3_sub(a, vec3_mul(ac , r2) );
  
  return a;
  
}

vertex triangle_random_position_interpolation(vertex v1, vertex v2, vertex v3) {

  float r1 = (float)rand() / (float)RAND_MAX;
  float r2 = (float)rand() / (float)RAND_MAX;
  
  if(r1 + r2 >= 1) {
    r1 = 1 - r1;
    r2 = 1 - r2;
  }
  
  vertex v;
  
  vec3 v_pos, v_norm, v_tang, v_binorm;
  vec4 v_col;
  vec2 v_uv;
  
  v_pos = v1.position;
  v_pos = vec3_sub(v_pos, vec3_mul(vec3_sub(v1.position, v2.position) , r1) );
  v_pos = vec3_sub(v_pos, vec3_mul(vec3_sub(v1.position, v3.position) , r2) );
  
  v_norm = v1.normal;
  v_norm = vec3_sub(v_norm, vec3_mul(vec3_sub(v1.normal, v2.normal) , r1) );
  v_norm = vec3_sub(v_norm, vec3_mul(vec3_sub(v1.normal, v3.normal) , r2) );
  
  v_tang = v1.tangent;
  v_tang = vec3_sub(v_tang, vec3_mul(vec3_sub(v1.tangent, v2.tangent) , r1) );
  v_tang = vec3_sub(v_tang, vec3_mul(vec3_sub(v1.tangent, v3.tangent) , r2) );
  
  v_binorm = v1.binormal;
  v_binorm = vec3_sub(v_binorm, vec3_mul(vec3_sub(v1.binormal, v2.binormal) , r1) );
  v_binorm = vec3_sub(v_binorm, vec3_mul(vec3_sub(v1.binormal, v3.binormal) , r2) );
  
  v_col = v1.color;
  v_col = vec4_sub(v_col, vec4_mul(vec4_sub(v1.color, v2.color) , r1) );
  v_col = vec4_sub(v_col, vec4_mul(vec4_sub(v1.color, v3.color)  , r2) );
  
  v_uv = v1.uvs;
  v_uv = vec2_sub(v_uv, vec2_mul(vec2_sub(v1.uvs, v2.uvs) , r1) );
  v_uv = vec2_sub(v_uv, vec2_mul(vec2_sub(v1.uvs, v3.uvs)  , r2) );
  
  v.position = v_pos;
  v.normal = v_norm;
  v.tangent = v_tang;
  v.binormal = v_binorm;
  v.color = v_col;
  v.uvs = v_uv;
  
  return v;
}


float triangle_difference_u(vertex v1, vertex v2, vertex v3) {
  float max = v1.uvs.x;
  max = v2.uvs.x > max ? v2.uvs.x : max;
  max = v3.uvs.x > max ? v3.uvs.x : max;
  
  float min = v1.uvs.x;
  min = v2.uvs.x < min ? v2.uvs.x : min;
  min = v3.uvs.x < min ? v3.uvs.x : min;
  
  return max - min;
}

float triangle_difference_v(vertex v1, vertex v2, vertex v3) {
  float max = v1.uvs.y;
  max = v2.uvs.y > max ? v2.uvs.y : max;
  max = v3.uvs.y > max ? v3.uvs.y : max;
  
  float min = v1.uvs.y;
  min = v2.uvs.y < min ? v2.uvs.y : min;
  min = v3.uvs.y < min ? v3.uvs.y : min;
  
  return max - min;
}


float tween_approach(float curr, float target, float timestep, float steepness) {
  return lerp(curr, target, saturate(steepness * timestep));
}


float tween_linear(float curr, float target, float timestep, float max) {
  return curr + clamp(target - curr, -max * timestep, max * timestep);
}

vec3 vec3_tween_approach(vec3 curr, vec3 target, float timestep, float steepness) {
  return vec3_lerp(curr, target, saturate(steepness * timestep));
}

vec3 vec3_tween_linear(vec3 curr, vec3 target, float timestep, float max) {
  float dist = vec3_dist(curr, target);
  vec3  dirr = vec3_normalize(vec3_sub(target, curr));
  return vec3_add(curr, vec3_mul(dirr, min(dist, max * timestep)));
  
}

