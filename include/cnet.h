#ifndef cnet_h
#define cnet_h

#include "cengine.h"

void net_init(void);
void net_finish(void);

void net_set_server(bool server);
bool net_is_server(void);
bool net_is_client(void);

enum {
  HTTP_ERR_NONE   = 0,
  HTTP_ERR_URL    = 1,
  HTTP_ERR_HOST   = 2,
  HTTP_ERR_SOCKET = 3,
  HTTP_ERR_DATA   = 4,
  HTTP_ERR_NOFILE = 5,
};

int net_http_get(char* out, int max, char* fmt, ...);
int net_http_upload(const char* filename, char* fmt, ...);

#endif