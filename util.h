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

void prints(char* s);
void printl(char* s);
void printi(int i);
void *get_in_addr(struct sockaddr *sa);
void itoa(int n, char s[]);
void reverse(char s[]);

#endif
