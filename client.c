#include "util.h"


int main(int argc, char const *argv[]) {
  if (argc != 5) {
    printl("ERR! ips & port numbers?");
    return 0;
  }

  struct Client me;
  strcpy(me.ip, (char *) argv[1]);
  strcpy(me.port, (char *) argv[2]);
  char *serverIp = (char *) argv[3];
  char *serverPort = (char *) argv[4];
  int serverfd;
  struct addrinfo hints, *servinfo, *p;
  char buffer[CHUNK_SIZE] = {0};
  char filename[CHUNK_SIZE];

  int socketfd, numbytes;
  fd_set master;
  fd_set read_fds;
  int fdmax;
  struct Client client;
  client.status = NOT_CONNECTED;

  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  // client connect to server
  memset(&hints, 0, sizeof hints);
  setHints(&hints, FALSE);

  if (getaddrinfo(serverIp, serverPort, &hints, &servinfo) != 0) {
    printl("ERR! getaddrinfo");
    return 0;
  }

  serverfd = runClient(&p, servinfo);
  if (p == NULL) {
    printl("ERR! failed to connect ");
    return 0;
  }
  me.status = INITIALIZED;
  // inet_ntop(p->ai_family,
  //           get_in_addr((struct sockaddr *)p->ai_addr),
  //           serverS, sizeof serverS);
  prints("connecting to server on port ");
  printl(serverPort);
  freeaddrinfo(servinfo);

  // make client, server
  memset(&hints, 0, sizeof hints);
  setHints(&hints, FALSE);
  if (getaddrinfo(me.ip, me.port, &hints, &servinfo) != 0) {
    printl("ERR! getaddrinfo");
    return 0;
  }

  socketfd = runServer(&p, servinfo);
  if (p == NULL) {
    printl("ERR! failed to bind");
    return 0;
  }

  if (listen(socketfd, BACKLOG) == -1) {
    printl("ERR! listen");
    return 0;
  }

  FD_SET(socketfd, &master);
  fdmax = socketfd;

  FD_SET(serverfd, &master);
  if (serverfd > fdmax)
    fdmax = serverfd;

  FD_SET(STDIN_FILENO, &master);
  if (STDIN_FILENO > fdmax)
    fdmax = STDIN_FILENO;

  int saved = FALSE;
  while (saved == FALSE) {
    prints("Hi, please enter your phone number: ");
    myread(me.phone, PHONE_LEN);
    char *s[] = {
      INIT,
      me.phone,
      me.ip,
      me.port
    };
    produceBuffer(buffer, s, sizeof (s) / sizeof (const char *));
    sendAll(serverfd, buffer, CHUNK_SIZE);
    int nbytes;
    if ((nbytes = recv(serverfd, buffer, CHUNK_SIZE, 0)) < 0) {
      printl("ERR! recv");
      return 0;
    } else {
      if (strcmp(buffer, DUPLICATE) == 0) {
        printl("-- Duplicate Number");
      } else {
        saved = TRUE;
      }
    }
  }

  int done = FALSE;
  while (done == FALSE) {
    read_fds = master;
    if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
      printl("ERR! select");
      return 0;
    }
    for(int i = 0; i <= fdmax; i++) {
      if (FD_ISSET(i, &read_fds)) {
        if (i == socketfd) {
          int res =  clientGetConnection(i, &client, &master);
          if (res > fdmax)
            fdmax = res;
        } else if (i == STDIN_FILENO) {
          myread(buffer, CHUNK_SIZE);
          if (client.status == NOT_CONNECTED) {
            removeSpaces(buffer);
            if (strncmp(buffer, CALL, strlen(CALL)) == 0) {
              char phone[PHONE_LEN];
              strcpy(phone, buffer + strlen(CALL));
              if (strcmp(phone, me.phone) == 0) {
                printl("-- It's Your Phone Number");
              } else {
                char *s[] = {
                  CALL,
                  phone
                };
                produceBuffer(buffer, s, sizeof (s) / sizeof (const char *));
                sendAll(serverfd, buffer, CHUNK_SIZE);
                recv(serverfd, buffer, CHUNK_SIZE, 0);
                if (strcmp(buffer, "chatting") == 0) {
                  prints("--server: ");
                  prints(phone);
                  printl(" is chatting with someone else!");
                } else {
                  int res = callSomeone(buffer, &client, &master, phone);
                  if (res > fdmax)
                    fdmax = res;
                }
              }
            } else if (strcmp(buffer, EXIT) == 0) {
              done = TRUE;
            } else {
              printl("ERR! Unkonwn Command");
            }
          } else {
            if (strcmp(buffer, DISCONNECT) == 0) {
              clientDisconnect(serverfd, buffer, &client, &master);
            } else if (strncmp(buffer, SEND_FILE, strlen(SEND_FILE)) == 0) {
              sendFileCmd(filename, buffer, client.fd, me.phone);
            } else {
              sendAll(client.fd, buffer, CHUNK_SIZE);
            }
          }
        } else if (i == client.fd) {
          int nbytes;
          if ((nbytes = recv(i, buffer, CHUNK_SIZE, 0)) <= 0) {
            if (nbytes == 0) {
              prints("socket ");
              printi(i);
              printl(" hung up");
            }
            else
              printl("ERR! recv");
            close(client.fd);
            FD_CLR(client.fd, &master);
            clearClient(&client);
            client.fd = INVALID_FD;
          } else {
            if (client.status != SENDING_FILE) {
              char *cmd = strtok(buffer, ";");
              if (strcmp(cmd, DISCONNECT) == 0) {
                close(client.fd);
                FD_CLR(client.fd, &master);
                clearClient(&client);
                client.fd = INVALID_FD;
              } else if (strcmp(cmd, SEND_FILE) == 0) {
                strcpy(filename, strtok(NULL, ";"));
                client.status = SENDING_FILE;
              } else {
                prints(client.phone);
                prints(": ");
                printl(buffer);
              }
            } else {
              if (strncmp(buffer, DONE, strlen(DONE)) == 0) {
                client.status = CHATTING;
                printl("-- File Received & Saved!");
              } else {
                receivingFile(filename, buffer, me.phone);
              }
            }
          }
        } else if (i == serverfd) {
          int nbytes;
          if ((nbytes = recv(i, buffer, CHUNK_SIZE, 0)) > 0) {
            printl(buffer);
            char *cmd = strtok(buffer, ";");
            if (strcmp(cmd, CALLER) == 0) {
              strcpy(client.phone, strtok(NULL, ";"));
              strcpy(client.ip, strtok(NULL, ";"));
              strcpy(client.port, strtok(NULL, ";"));
              prints("new connection from Client ");
              printl(client.phone);
            }
          }
        }
      }
    }
  }

  close(serverfd);
  close(socketfd);
  return 0;
}
