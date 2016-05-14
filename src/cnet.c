
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
  va_end(args);
  
  int parts = sscanf(url_buffer, "http://%[^/]%s", host_buffer, path_buffer);
  
  if (parts != 2) {
    warning("Couldn't resolve parts of URL '%s'", url_buffer);
    return HTTP_ERR_URL;
  }
  
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
  sprintf(sockout,
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "\r\n", path_buffer, host_buffer);
  
  int result = SDLNet_TCP_Send(sock, sockout, strlen(sockout));
  
  if (result < strlen(sockout)) {
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

int net_http_upload(const char* filename, char* fmt, ...) {

  va_list args;
  va_start(args, fmt);
  vsnprintf(url_buffer, 500, fmt, args);
  va_end(args);
  
  int parts = sscanf(url_buffer, "http://%[^/]%s", host_buffer, path_buffer);
  
  if (parts != 2) {
    warning("Couldn't resolve parts of URL '%s'", url_buffer);
    return HTTP_ERR_URL;
  }
  
  IPaddress ip;
  if ( SDLNet_ResolveHost(&ip, host_buffer, 80) == -1) {
    warning("Couldn't Resolve Host: %s", SDLNet_GetError());
    return HTTP_ERR_HOST;
  }

  TCPsocket sock = SDLNet_TCP_Open(&ip);
  
  if (!sock) {
    warning("Couldn't open socket: %s", SDLNet_GetError());
    return HTTP_ERR_SOCKET;
  }
  
  SDL_RWops* file = SDL_RWFromFile(filename, "r");
  
  if (file == NULL) {
    warning("Couldn't Open File '%s' to upload", filename);
    return HTTP_ERR_NOFILE;
  }
  
  size_t size = SDL_RWseek(file,0,SEEK_END);
  char* contents = malloc(size+1);
  contents[size] = '\0';
  
  SDL_RWseek(file, 0, SEEK_SET);
  SDL_RWread(file, contents, size, 1);
  SDL_RWclose(file);
  
  char sockbody[size + 1024];
  char sockheaders[1024];
  
  sprintf(sockbody,
    "--CorangeUploadBoundary\r\n"
    "content-disposition: form-data; name=\"corangeupload\"; filename=\"%s\"\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "%s\r\n"
    "--CorangeUploadBoundary--\r\n"
    "\r\n", filename, contents);
  
  sprintf(sockheaders, 
    "POST %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Content-Length: %i\r\n"
    "Content-Type: multipart/form-data; boundary=CorangeUploadBoundary\r\n"
    "\r\n" , path_buffer, host_buffer, (int)strlen(sockbody));
  
  int result = 0;
  
  result = SDLNet_TCP_Send(sock, sockheaders, strlen(sockheaders));
  if (result < strlen(sockheaders)) {
    warning("Error sending http request: %s", SDLNet_GetError());
    return HTTP_ERR_DATA;
  }

  result = SDLNet_TCP_Send(sock, sockbody, strlen(sockbody));
  if (result < strlen(sockbody)) {
    warning("Error sending http request: %s", SDLNet_GetError());
    return HTTP_ERR_DATA;
  }
  
  char line[1024];  
  while (SDLNet_TCP_RecvLine(sock, line, 1023)) {
    //debug("Recived: %s", line);
  }
  
  return HTTP_ERR_NONE;

}