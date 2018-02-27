#include "util.h"

struct Client {
  int fd;
  char phone[PHONE_LEN];
  char port[PORT_LEN];
};

int  main(int argc, char const *argv[]) {
  if (argc != 2) {
    printl("ERR! port number?");
    return 0;
  }

  char *port = (char *) argv[1];
  int socketfd, new_fd;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage remoteaddr;
  socklen_t sin_size;
  char remoteIP[INET6_ADDRSTRLEN];
  fd_set master;
  fd_set read_fds;
  int fdmax;
  char buffer[CHUNK_SIZE] = {0};
  struct Client clients[USERS];
  int numOfUsers = 0;


  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  memset(&hints, 0, sizeof hints);
  setHints(&hints, TRUE);
  if (getaddrinfo(NULL, port, &hints, &servinfo) != 0) {
    printl("ERR! getaddrinfo");
    return 0;
  }

  socketfd = runServer(&p, servinfo);

  freeaddrinfo(servinfo);

  if (p == NULL)  {
    printl("ERR! failed to bind");
    return 0;
  }

  if (listen(socketfd, BACKLOG) == -1) {
    printl("ERR! listen");
    return 0;
  }

  FD_SET(socketfd, &master);
  fdmax = socketfd;
  printl("waiting for connections...");

  while (1) {
    read_fds = master;
    if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
      printl("ERR! select");
      return 0;
    }
    for(int i = 0; i <= fdmax; i++) {
      if (FD_ISSET(i, &read_fds)) {
        if (i == socketfd) {
          int addrlen = sizeof remoteaddr;
          new_fd = accept(socketfd, (struct sockaddr *)&remoteaddr, &addrlen);
          if (new_fd == -1) {
            printl("ERR! accept");
          } else {
            FD_SET(new_fd, &master);
            clients[numOfUsers].fd = new_fd;
            memset(clients[numOfUsers].phone, 0, PHONE_LEN);
            numOfUsers++;
            if (new_fd > fdmax)
              fdmax = new_fd;
            prints("new connection from ");
            prints((char *)inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*) &remoteaddr), remoteIP, INET6_ADDRSTRLEN));
            prints(" on socket ");
            printi(new_fd);prints("\n");
          }
        } else {
          // handle data from a client
          int nbytes;
          if ((nbytes = recv(i, buffer, CHUNK_SIZE, 0)) <= 0) {
            // got error or connection closed by client
            if (nbytes == 0) {
              // connection closed
              prints("socket ");
              printi(i);
              printl(" hung up");
            }
            else
              printl("ERR! recv");
            close(i); // bye!
            FD_CLR(i, &master); // remove from master set
            for (int j = 0; j < numOfUsers; j++) {
              if (clients[j].fd == i) {
                if (j != numOfUsers - 1) {
                  clients[j].fd = clients[numOfUsers - 1].fd;
                  strcpy(clients[j].phone, clients[numOfUsers - 1].phone);
                  strcpy(clients[j].port, clients[numOfUsers - 1].port);
                }
                clients[j].fd = INVALID_FD;
                memset(clients[j].phone, 0, PHONE_LEN);
                memset(clients[j].port, 0, PORT_LEN);
              }
            }
            numOfUsers--;
          } else {
            printl(buffer);

            char *cmd = strtok(buffer, ";");
            if (strcmp(cmd, INIT) == 0) {
              char *phone = strtok(NULL, ";");
              char *port = strtok(NULL, ";");
              for (int j = 0; j < numOfUsers; j++) {
                if (clients[j].fd == i) {
                  strcpy(clients[j].phone, phone);
                  strcpy(clients[j].port, port);
                }
              }
              printl("----------");
              for (int j = 0; j < numOfUsers; j++) {
                if (clients[j].fd != INVALID_FD) {
                  prints("client ");
                  printi(clients[j].fd);
                  prints(": ");
                  if (clients[j].phone == "\0")
                    printl("waiting for phone number...");
                  else {
                    prints(clients[j].phone);
                    prints(" - port: ");
                    printl(clients[j].port);
                  }
                }
              }
            } else if (strcmp(cmd, CALL) == 0) {
              char *phone = strtok(NULL, ";");
              for (int j = 0; j < numOfUsers; j++) {
                if (clients[j].fd != i && clients[j].fd != INVALID_FD) {
                  if (strcmp(phone, clients[j].phone) == 0) {
                    printl("---");
                    printi(i);
                    send(i, clients[j].port, PORT_LEN, 0);
                  }
                }
              }
            }
            memset(buffer, '\0', CHUNK_SIZE);
          }
        } // END handle data from client
      } // END got new incoming connection
    } // END looping through file descriptors
  }

  return 0;
}
