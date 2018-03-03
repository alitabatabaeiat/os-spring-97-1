#include "util.h"

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
  for (int i = 0; i < USERS; i++)
    clients[i].status = NOT_CONNECTED;


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

  FD_SET(STDIN_FILENO, &master);
  if (STDIN_FILENO > fdmax)
    fdmax = STDIN_FILENO;
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
            clients[new_fd - FIRST_CLIENT].status = CONNECTED;
            if (new_fd > fdmax)
              fdmax = new_fd;
            prints("new connection from ");
            prints((char *)inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*) &remoteaddr), remoteIP, INET6_ADDRSTRLEN));
            prints(" on socket ");
            printi(new_fd);prints("\n");
          }
        } else if (i == STDIN_FILENO) {
          myread(buffer, CHUNK_SIZE);
          removeSpaces(buffer);
          if (strncmp(buffer, "getc", 4) == 0) {
            printClients(clients);
          } else {
            printl("ERR! Unkonwn Command");
          }
        } else {
          int nbytes;
          if ((nbytes = recv(i, buffer, CHUNK_SIZE, 0)) <= 0) {
            if (nbytes == 0) {
              prints("socket ");
              printi(i);
              printl(" hung up");
            }
            else
              printl("ERR! recv");
            close(i);
            FD_CLR(i, &master);
            clearClient(&clients[i - FIRST_CLIENT]);
          } else {
            char *cmd = strtok(buffer, ";");
            if (strcmp(cmd, INIT) == 0) {
              char phone[PHONE_LEN];
              int duplicate = FALSE;
              strcpy(phone, strtok(NULL, ";"));
              for (int j = 0; j < USERS; j++) {
                if (strcmp(clients[j].phone, phone) == 0 && clients[j].status > CONNECTED) {
                  duplicate = TRUE;
                  break;
                }
              }
              if (duplicate == TRUE) {
                memset(buffer, '\0', CHUNK_SIZE);
                strcpy(buffer, DUPLICATE);
                sendAll(i, buffer, CHUNK_SIZE);
              } else {
                strcpy(clients[i - FIRST_CLIENT].phone, phone);
                strcpy(clients[i - FIRST_CLIENT].ip, strtok(NULL, ";"));
                strcpy(clients[i - FIRST_CLIENT].port, strtok(NULL, ";"));
                clients[i - FIRST_CLIENT].status = INITIALIZED;
                sendAll(i, "\0", 1);
              }
              printClients(clients);
            } else if (strcmp(cmd, CALL) == 0) {
              char phone[PHONE_LEN];
              strcpy(phone, strtok(NULL, ";"));
              for (int j = 0; j < USERS; j++) {
                if ((i - FIRST_CLIENT) != j && (clients[j].status > CONNECTED) &&
                    strcmp(phone, clients[j].phone) == 0) {
                  if (clients[j].status == CHATTING) {
                    memset(buffer, '\0', CHUNK_SIZE);
                    strcpy(buffer, "chatting");
                    sendAll(i, buffer, CHUNK_SIZE);
                  } else if (INITIALIZED) {
                    char *s1[] = {
                      clients[j].phone,
                      clients[j].ip,
                      clients[j].port
                    };
                    produceBuffer(buffer, s1, sizeof (s1) / sizeof (const char *));
                    sendAll(i, buffer, CHUNK_SIZE);
                    clients[i - FIRST_CLIENT].status = CHATTING;
                    clients[j].status = CHATTING;
                    memset(buffer, '\0', CHUNK_SIZE);
                    char *s2[] = {
                      CALLER,
                      clients[i - FIRST_CLIENT].phone,
                      clients[i - FIRST_CLIENT].ip,
                      clients[i - FIRST_CLIENT].port
                    };
                    produceBuffer(buffer, s2, sizeof (s2) / sizeof (const char *));
                    sendAll(j + FIRST_CLIENT, buffer, CHUNK_SIZE);
                  }
                }
              }
            } else if (strcmp(cmd, DISCONNECT) == 0) {
              char phone[PHONE_LEN];
              strcpy(phone, strtok(NULL, ";"));
              clients[i - FIRST_CLIENT].status = INITIALIZED;
              for (int j = 0; j < USERS; j++)
                if (strcmp(phone, clients[j].phone) == 0)
                  clients[j].status = INITIALIZED;
            }
            memset(buffer, '\0', CHUNK_SIZE);
          }
        } // END handle data from client
      } // END got new incoming connection
    } // END looping through file descriptors
  }

  return 0;
}
