#ifndef __UTILS__
#define __UTILS__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BACKLOG 10
#define CHUNK_SIZE 256
#define USERS 25
#define CMD_LEN 4
#define TRUE 1
#define FALSE 0
#define INVALID_FD -1000
#define PHONE_LEN 12
#define PORT_LEN 6
#define INIT "init"
#define CALL "call"
#define EXIT "exit"

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

#endif
