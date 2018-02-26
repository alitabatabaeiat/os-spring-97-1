#include "util.h"

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
    print("ERR! getaddrinfo have error");
    return 0;
  }
  return 0;
}
