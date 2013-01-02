#ifndef cnet_h
#define cnet_h

#include "cengine.h"

void net_init(void);
void net_finish(void);

void net_set_server(bool server);
bool net_is_server(void);
bool net_is_client(void);

#endif