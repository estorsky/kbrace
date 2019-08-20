#ifndef CORE_H
#define CORE_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <ncursesw/curses.h>
// #include <ncurses.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <strings.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <signal.h>

#define true 1
#define false 0

#define PORT 1337
#define MAX_PLAYERS 30
#define MAX_USERNAME 10
#define MAX_WORD_LEN 20
#define MAX_WORDS 100

struct stat {
    int player_id;
    char name[MAX_USERNAME];
    int speed;
    int miss;
    double time;
    int prog;
    char state;
};

void exitprog();
double wtime();

extern char username[MAX_USERNAME];
extern char state;

extern pthread_t tid[2];
extern int sockfd;

#endif
