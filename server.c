#include "util.h"

#define BACKLOG 10

int  main(int argc, char const *argv[]) {
  if (argc != 2) {
    prints("ERR! port number?\n");
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
    prints("ERR! getaddrinfo\n");
    return 0;
  }

  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      prints("ERR! socket\n");
      continue;
    }
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      prints("ERR! setsockopt\n");
      return 0;
    }
    if (bind(socketfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(socketfd);
      prints("ERR! bind\n");
      continue;
    }
    break;
  }

  freeaddrinfo(servinfo);

  if (p == NULL)  {
    prints("ERR! failed to bind\n");
    return 0;
  }

  if (listen(socketfd, BACKLOG) == -1) {
    prints("ERR! listen\n");
    return 0;
  }

  prints("waiting for connections...\n");

  while (1) {
    sin_size = sizeof their_addr;
    new_fd = accept(socketfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      prints("ERR! accept\n");
      continue;
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    prints("new connection from ");
    prints(s);

    if


    if (!fork()) {
      close(socketfd);
      if (send(new_fd, "Hello, world!", 13, 0) == -1)
        prints("ERR! send\n");
      close(new_fd);
      return 0;
    }
    close(new_fd);
  }

  prints("all done\n");
  return 0;
}
