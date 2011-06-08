#include "logger.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int logging_enabled = 1;
static int console_logging_enabled = 1; 
static int file_logging_enabled = 1;

void logger_enable() {
  logging_enabled = 1;
};

void logger_disable() {
  logging_enabled = 0;
};

void logger_enable_console() {
  console_logging_enabled = 1;
};

void logger_disable_console() {
  console_logging_enabled = 0;
};

void logger_enable_files() {
  file_logging_enabled = 1;
};

void logger_disable_files() {
  file_logging_enabled = 0;
};

static char log_name[56];

void log_to(const char* filename, char* fmt, ...) {

  if (!logging_enabled) return;
  if (!file_logging_enabled) return;
  
  log_name[0] = '\0';
  strcat(log_name, "./logs/"); strcat(log_name, filename); strcat(log_name, ".log"); 
  
  FILE* file = fopen(log_name,"a");    
  
  va_list args;
  va_start(args,fmt);
  vfprintf(file,fmt,args);
  va_end(args);
  
  fflush(file);
  fclose(file);
}

void log_stdout(char* fmt, ...) {

  if (!logging_enabled) return;
  if (!console_logging_enabled) return;

  va_list args;
  va_start(args,fmt);
  vfprintf(stdout,fmt,args);
  va_end(args);
  
  fflush(stdout);
}

void log_stderr(char* fmt, ...) {

  if (!logging_enabled) return;
  if (!console_logging_enabled) return;

  va_list args;
  va_start(args,fmt);
  vfprintf(stderr,fmt,args);
  va_end(args);
  
  fflush(stderr);
}