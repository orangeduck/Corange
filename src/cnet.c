
#include "cnet.h"

static bool is_server = false;

void net_init(void) {
  
  if (SDLNet_Init() < 0) {
    error("Could not initialize SDL Net: %s\n", SDLNet_GetError());
  }

}

void net_finish(void) {
  
  SDLNet_Quit();
  
}

void net_set_server(bool server) {
  is_server = server;
}

bool net_is_server(void) {
  return is_server;
}

bool net_is_client(void) {
  return !is_server;
}