#include "util.h"

#define BACKLOG 10

int  main(int argc, char const *argv[]) {
  if (argc != 2) {
    print("ERR! port number?");
    return 0;
  }
  char port = argv[1];
  struct addrinfo hints, *res;
  int socketfd;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if (getaddrinfo(NULL, port, &hints, &res) != 0) {
    print("ERR! getaddrinfo");
    return 0;
  }

  for(p = res; p != NULL; p = p->ai_next) {
    if ((socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      print("ERR! socket");
      continue;
    }
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      print("ERR! setsockopt");
      return 0;
    }
    if (bind(socketfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(socketfd);
      print("ERR! bind");
      continue;
    }
    break;
  }

  
  return 0;
}
