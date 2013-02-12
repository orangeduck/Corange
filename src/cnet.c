
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

int SDLNet_TCP_RecvLine(TCPsocket sock, char* data, int maxlen) {
  
  char c;
  int status = 0;
  int i = 0;
  while(1) {
    
    status = SDLNet_TCP_Recv(sock, &c, 1);
    
    if (status == -1) return -1;
    if (i == maxlen-1) return -1;
    if (status == 0) break;
    
    data[i] = c;
    i++;
    
    if (c == '\n') {
      data[i] = '\0';
      return i;
    }
  }
  
  if(i > 0) {
    data[i] = '\0';
    return i;
  } else {
    return 0;
  }
  
}

static char url_buffer[512];
static char host_buffer[512];
static char path_buffer[512];

int net_http_get(char* out, int max, char* fmt, ...) {
  
  va_list args;
  va_start(args, fmt);
  vsnprintf(url_buffer, 500, fmt, args);
  
  int parts = sscanf(url_buffer, "http://%[^/]%s", host_buffer, path_buffer);
  
  if (parts != 2) {
    warning("Couldn't resolve parts of URL '%s'", url_buffer);
    return HTTP_ERR_URL;
  }
  
  debug("Host: %s Path: %s", host_buffer, path_buffer);
  
  IPaddress ip;
  if ( SDLNet_ResolveHost(&ip, host_buffer, 80) == -1) {
    warning("Couldn't Resolve Host: %s", SDLNet_GetError());
    return HTTP_ERR_HOST;
  }

  TCPsocket sock = SDLNet_TCP_Open(&ip);
  
  if(!sock) {
    warning("Couldn't open socket: %s", SDLNet_GetError());
    return HTTP_ERR_SOCKET;
  }
  
  char sockout[1024];
  sprintf(sockout, "GET %256s HTTP/1.1\r\nHost: %256s\r\n\r\n", path_buffer, host_buffer);
  
  int result = SDLNet_TCP_Send(sock, sockout, strlen(sockout)+1);
  
  if (result < strlen(sockout)+1) {
    warning("Error sending http request: %s", SDLNet_GetError());
    return HTTP_ERR_DATA;
  }
  
  char line[1024];
  bool header = true;
  
  strcpy(out, "");
  
  while (SDLNet_TCP_RecvLine(sock, line, 1023)) {
    
    /* TODO: Check against max arg */
    
    if (header) {
      if (strcmp(line, "\r\n") == 0) { header = false; }
    } else {
      strcat(out, line);
    }
    
  }
  
  return HTTP_ERR_NONE;
  
}