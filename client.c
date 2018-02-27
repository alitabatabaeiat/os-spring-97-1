#include "util.h"


int main(int argc, char const *argv[]) {
  if (argc != 4) {
    printl("ERR! ip & port number & server port number?");
    return 0;
  }

  char *ip = (char *) argv[1];
  char *port = (char *) argv[2];
  char *serverport = (char *) argv[3];
  int socketfd, numbytes;
  struct addrinfo hints, *servinfo, *p;
  char s[INET6_ADDRSTRLEN];
  char buffer[CHUNK_SIZE] = {0};

  memset(&hints, 0, sizeof hints);
  setHints(&hints, FALSE);
  if (getaddrinfo(ip, serverport, &hints, &servinfo) != 0) {
    prints("ERR! getaddrinfo ");
    printl(serverport);
    return 0;
  }

  socketfd = runClient(&p, servinfo);
  if (p == NULL) {
    prints("ERR! failed to connect ");
    printl(serverport);
    return 0;
  }
  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);

  prints("connecting to ");
  printl(s);
  freeaddrinfo(servinfo); // all done with this structure

  strcat(buffer, INIT);
  strcat(buffer, ";");
  prints("Hi, please enter your phone number: ");
  myread(buffer + CMD_LEN + 1, CHUNK_SIZE - CMD_LEN - 1);
  strcat(buffer, ";");
  strcat(buffer, port);
  send(socketfd, buffer, CHUNK_SIZE, 0);

  while (1) {
    myread(buffer, CHUNK_SIZE);
    removeSpaces(buffer);
    prints(buffer);
    printl("---");
    if (strncmp(buffer, CALL, CMD_LEN) == 0) {
      char *phone;
      strcpy(phone, buffer + CMD_LEN);
      strcpy(buffer, "call;");
      strcat(buffer, phone);
      send(socketfd, buffer, CHUNK_SIZE, 0);
      recv(socketfd, buffer, CHUNK_SIZE, 0);
      printl(buffer);
    } else if (strcmp(buffer, EXIT) == 0) {
      break;
    }
  }

  close(socketfd);
  return 0;
}
