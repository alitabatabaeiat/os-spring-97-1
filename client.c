#include "util.h"

#define MAXDATASIZE 100

int main(int argc, char const *argv[]) {
  if (argc != 3) {
    prints("ERR! ip & port number\n");
    return 0;
  }

  char *ip = (char *) argv[1];
  char *port = (char *) argv[2];
  int socketfd, numbytes;
  char buf[MAXDATASIZE];
  struct addrinfo hints, *servinfo, *p;
  char s[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(ip, port, &hints, &servinfo) != 0) {
    prints("ERR! getaddrinfo\n");
    return 0;
  }
    // loop through all the results and connect to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      prints("ERR! socket\n");
      continue;
    }
    if (connect(socketfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(socketfd);
      prints("ERR! connect\n");
      continue;
    }
    break;
  }
  if (p == NULL) {
    prints("ERR! failed to connect\n");
    return 0;
  }
  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);

  prints("connecting to ");
  prints(s + '\n');
  freeaddrinfo(servinfo); // all done with this structure
  if ((numbytes = recv(socketfd, buf, MAXDATASIZE-1, 0)) == -1) {
    prints("ERR! recv\n");
    return 0;
  }
  buf[numbytes] = '\0';
  prints("server: ");
  prints(buf);
  close(socketfd);
  return 0;
}
