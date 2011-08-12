#ifndef logger_h
#define logger_h

#include <stdarg.h>

void logger_enable();
void logger_disable();

void logger_enable_console();
void logger_disable_console();

void logger_enable_files();
void logger_disable_files();

#define log_msg(A, ... ) log_stdout(A, __VA_ARGS__); log_to("all", A, __VA_ARGS__)
#define log_debug(A, ... ) log_stdout(A, __VA_ARGS__); log_to("debug",A, __VA_ARGS__); log_to("all",A, __VA_ARGS__)
#define log_warning(A, ... ) log_stdout(A, __VA_ARGS__); log_to("warning",A, __VA_ARGS__); log_to("all",A, __VA_ARGS__)
#define log_error(A, ... ) log_stdout(A, __VA_ARGS__); log_to("error",A, __VA_ARGS__); log_to("all",A, __VA_ARGS__)
#define log_asset(A, ... ) log_stdout(A, __VA_ARGS__); log_to("asset",A, __VA_ARGS__); log_to("all",A, __VA_ARGS__)

void log_to(const char* logname, char* fmt, ...);

void log_stdout(char* fmt, ...);
void log_stderr(char* fmt, ...);

#endif