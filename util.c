#include "util.h"

void prints(char* s) {
  int i = write(STDOUT_FILENO, s, strlen(s));
}

void printl(char* s) {
  prints(s);
  prints("\n");
}

void printi(int i) {
  char buffer[100];
  itoa(i, buffer);
  write(STDOUT_FILENO, buffer, strlen(buffer));
}

int myread(char buffer[], int len) {
  int size = read(STDIN_FILENO, buffer, len);
  buffer[size - 1] = '\0';
  return size;
}

void clearArray(char buffer[]) {
  for (int i = 0; i < strlen(buffer); i++)
    buffer[i] = '\0';
}

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET)
    return &(((struct sockaddr_in*)sa)->sin_addr);

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void itoa(int n, char s[]) {
    int i, sign;

    if ((sign = n) < 0)
        n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

void reverse(char s[]) {
    int i, j;
    char c;

    for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void setHints(struct addrinfo *hints, int ai_flags) {
  hints->ai_family = AF_UNSPEC;
  hints->ai_socktype = SOCK_STREAM;
  if (ai_flags != 0)
    hints->ai_flags = AI_PASSIVE;
}
int runServer(struct addrinfo **p, struct addrinfo *servinfo) {
  int socketfd;
  int yes = 1;
  for((*p) = servinfo; (*p) != NULL; (*p) = (*p)->ai_next) {
    if ((socketfd = socket((*p)->ai_family, (*p)->ai_socktype, (*p)->ai_protocol)) == -1) {
      printl("ERR! socket");
      continue;
    }
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      printl("ERR! setsockopt");
      return 0;
    }
    if (bind(socketfd, (*p)->ai_addr, (*p)->ai_addrlen) == -1) {
      close(socketfd);
      printl("ERR! bind");
      continue;
    }
    break;
  }

  return socketfd;
}

int runClient(struct addrinfo **p, struct addrinfo *servinfo) {
  int socketfd;
  for((*p) = servinfo; (*p) != NULL; (*p) = (*p)->ai_next) {
    if ((socketfd = socket((*p)->ai_family, (*p)->ai_socktype, (*p)->ai_protocol)) == -1) {
      printl("ERR! socket");
      continue;
    }
    if (connect(socketfd, (*p)->ai_addr, (*p)->ai_addrlen) == -1) {
      close(socketfd);
      printl("ERR! connect");
      continue;
    }
    break;
  }

  return socketfd;
}

void removeSpaces(char *str) {
    int count = 0;
    for (int i = 0; str[i]; i++)
        if (str[i] != ' ')
            str[count++] = str[i];
    str[count] = '\0';
}

void clearClient(struct Client *c) {
  strcpy(c->ip, "\0");
  strcpy(c->phone, "\0");
  strcpy(c->port, "\0");
  c->status = NOT_CONNECTED;
}

void printClients(struct Client c[]) {
  int found = FALSE;
  printl("----------------------------------------------");
  for (int j = 0; j < USERS; j++) {
    if (c[j].status != NOT_CONNECTED) {
      found = TRUE;
      prints("client ");
      printi(j + FIRST_CLIENT);
      prints(": ");
      if (c[j].status < INITIALIZED)
        printl("waiting for client informations...");
      else {
        prints(c[j].phone);
        prints(" - ip: ");
        prints(c[j].ip);
        prints(" - port: ");
        prints(c[j].port);
        prints(" - status: ");
        switch (c[j].status) {
          case NOT_CONNECTED:
            printl("Not Connected");
            break;
          case CONNECTED:
            printl("Connected");
            break;
          case INITIALIZED:
            printl("Initialized");
            break;
          case CHATTING:
            printl("Chatting");
            break;
        }
      }
    }
  }
  if (found == FALSE) {
    printl("-- No Client Found");
  }
}

void produceBuffer(char buffer[], char **s, int len) {
  strcpy(buffer, s[0]);
  for (int i = 1; i < len; i++) {
    strcat(buffer, ";");
    strcat(buffer, s[i]);
  }
}


int sendAll(int fd, char *buf, int len) {
  int total = 0;
  int bytesleft = len;
  int n;
  while(total < len) {
    n = send(fd, buf + total, bytesleft, 0);
    if (n == -1)
      break;
    total += n;
    bytesleft -= n;
  }
  return n == -1 ? FALSE : total;
}

int sendFile(int fd, char *path, int flags) {
  int total = 0;
  int bytesleft = CHUNK_SIZE;
  int n = 1;
  char buffer[CHUNK_SIZE], temp[CHUNK_SIZE];
  int filefd = open(path, O_RDONLY);
  if (filefd == -1) {
    printl("ERR! open");
    return FALSE;
  }
  while (n > 0) {
    n = read(filefd, buffer + total, bytesleft);
    if (n == 0 && total > 0)
      bytesleft = 0;
    if (n == -1)
      break;
    total += n;
    bytesleft -= n;
    if (bytesleft == 0) {
      int x = sendAll(fd, buffer, CHUNK_SIZE);
      total = 0;
      bytesleft = CHUNK_SIZE;
      memset(buffer, '\0', CHUNK_SIZE);
    }
  }
  close(filefd);

  return n == -1 ? FALSE : TRUE;
}

int savefile(char *path ,char *buffer) {
  int total = 0;
  int bytesleft = CHUNK_SIZE;
  int n = 1;
  int filefd = open(path, O_CREAT, 0777);
  if (filefd == -1 && errno != EEXIST) {
    printl("ERR! open");
    return FALSE;
  } else {
    filefd = open(path, O_WRONLY | O_APPEND);
  }
  while (bytesleft > 0) {
    n = write(filefd, buffer + total, bytesleft);
    if (n == -1)
      break;
    total += n;
    bytesleft -= n;
  }
  close(filefd);

  return n == -1 ? FALSE : TRUE;
}

// void copyClient(struct Client *c1, struct Client c2) {
//   strcpy(c1->ip, c2.ip);
//   strcpy(c1->phone, c2.phone);
//   strcpy(c1->port, c2.port);
// }
