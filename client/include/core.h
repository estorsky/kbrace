#ifndef CORE_H
#define CORE_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define true 1
#define false 0

#define PORT 1337
#define MAX_PLAYERS 30
#define MAX_USERNAME 10
#define MAX_WORD_LEN 20
#define MAX_WORDS 100

void exitprog();

extern char username[MAX_USERNAME];
extern char state;

extern pthread_t tid[2];
extern int sockfd;

#endif
