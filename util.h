#ifndef __UTILS__
#define __UTILS__

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BACKLOG 10
#define CHUNK_SIZE 256
#define USERS 25
#define FIRST_CLIENT 4
#define TRUE 1
#define FALSE 0
#define INVALID_FD -2
#define IP_LEN 15
#define PHONE_LEN 12
#define PORT_LEN 6
#define NOT_CONNECTED 0
#define CONNECTED 1
#define INITIALIZED 2
#define CHATTING 3
#define SENDING_FILE 4
#define INIT "init"
#define CALL "call"
#define CALLER "caller"
#define SEND_FILE "sendfile"
#define DONE "done"
#define DISCONNECT "disconnect"
#define EXIT "exit"

struct Client {
  char ip[IP_LEN];
  char phone[PHONE_LEN];
  char port[PORT_LEN];
  int status;
  int fd; // otional
};

void prints(char* s);
void printl(char* s);
void printi(int i);
int myread(char *buffer, int len);
void clearArray(char buffer[]);
void *get_in_addr(struct sockaddr *sa);
void itoa(int n, char s[]);
void reverse(char s[]);
void setHints(struct addrinfo *hints, int ai_flags);
int runServer(struct addrinfo **p, struct addrinfo *serveinfo);
int runClient(struct addrinfo **p, struct addrinfo *servinfo);
void removeSpaces(char *str);
void clearClient(struct Client *c);
void printClients(struct Client c[]);
void produceBuffer(char buffer[], char **s, int len);
int sendAll(int fd, char *buf, int len);
int sendFile(int fd, char *path, int flags);
int savefile(char *path ,char *buffer);
// void copyClient(struct Client *c1, struct Client c2);

#endif
