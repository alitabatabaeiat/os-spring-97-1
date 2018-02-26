#include "util.h"

#define BACKLOG 10

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET)
    return &(((struct sockaddr_in*)sa)->sin_addr);
    
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int  main(int argc, char const *argv[]) {
  if (argc != 2) {
    print("ERR! port number?");
    return 0;
  }
  char *port = (char *) argv[1];
  int socketfd, new_fd;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr;
  socklen_t sin_size;
  int yes = 1;
  char s[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if (getaddrinfo(NULL, port, &hints, &servinfo) != 0) {
    print("ERR! getaddrinfo");
    return 0;
  }

  for(p = servinfo; p != NULL; p = p->ai_next) {
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

  freeaddrinfo(servinfo);

  if (p == NULL)  {
    print("ERR! failed to bind");
    return 0;
  }

  if (listen(socketfd, BACKLOG) == -1) {
    print("ERR! listen");
    return 0;
  }

  print("waiting for connections...");

  while (1) {
    sin_size = sizeof their_addr;
    new_fd = accept(socketfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      print("ERR! accept");
      continue;
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    print("new connection from ");
    print(s);


    if (!fork()) {
      close(socketfd);
      if (send(new_fd, "Hello, world!", 13, 0) == -1)
        print("ERR! send");
      close(new_fd);
      return 0;
    }
    close(new_fd);
  }

  print("all done");
  return 0;
}
